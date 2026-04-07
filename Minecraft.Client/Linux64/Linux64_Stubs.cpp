#include "stdafx.h"

#include <array>
#include <functional>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>

#include "Common/PostProcesser.h"
#include "Minecraft.World/AddEntityPacket.h"
#include "Minecraft.World/AnimatePacket.h"
#include "Minecraft.World/ByteBuffer.h"
#include "Minecraft.World/ChestTile.h"
#include "Minecraft.World/ClientCommandPacket.h"
#include "Minecraft.World/ContainerOpenPacket.h"
#include "Minecraft.World/EntityEvent.h"
#include "Minecraft.World/FloatBuffer.h"
#include "Minecraft.World/GameEventPacket.h"
#include "Minecraft.World/IntBuffer.h"
#include "Minecraft.World/Level.h"
#include "Minecraft.World/PotionBrewing.h"
#include "Minecraft.World/QuartzBlockTile.h"
#include "Minecraft.World/Sapling.h"
#include "Minecraft.World/SetEntityLinkPacket.h"
#include "Minecraft.World/SharedConstants.h"
#include "Minecraft.World/SkullTileEntity.h"
#include "Minecraft.World/TallGrass.h"
#include "Minecraft.World/TileEditorOpenPacket.h"
#include "Minecraft.World/TileEntityDataPacket.h"
#include "Linux64/Iggy/include/iggy.h"

extern SDL_Window *g_pWindow;
extern char g_Win64Username[17];
extern wchar_t g_Win64UsernameW[17];

#ifdef _LINUX64
// stubs.h remaps legacy GL entry points through RenderManager for the rest of
// the codebase. This implementation file is the RenderManager backend, so it
// must call the real GL functions directly to avoid recursive self-dispatch.
#undef glTranslatef
#undef glRotatef
#undef glPopMatrix
#undef glPushMatrix
#undef glScalef
#undef glMultMatrixf
#undef glMatrixMode
#undef glLoadIdentity
#undef glDeleteLists
#undef glGenLists
#undef glNewList
#undef glEndList
#undef glCallList
#undef glClear
#undef glClearColor
#undef glBindTexture
#undef glColor3f
#undef glColor4f
#undef glDepthMask
#undef glBlendFunc
#undef glDepthFunc
#undef glTexParameteri
#undef glPolygonOffset
#undef glLineWidth
#undef glColorMask
#undef glScaled
#undef glAlphaFunc
#undef glOrtho
#undef glFogi
#undef glFogf
#undef glCullFace
#undef glMultiTexCoord2f
#undef glDisable
#undef glEnable
#endif

namespace
{
int s_nextTextureId = 1;
int s_textureLevels = 1;
bool s_renderSuspended = false;
int s_activeCommandBuffer = -1;
bool s_replayingCommandBuffer = false;
int s_nextCommandBufferId = 1;
int s_boundTexture = -1;
int s_boundVertexTexture = -1;
int s_lastTextureUnit = 0;
float s_clearColour[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float s_globalVertexTexU = 0.0f;
float s_globalVertexTexV = 0.0f;
bool s_matrixDirty = true;

int s_primaryPad = 0;
int s_lockedProfile = 0;
bool s_fullVersionOverride = true;
std::array<C_4JProfile::PROFILESETTINGS, XUSER_MAX_COUNT> s_profileSettings = {};
std::array<std::vector<unsigned char>, XUSER_MAX_COUNT> s_profileData;
char s_fallbackGamertag[] = "Player";
int (*s_defaultOptionsCallback)(LPVOID, C_4JProfile::PROFILESETTINGS *, const int) = nullptr;
LPVOID s_defaultOptionsCallbackParam = nullptr;

std::array<unsigned char, XUSER_MAX_COUNT> s_joypadMaps = {};
std::array<bool, XUSER_MAX_COUNT> s_saveDeviceSelected = {};
bool s_saveDisabled = false;
std::vector<unsigned char> s_saveData;
char s_saveUniqueFilename[MAX_SAVEFILENAME_LENGTH] = "linuxsave";
int s_saveUniqueNumber = 0;
XMARKETPLACE_CONTENTOFFER_INFO s_offerData = {};
XCONTENT_DATA s_contentData = {};
IggyProperties s_dummyIggyProperties = {};
IggyValuePath s_dummyValuePath = {};
IggyFocusableObject s_dummyFocusableObject = {};
void *s_dummyIggyUserdata = nullptr;
unsigned char s_dummyIggyStorage = 0;

struct TextureEntry
{
	GLuint handle = 0;
	int width = 0;
	int height = 0;
};

struct CommandBuffer
{
	std::vector<std::function<void()>> commands;
	size_t approxBytes = 0;
};

std::unordered_map<int, TextureEntry> s_textures;
std::unordered_map<int, CommandBuffer> s_commandBuffers;

bool RecordCommand(std::function<void()> command, size_t approxBytes = 0)
{
	if (s_activeCommandBuffer < 0 || s_replayingCommandBuffer)
	{
		return false;
	}

	CommandBuffer &buffer = s_commandBuffers[s_activeCommandBuffer];
	buffer.commands.push_back(std::move(command));
	buffer.approxBytes += approxBytes;
	return true;
}

GLenum MatrixModeToGL(int type)
{
	if (type == GL_PROJECTION || type == GL_PROJECTION_MATRIX)
	{
		return GL_PROJECTION;
	}
	if (type == GL_TEXTURE)
	{
		return GL_TEXTURE;
	}
	return GL_MODELVIEW;
}

GLenum PrimitiveTypeToGL(C4JRender::ePrimitiveType primitiveType)
{
	switch (primitiveType)
	{
	case C4JRender::PRIMITIVE_TYPE_TRIANGLE_LIST:
		return GL_TRIANGLES;
	case C4JRender::PRIMITIVE_TYPE_TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
	case C4JRender::PRIMITIVE_TYPE_TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
	case C4JRender::PRIMITIVE_TYPE_QUAD_LIST:
		return GL_QUADS;
	case C4JRender::PRIMITIVE_TYPE_LINE_LIST:
		return GL_LINES;
	case C4JRender::PRIMITIVE_TYPE_LINE_STRIP:
		return GL_LINE_STRIP;
	default:
		return GL_TRIANGLES;
	}
}

TextureEntry *FindTextureEntry(int idx)
{
	auto it = s_textures.find(idx);
	return (it == s_textures.end()) ? nullptr : &it->second;
}

void BindTrackedTexture(int unit, int idx)
{
	glActiveTexture(unit == 0 ? GL_TEXTURE0 : GL_TEXTURE1);

	if (idx < 0)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
	else
	{
		TextureEntry *entry = FindTextureEntry(idx);
		if (entry != nullptr && entry->handle != 0)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, entry->handle);
		}
	}

	glActiveTexture(GL_TEXTURE0);
}

void ConfigureViewport(C4JRender::eViewportType viewportType)
{
	if (g_pWindow == nullptr)
	{
		return;
	}

	int width = 0;
	int height = 0;
	SDL_GL_GetDrawableSize(g_pWindow, &width, &height);
	if (width <= 0 || height <= 0)
	{
		return;
	}

	int x = 0;
	int y = 0;
	int w = width;
	int h = height;

	switch (viewportType)
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		h = height / 2;
		y = height - h;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		h = height / 2;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		w = width / 2;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		w = width / 2;
		x = width - w;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		w = width / 2;
		h = height / 2;
		y = height - h;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		w = width / 2;
		h = height / 2;
		x = width - w;
		y = height - h;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		w = width / 2;
		h = height / 2;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		w = width / 2;
		h = height / 2;
		x = width - w;
		break;
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
	default:
		break;
	}

	glViewport(x, y, w, h);
}

void ExecuteDrawVertices(C4JRender::ePrimitiveType primitiveType, int count, void *dataIn, C4JRender::eVertexType vertexType)
{
	if (dataIn == nullptr || count <= 0)
	{
		return;
	}

	if (vertexType == C4JRender::VERTEX_TYPE_COMPRESSED)
	{
		static bool warned = false;
		if (!warned)
		{
			fprintf(stderr, "[Linux64] VERTEX_TYPE_COMPRESSED is not implemented; draw skipped.\n");
			warned = true;
		}
		return;
	}

	const unsigned char *bytes = static_cast<const unsigned char *>(dataIn);
	const int stride = 8 * static_cast<int>(sizeof(int));

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, stride, bytes + 0);

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, stride, bytes + 3 * sizeof(float));

	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_UNSIGNED_BYTE, stride, bytes + 5 * sizeof(int));

	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_BYTE, stride, bytes + 6 * sizeof(int));

	std::vector<float> vertexTextureCoords;
	if (s_boundVertexTexture >= 0)
	{
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);

		TextureEntry *vertexTexture = FindTextureEntry(s_boundVertexTexture);
		if (vertexTexture != nullptr && vertexTexture->handle != 0)
		{
			glBindTexture(GL_TEXTURE_2D, vertexTexture->handle);
		}

		const int sentinel = static_cast<int>(0xfe00fe00u);
		const int *vertexData = static_cast<const int *>(dataIn);
		if (vertexData[7] == sentinel)
		{
			glClientActiveTexture(GL_TEXTURE1);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			__glewMultiTexCoord2f(GL_TEXTURE1, s_globalVertexTexU, s_globalVertexTexV);
		}
		else
		{
			vertexTextureCoords.resize(static_cast<size_t>(count) * 2);
			for (int i = 0; i < count; ++i)
			{
				const int tex2Packed = vertexData[i * 8 + 7];
				const int16_t *tex2 = reinterpret_cast<const int16_t *>(&tex2Packed);
				vertexTextureCoords[static_cast<size_t>(i) * 2 + 0] = static_cast<float>(tex2[0]) / 256.0f;
				vertexTextureCoords[static_cast<size_t>(i) * 2 + 1] = static_cast<float>(tex2[1]) / 256.0f;
			}

			glClientActiveTexture(GL_TEXTURE1);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, 0, vertexTextureCoords.data());
		}

		glActiveTexture(GL_TEXTURE0);
	}
	else
	{
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE0);
	}

	glDrawArrays(PrimitiveTypeToGL(primitiveType), 0, count);

	if (s_boundVertexTexture >= 0)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE0);
	}

	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

int ClampPadIndex(int iPad)
{
	if (iPad < 0)
	{
		return 0;
	}
	if (iPad >= XUSER_MAX_COUNT)
	{
		return XUSER_MAX_COUNT - 1;
	}
	return iPad;
}

Iggy *DummyIggy()
{
	return reinterpret_cast<Iggy *>(&s_dummyIggyStorage);
}
}

C4JRender RenderManager;

void C4JRender::Tick() {}
void C4JRender::UpdateGamma(unsigned short) {}
void C4JRender::MatrixMode(int type)
{
	const GLenum mode = MatrixModeToGL(type);
	if (RecordCommand([mode]() { glMatrixMode(mode); s_matrixDirty = true; }))
	{
		return;
	}

	glMatrixMode(mode);
	s_matrixDirty = true;
}
void C4JRender::MatrixSetIdentity()
{
	if (RecordCommand([]() { glLoadIdentity(); s_matrixDirty = true; }))
	{
		return;
	}

	glLoadIdentity();
	s_matrixDirty = true;
}
void C4JRender::MatrixTranslate(float x, float y, float z)
{
	if (RecordCommand([x, y, z]() { glTranslatef(x, y, z); s_matrixDirty = true; }))
	{
		return;
	}

	glTranslatef(x, y, z);
	s_matrixDirty = true;
}
void C4JRender::MatrixRotate(float angle, float x, float y, float z)
{
	const float degrees = angle * (180.0f / PI);
	if (RecordCommand([degrees, x, y, z]() { glRotatef(degrees, x, y, z); s_matrixDirty = true; }))
	{
		return;
	}

	glRotatef(degrees, x, y, z);
	s_matrixDirty = true;
}
void C4JRender::MatrixScale(float x, float y, float z)
{
	if (RecordCommand([x, y, z]() { glScalef(x, y, z); s_matrixDirty = true; }))
	{
		return;
	}

	glScalef(x, y, z);
	s_matrixDirty = true;
}
void C4JRender::MatrixPerspective(float fovy, float aspect, float zNear, float zFar)
{
	const float radians = fovy * (PI / 180.0f);
	const float top = tanf(radians * 0.5f) * zNear;
	const float bottom = -top;
	const float right = top * aspect;
	const float left = -right;
	if (RecordCommand([left, right, bottom, top, zNear, zFar]() { glFrustum(left, right, bottom, top, zNear, zFar); s_matrixDirty = true; }))
	{
		return;
	}

	glFrustum(left, right, bottom, top, zNear, zFar);
	s_matrixDirty = true;
}
void C4JRender::MatrixOrthogonal(float left, float right, float bottom, float top, float zNear, float zFar)
{
	if (RecordCommand([left, right, bottom, top, zNear, zFar]() { glOrtho(left, right, bottom, top, zNear, zFar); s_matrixDirty = true; }))
	{
		return;
	}

	glOrtho(left, right, bottom, top, zNear, zFar);
	s_matrixDirty = true;
}
void C4JRender::MatrixPop()
{
	if (RecordCommand([]() { glPopMatrix(); s_matrixDirty = true; }))
	{
		return;
	}

	glPopMatrix();
	s_matrixDirty = true;
}
void C4JRender::MatrixPush()
{
	if (RecordCommand([]() { glPushMatrix(); s_matrixDirty = true; }))
	{
		return;
	}

	glPushMatrix();
	s_matrixDirty = true;
}
void C4JRender::MatrixMult(float *mat)
{
	if (mat == nullptr)
	{
		return;
	}

	std::array<float, 16> copy = {};
	memcpy(copy.data(), mat, sizeof(copy));
	if (RecordCommand([copy]() mutable { glMultMatrixf(copy.data()); s_matrixDirty = true; }))
	{
		return;
	}

	glMultMatrixf(copy.data());
	s_matrixDirty = true;
}
const float *C4JRender::MatrixGet(int type)
{
	static std::array<float, 16> matrix = {};
	glGetFloatv(MatrixModeToGL(type == GL_MODELVIEW_MATRIX ? GL_MODELVIEW : type), matrix.data());
	return matrix.data();
}
void C4JRender::Set_matrixDirty() { s_matrixDirty = true; }
void C4JRender::Initialise(ID3D11Device *, IDXGISwapChain *)
{
	glClearDepth(1.0);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);
	glShadeModel(GL_SMOOTH);
	PostProcesser::GetInstance().Init();
}
void C4JRender::InitialiseContext() {}
void C4JRender::StartFrame()
{
	if (g_pWindow != nullptr)
	{
		int width = 0;
		int height = 0;
		SDL_GL_GetDrawableSize(g_pWindow, &width, &height);
		if (width > 0 && height > 0)
		{
			glViewport(0, 0, width, height);
		}
	}
}
void C4JRender::DoScreenGrabOnNextPresent() {}
void C4JRender::Present()
{
	PostProcesser::GetInstance().Apply();
	if (g_pWindow != nullptr)
	{
		SDL_GL_SwapWindow(g_pWindow);
	}
}
void C4JRender::Clear(int flags, D3D11_RECT *pRect)
{
	GLbitfield mask = 0;
	if ((flags & GL_COLOR_BUFFER_BIT) != 0)
	{
		mask |= GL_COLOR_BUFFER_BIT;
	}
	if ((flags & GL_DEPTH_BUFFER_BIT) != 0)
	{
		mask |= GL_DEPTH_BUFFER_BIT;
	}
	if (mask == 0)
	{
		return;
	}

	if (RecordCommand([mask, rect = (pRect != nullptr) ? *pRect : D3D11_RECT{} , hasRect = (pRect != nullptr)]()
	{
		if (hasRect)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
		}
		glClear(mask);
		if (hasRect)
		{
			glDisable(GL_SCISSOR_TEST);
		}
	}))
	{
		return;
	}

	if (pRect != nullptr)
	{
		glEnable(GL_SCISSOR_TEST);
		glScissor(pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top);
	}
	glClear(mask);
	if (pRect != nullptr)
	{
		glDisable(GL_SCISSOR_TEST);
	}
}
void C4JRender::SetClearColour(const float colourRGBA[4])
{
	if (colourRGBA == nullptr)
	{
		return;
	}

	memcpy(s_clearColour, colourRGBA, sizeof(s_clearColour));
	if (RecordCommand([colour = std::array<float, 4>{colourRGBA[0], colourRGBA[1], colourRGBA[2], colourRGBA[3]}]()
	{
		glClearColor(colour[0], colour[1], colour[2], colour[3]);
	}))
	{
		return;
	}

	glClearColor(colourRGBA[0], colourRGBA[1], colourRGBA[2], colourRGBA[3]);
}
bool C4JRender::IsWidescreen() { return true; }
bool C4JRender::IsHiDef() { return true; }
void C4JRender::CaptureThumbnail(ImageFileBuffer *pngOut)
{
	if (pngOut != nullptr)
	{
		pngOut->m_type = ImageFileBuffer::e_typePNG;
		pngOut->m_pBuffer = nullptr;
		pngOut->m_bufferSize = 0;
	}
}
void C4JRender::CaptureScreen(ImageFileBuffer *jpgOut, XSOCIAL_PREVIEWIMAGE *previewOut)
{
	if (jpgOut != nullptr)
	{
		jpgOut->m_type = ImageFileBuffer::e_typeJPG;
		jpgOut->m_pBuffer = nullptr;
		jpgOut->m_bufferSize = 0;
	}
	if (previewOut != nullptr)
	{
		ZeroMemory(previewOut, sizeof(*previewOut));
	}
}
void C4JRender::BeginConditionalSurvey(int) {}
void C4JRender::EndConditionalSurvey() {}
void C4JRender::BeginConditionalRendering(int) {}
void C4JRender::EndConditionalRendering() {}
void C4JRender::DrawVertices(C4JRender::ePrimitiveType primitiveType, int count, void *dataIn, C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType)
{
	if (dataIn == nullptr || count <= 0)
	{
		return;
	}

	const size_t stride = (vertexType == C4JRender::VERTEX_TYPE_COMPRESSED) ? 16u : 32u;
	std::vector<unsigned char> copiedData;
	copiedData.resize(stride * static_cast<size_t>(count));
	memcpy(copiedData.data(), dataIn, copiedData.size());

	if (RecordCommand([primitiveType, count, vertexType, copiedData]()
	{
		ExecuteDrawVertices(primitiveType, count, const_cast<unsigned char *>(copiedData.data()), vertexType);
	}, copiedData.size()))
	{
		return;
	}

	ExecuteDrawVertices(primitiveType, count, copiedData.data(), vertexType);
}
void C4JRender::DrawVertexBuffer(C4JRender::ePrimitiveType, int, ID3D11Buffer *, C4JRender::eVertexType, C4JRender::ePixelShaderType) {}
void C4JRender::CBuffLockStaticCreations() {}
int C4JRender::CBuffCreate(int count)
{
	const int first = s_nextCommandBufferId;
	const int actualCount = (count > 0) ? count : 1;
	for (int i = 0; i < actualCount; ++i)
	{
		s_commandBuffers[first + i] = {};
	}
	s_nextCommandBufferId += actualCount;
	return first;
}
void C4JRender::CBuffDelete(int first, int count)
{
	const int actualCount = (count > 0) ? count : 1;
	for (int i = 0; i < actualCount; ++i)
	{
		s_commandBuffers.erase(first + i);
	}
}
void C4JRender::CBuffStart(int index, bool)
{
	CommandBuffer &buffer = s_commandBuffers[index];
	buffer.commands.clear();
	buffer.approxBytes = 0;
	s_activeCommandBuffer = index;
}
void C4JRender::CBuffClear(int index)
{
	auto it = s_commandBuffers.find(index);
	if (it != s_commandBuffers.end())
	{
		it->second.commands.clear();
		it->second.approxBytes = 0;
	}
}
int C4JRender::CBuffSize(int index)
{
	if (index < 0)
	{
		size_t totalBytes = 0;
		for (const auto &entry : s_commandBuffers)
		{
			totalBytes += entry.second.approxBytes;
		}
		return static_cast<int>(totalBytes);
	}

	auto it = s_commandBuffers.find(index);
	return (it == s_commandBuffers.end()) ? 0 : static_cast<int>(it->second.approxBytes);
}
void C4JRender::CBuffEnd()
{
	s_activeCommandBuffer = -1;
}
bool C4JRender::CBuffCall(int index, bool)
{
	auto it = s_commandBuffers.find(index);
	if (it == s_commandBuffers.end() || it->second.commands.empty())
	{
		return false;
	}

	const bool wasReplaying = s_replayingCommandBuffer;
	s_replayingCommandBuffer = true;
	for (const auto &command : it->second.commands)
	{
		command();
	}
	s_replayingCommandBuffer = wasReplaying;
	return true;
}
void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}
int C4JRender::TextureCreate()
{
	const int idx = s_nextTextureId++;
	TextureEntry &entry = s_textures[idx];
	glGenTextures(1, &entry.handle);
	return idx;
}
void C4JRender::TextureFree(int idx)
{
	auto it = s_textures.find(idx);
	if (it == s_textures.end())
	{
		return;
	}

	if (it->second.handle != 0)
	{
		glDeleteTextures(1, &it->second.handle);
	}
	s_textures.erase(it);
}
void C4JRender::TextureBind(int idx)
{
	s_boundTexture = idx;
	s_lastTextureUnit = 0;
	if (RecordCommand([idx]() { BindTrackedTexture(0, idx); }))
	{
		return;
	}

	BindTrackedTexture(0, idx);
}
void C4JRender::TextureBindVertex(int idx)
{
	s_boundVertexTexture = idx;
	s_lastTextureUnit = 1;
	if (RecordCommand([idx]() { BindTrackedTexture(1, idx); }))
	{
		return;
	}

	BindTrackedTexture(1, idx);
}
void C4JRender::TextureSetTextureLevels(int levels) { s_textureLevels = levels; }
int C4JRender::TextureGetTextureLevels() { return s_textureLevels; }
void C4JRender::TextureData(int width, int height, void *data, int level, eTextureFormat)
{
	if (s_boundTexture < 0 || data == nullptr)
	{
		return;
	}

	TextureEntry *entry = FindTextureEntry(s_boundTexture);
	if (entry == nullptr)
	{
		return;
	}

	entry->width = width;
	entry->height = height;
	glBindTexture(GL_TEXTURE_2D, entry->handle);
	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	if (level == 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}
void C4JRender::TextureDataUpdate(int xoffset, int yoffset, int width, int height, void *data, int level)
{
	if (s_boundTexture < 0 || data == nullptr)
	{
		return;
	}

	TextureEntry *entry = FindTextureEntry(s_boundTexture);
	if (entry == nullptr)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, entry->handle);
	glTexSubImage2D(GL_TEXTURE_2D, level, xoffset, yoffset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, data);
}
void C4JRender::TextureSetParam(int param, int value)
{
	const int textureIndex = (s_lastTextureUnit == 1) ? s_boundVertexTexture : s_boundTexture;
	TextureEntry *entry = FindTextureEntry(textureIndex);
	if (entry == nullptr)
	{
		return;
	}

	GLenum actualParam = static_cast<GLenum>(param);
	GLint actualValue = value;
	if (param == GL_TEXTURE_WRAP_S || param == GL_TEXTURE_WRAP_T)
	{
		if (value == GL_CLAMP)
		{
			actualValue = GL_CLAMP_TO_EDGE;
		}
		else if (value == GL_REPEAT)
		{
			actualValue = GL_REPEAT;
		}
	}

	glActiveTexture(s_lastTextureUnit == 1 ? GL_TEXTURE1 : GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, entry->handle);
	glTexParameteri(GL_TEXTURE_2D, actualParam, actualValue);
	glActiveTexture(GL_TEXTURE0);
}
void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}
// LoadTextureData implementations moved to Linux64_TextureLoader.cpp (stb_image)
HRESULT C4JRender::SaveTextureData(const char *, D3DXIMAGE_INFO *, int *) { return E_FAIL; }
HRESULT C4JRender::SaveTextureDataToMemory(void *, int, int *, int, int, int *) { return E_FAIL; }
void C4JRender::TextureGetStats() {}
ID3D11ShaderResourceView *C4JRender::TextureGetTexture(int) { return nullptr; }
void C4JRender::StateSetColour(float r, float g, float b, float a)
{
	if (RecordCommand([r, g, b, a]() { glColor4f(r, g, b, a); }))
	{
		return;
	}

	glColor4f(r, g, b, a);
}
void C4JRender::StateSetDepthMask(bool enable)
{
	if (RecordCommand([enable]() { glDepthMask(enable ? GL_TRUE : GL_FALSE); }))
	{
		return;
	}

	glDepthMask(enable ? GL_TRUE : GL_FALSE);
}
void C4JRender::StateSetBlendEnable(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}
void C4JRender::StateSetBlendFunc(int src, int dst)
{
	if (RecordCommand([src, dst]() { glBlendFunc(src, dst); }))
	{
		return;
	}

	glBlendFunc(src, dst);
}
void C4JRender::StateSetBlendFactor(unsigned int colour)
{
	const float a = static_cast<float>(colour & 0xff) / 255.0f;
	const float b = static_cast<float>((colour >> 8) & 0xff) / 255.0f;
	const float g = static_cast<float>((colour >> 16) & 0xff) / 255.0f;
	const float r = static_cast<float>((colour >> 24) & 0xff) / 255.0f;
	if (RecordCommand([r, g, b, a]() { glBlendColor(r, g, b, a); }))
	{
		return;
	}

	glBlendColor(r, g, b, a);
}
void C4JRender::StateSetAlphaFunc(int func, float param)
{
	if (RecordCommand([func, param]() { glAlphaFunc(func, param); }))
	{
		return;
	}

	glAlphaFunc(func, param);
}
void C4JRender::StateSetDepthFunc(int func)
{
	if (RecordCommand([func]() { glDepthFunc(func); }))
	{
		return;
	}

	glDepthFunc(func);
}
void C4JRender::StateSetFaceCull(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}
void C4JRender::StateSetFaceCullCW(bool enable)
{
	const GLenum mode = enable ? GL_BACK : GL_FRONT;
	if (RecordCommand([mode]() { glCullFace(mode); }))
	{
		return;
	}

	glCullFace(mode);
}
void C4JRender::StateSetLineWidth(float width)
{
	if (RecordCommand([width]() { glLineWidth(width); }))
	{
		return;
	}

	glLineWidth(width);
}
void C4JRender::StateSetWriteEnable(bool red, bool green, bool blue, bool alpha)
{
	if (RecordCommand([red, green, blue, alpha]() { glColorMask(red, green, blue, alpha); }))
	{
		return;
	}

	glColorMask(red, green, blue, alpha);
}
void C4JRender::StateSetDepthTestEnable(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}
void C4JRender::StateSetAlphaTestEnable(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_ALPHA_TEST);
		}
		else
		{
			glDisable(GL_ALPHA_TEST);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}
}
void C4JRender::StateSetDepthSlopeAndBias(float slope, float bias)
{
	if (RecordCommand([slope, bias]()
	{
		if (slope != 0.0f || bias != 0.0f)
		{
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(slope, bias);
		}
		else
		{
			glDisable(GL_POLYGON_OFFSET_FILL);
		}
	}))
	{
		return;
	}

	if (slope != 0.0f || bias != 0.0f)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(slope, bias);
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}
void C4JRender::StateSetFogEnable(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_FOG);
		}
		else
		{
			glDisable(GL_FOG);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}
}
void C4JRender::StateSetFogMode(int mode)
{
	if (RecordCommand([mode]() { glFogi(GL_FOG_MODE, mode); }))
	{
		return;
	}

	glFogi(GL_FOG_MODE, mode);
}
void C4JRender::StateSetFogNearDistance(float dist)
{
	if (RecordCommand([dist]() { glFogf(GL_FOG_START, dist); }))
	{
		return;
	}

	glFogf(GL_FOG_START, dist);
}
void C4JRender::StateSetFogFarDistance(float dist)
{
	if (RecordCommand([dist]() { glFogf(GL_FOG_END, dist); }))
	{
		return;
	}

	glFogf(GL_FOG_END, dist);
}
void C4JRender::StateSetFogDensity(float density)
{
	if (RecordCommand([density]() { glFogf(GL_FOG_DENSITY, density); }))
	{
		return;
	}

	glFogf(GL_FOG_DENSITY, density);
}
void C4JRender::StateSetFogColour(float red, float green, float blue)
{
	const std::array<float, 4> colour = {red, green, blue, 1.0f};
	if (RecordCommand([colour]() { glFogfv(GL_FOG_COLOR, colour.data()); }))
	{
		return;
	}

	glFogfv(GL_FOG_COLOR, colour.data());
}
void C4JRender::StateSetLightingEnable(bool enable)
{
	if (RecordCommand([enable]()
	{
		if (enable)
		{
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}
}
void C4JRender::StateSetVertexTextureUV(float u, float v)
{
	s_globalVertexTexU = u;
	s_globalVertexTexV = v;
}
void C4JRender::StateSetLightColour(int light, float red, float green, float blue)
{
	const GLenum glLightId = (light == 0) ? GL_LIGHT0 : GL_LIGHT1;
	const std::array<float, 4> colour = {red, green, blue, 1.0f};
	if (RecordCommand([glLightId, colour]() { glLightfv(glLightId, GL_DIFFUSE, colour.data()); }))
	{
		return;
	}

	glLightfv(glLightId, GL_DIFFUSE, colour.data());
}
void C4JRender::StateSetLightAmbientColour(float red, float green, float blue)
{
	const std::array<float, 4> colour = {red, green, blue, 1.0f};
	if (RecordCommand([colour]() { glLightModelfv(GL_LIGHT_MODEL_AMBIENT, colour.data()); }))
	{
		return;
	}

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, colour.data());
}
void C4JRender::StateSetLightDirection(int light, float x, float y, float z)
{
	const GLenum glLightId = (light == 0) ? GL_LIGHT0 : GL_LIGHT1;
	const std::array<float, 4> dir = {x, y, z, 0.0f};
	if (RecordCommand([glLightId, dir]() { glLightfv(glLightId, GL_POSITION, dir.data()); }))
	{
		return;
	}

	glLightfv(glLightId, GL_POSITION, dir.data());
}
void C4JRender::StateSetLightEnable(int light, bool enable)
{
	const GLenum glLightId = (light == 0) ? GL_LIGHT0 : GL_LIGHT1;
	if (RecordCommand([glLightId, enable]()
	{
		if (enable)
		{
			glEnable(glLightId);
		}
		else
		{
			glDisable(glLightId);
		}
	}))
	{
		return;
	}

	if (enable)
	{
		glEnable(glLightId);
	}
	else
	{
		glDisable(glLightId);
	}
}
void C4JRender::StateSetViewport(C4JRender::eViewportType viewportType)
{
	if (RecordCommand([viewportType]() { ConfigureViewport(viewportType); }))
	{
		return;
	}

	ConfigureViewport(viewportType);
}
void C4JRender::StateSetEnableViewportClipPlanes(bool) {}
void C4JRender::StateSetTexGenCol(int col, float x, float y, float z, float w, bool eyeSpace)
{
	const GLenum coord = (col == GL_S) ? GL_S : ((col == GL_T) ? GL_T : ((col == GL_Q) ? GL_Q : GL_R));
	const GLenum plane = eyeSpace ? GL_EYE_PLANE : GL_OBJECT_PLANE;
	const std::array<float, 4> values = {x, y, z, w};
	if (RecordCommand([coord, plane, values]()
	{
		glTexGeni(coord, GL_TEXTURE_GEN_MODE, plane == GL_EYE_PLANE ? GL_EYE_LINEAR : GL_OBJECT_LINEAR);
		glTexGenfv(coord, plane, values.data());
		glEnable((coord == GL_S) ? GL_TEXTURE_GEN_S :
				 (coord == GL_T) ? GL_TEXTURE_GEN_T :
				 (coord == GL_Q) ? GL_TEXTURE_GEN_Q : GL_TEXTURE_GEN_R);
	}))
	{
		return;
	}

	glTexGeni(coord, GL_TEXTURE_GEN_MODE, eyeSpace ? GL_EYE_LINEAR : GL_OBJECT_LINEAR);
	glTexGenfv(coord, plane, values.data());
	glEnable((coord == GL_S) ? GL_TEXTURE_GEN_S :
			 (coord == GL_T) ? GL_TEXTURE_GEN_T :
			 (coord == GL_Q) ? GL_TEXTURE_GEN_Q : GL_TEXTURE_GEN_R);
}
void C4JRender::StateSetStencil(int function, uint8_t stencil_ref, uint8_t stencil_func_mask, uint8_t stencil_write_mask)
{
	if (RecordCommand([function, stencil_ref, stencil_func_mask, stencil_write_mask]()
	{
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(function, stencil_ref, stencil_func_mask);
		glStencilMask(stencil_write_mask);
	}))
	{
		return;
	}

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(function, stencil_ref, stencil_func_mask);
	glStencilMask(stencil_write_mask);
}
void C4JRender::StateSetForceLOD(int) {}
void C4JRender::BeginEvent(LPCWSTR) {}
void C4JRender::EndEvent() {}
void C4JRender::Suspend() { s_renderSuspended = true; }
bool C4JRender::Suspended() { return s_renderSuspended; }
void C4JRender::Resume() { s_renderSuspended = false; }

const char *PostProcesser::g_gammaVSCode = "";
const char *PostProcesser::g_gammaPSCode = "";

PostProcesser::PostProcesser() = default;
PostProcesser::~PostProcesser() = default;

bool PostProcesser::IsRunningUnderWine()
{
	return false;
}

void PostProcesser::Init()
{
	m_initialized = true;
}

void PostProcesser::Apply() const {}
void PostProcesser::SetViewport(const D3D11_VIEWPORT &viewport)
{
	m_customViewport = viewport;
	m_useCustomViewport = true;
}
void PostProcesser::ResetViewport()
{
	m_useCustomViewport = false;
}
void PostProcesser::CopyBackbuffer() {}
void PostProcesser::ApplyFromCopied() const {}
void PostProcesser::Cleanup()
{
	m_initialized = false;
	m_useCustomViewport = false;
}
void PostProcesser::SetGamma(float gamma)
{
	m_gamma = gamma;
}

C_4JProfile ProfileManager;

void C_4JProfile::Initialise(DWORD, DWORD, unsigned short, UINT, UINT, DWORD *, int iGameDefinedDataSizeX4, unsigned int *)
{
	const int bytesPerProfile = (iGameDefinedDataSizeX4 > 0) ? (iGameDefinedDataSizeX4 / XUSER_MAX_COUNT) : 0;
	for (int i = 0; i < XUSER_MAX_COUNT; ++i)
	{
		s_profileData[i].assign(bytesPerProfile, 0);
		if (s_defaultOptionsCallback != nullptr)
		{
			s_defaultOptionsCallback(s_defaultOptionsCallbackParam, &s_profileSettings[i], i);
		}
	}
}
void C_4JProfile::SetTrialTextStringTable(CXuiStringTable *, int, int) {}
void C_4JProfile::SetTrialAwardText(eAwardType, int, int) {}
int C_4JProfile::GetLockedProfile() { return s_lockedProfile; }
void C_4JProfile::SetLockedProfile(int iProf) { s_lockedProfile = iProf; }
bool C_4JProfile::IsSignedIn(int iQuadrant)
{
	return iQuadrant == XUSER_INDEX_ANY || (iQuadrant >= 0 && iQuadrant < XUSER_MAX_COUNT);
}
bool C_4JProfile::IsSignedInLive(int) { return false; }
bool C_4JProfile::IsGuest(int) { return false; }
UINT C_4JProfile::RequestSignInUI(bool, bool, bool, bool, bool, int (*Func)(LPVOID, const bool, const int), LPVOID lpParam, int iQuadrant)
{
	if (Func != nullptr)
	{
		Func(lpParam, true, (iQuadrant == XUSER_INDEX_ANY) ? 0 : ClampPadIndex(iQuadrant));
	}
	return 0;
}
UINT C_4JProfile::DisplayOfflineProfile(int (*Func)(LPVOID, const bool, const int), LPVOID lpParam, int iQuadrant)
{
	if (Func != nullptr)
	{
		Func(lpParam, true, (iQuadrant == XUSER_INDEX_ANY) ? 0 : ClampPadIndex(iQuadrant));
	}
	return 0;
}
UINT C_4JProfile::RequestConvertOfflineToGuestUI(int (*Func)(LPVOID, const bool, const int), LPVOID lpParam, int iQuadrant)
{
	if (Func != nullptr)
	{
		Func(lpParam, true, (iQuadrant == XUSER_INDEX_ANY) ? 0 : ClampPadIndex(iQuadrant));
	}
	return 0;
}
void C_4JProfile::SetPrimaryPlayerChanged(bool) {}
bool C_4JProfile::QuerySigninStatus(void) { return true; }
void C_4JProfile::GetXUID(int iPad, PlayerUID *pXuid, bool)
{
	if (pXuid != nullptr)
	{
		*pXuid = 0xe000d45248242f2eULL + static_cast<unsigned long long>(ClampPadIndex(iPad));
	}
}
BOOL C_4JProfile::AreXUIDSEqual(PlayerUID xuid1, PlayerUID xuid2) { return xuid1 == xuid2; }
BOOL C_4JProfile::XUIDIsGuest(PlayerUID) { return false; }
bool C_4JProfile::AllowedToPlayMultiplayer(int) { return true; }
bool C_4JProfile::GetChatAndContentRestrictions(int, bool *pbChatRestricted, bool *pbContentRestricted, int *piAge)
{
	if (pbChatRestricted != nullptr)
	{
		*pbChatRestricted = false;
	}
	if (pbContentRestricted != nullptr)
	{
		*pbContentRestricted = false;
	}
	if (piAge != nullptr)
	{
		*piAge = 18;
	}
	return true;
}
void C_4JProfile::StartTrialGame() {}
void C_4JProfile::AllowedPlayerCreatedContent(int, bool, BOOL *allAllowed, BOOL *friendsAllowed)
{
	if (allAllowed != nullptr)
	{
		*allAllowed = TRUE;
	}
	if (friendsAllowed != nullptr)
	{
		*friendsAllowed = TRUE;
	}
}
BOOL C_4JProfile::CanViewPlayerCreatedContent(int, bool, PPlayerUID, DWORD) { return TRUE; }
void C_4JProfile::ShowProfileCard(int, PlayerUID) {}
bool C_4JProfile::GetProfileAvatar(int, int (*Func)(LPVOID, PBYTE, DWORD), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, nullptr, 0);
	}
	return false;
}
void C_4JProfile::CancelProfileAvatarRequest() {}
int C_4JProfile::GetPrimaryPad() { return s_primaryPad; }
void C_4JProfile::SetPrimaryPad(int iPad) { s_primaryPad = ClampPadIndex(iPad); }
char *C_4JProfile::GetGamertag(int)
{
	return (g_Win64Username[0] != '\0') ? g_Win64Username : s_fallbackGamertag;
}
wstring C_4JProfile::GetDisplayName(int)
{
	return (g_Win64UsernameW[0] != L'\0') ? wstring(g_Win64UsernameW) : wstring(L"Player");
}
bool C_4JProfile::IsFullVersion() { return s_fullVersionOverride; }
void C_4JProfile::SetSignInChangeCallback(void (*)(LPVOID, bool, unsigned int), LPVOID) {}
void C_4JProfile::SetNotificationsCallback(void (*)(LPVOID, DWORD, unsigned int), LPVOID) {}
bool C_4JProfile::RegionIsNorthAmerica(void) { return true; }
bool C_4JProfile::LocaleIsUSorCanada(void) { return true; }
HRESULT C_4JProfile::GetLiveConnectionStatus() { return S_OK; }
bool C_4JProfile::IsSystemUIDisplayed() { return false; }
void C_4JProfile::SetProfileReadErrorCallback(void (*)(LPVOID), LPVOID) {}
int C_4JProfile::SetDefaultOptionsCallback(int (*Func)(LPVOID, PROFILESETTINGS *, const int), LPVOID lpParam)
{
	s_defaultOptionsCallback = Func;
	s_defaultOptionsCallbackParam = lpParam;
	return 0;
}
int C_4JProfile::SetOldProfileVersionCallback(int (*)(LPVOID, unsigned char *, const unsigned short, const int), LPVOID) { return 0; }
C_4JProfile::PROFILESETTINGS *C_4JProfile::GetDashboardProfileSettings(int iPad)
{
	return &s_profileSettings[ClampPadIndex(iPad)];
}
void C_4JProfile::WriteToProfile(int, bool, bool) {}
void C_4JProfile::ForceQueuedProfileWrites(int) {}
void *C_4JProfile::GetGameDefinedProfileData(int iQuadrant)
{
	return s_profileData[ClampPadIndex(iQuadrant)].empty() ? nullptr : s_profileData[ClampPadIndex(iQuadrant)].data();
}
void C_4JProfile::ResetProfileProcessState() {}
void C_4JProfile::Tick(void) {}
void C_4JProfile::RegisterAward(int, int, eAwardType, bool, CXuiStringTable *, int, int, int, char *, unsigned int) {}
int C_4JProfile::GetAwardId(int iAwardNumber) { return iAwardNumber; }
eAwardType C_4JProfile::GetAwardType(int) { return eAwardType_Achievement; }
bool C_4JProfile::CanBeAwarded(int, int) { return false; }
void C_4JProfile::Award(int, int, bool) {}
bool C_4JProfile::IsAwardsFlagSet(int, int) { return false; }
void C_4JProfile::RichPresenceInit(int, int) {}
void C_4JProfile::RegisterRichPresenceContext(int) {}
void C_4JProfile::SetRichPresenceContextValue(int, int, int) {}
void C_4JProfile::SetCurrentGameActivity(int, int, bool) {}
void C_4JProfile::DisplayFullVersionPurchase(bool, int, int) {}
void C_4JProfile::SetUpsellCallback(void (*)(LPVOID, eUpsellType, eUpsellResponse, int), LPVOID) {}
void C_4JProfile::SetDebugFullOverride(bool bVal) { s_fullVersionOverride = bVal; }

C_4JInput InputManager;

void C_4JInput::Initialise(int, unsigned char, unsigned char, unsigned char) {}
void C_4JInput::Tick(void) {}
void C_4JInput::SetDeadzoneAndMovementRange(unsigned int, unsigned int) {}
void C_4JInput::SetGameJoypadMaps(unsigned char, unsigned char, unsigned int) {}
unsigned int C_4JInput::GetGameJoypadMaps(unsigned char, unsigned char) { return 0; }
void C_4JInput::SetJoypadMapVal(int iPad, unsigned char ucMap) { s_joypadMaps[ClampPadIndex(iPad)] = ucMap; }
unsigned char C_4JInput::GetJoypadMapVal(int iPad) { return s_joypadMaps[ClampPadIndex(iPad)]; }
void C_4JInput::SetJoypadSensitivity(int, float) {}
unsigned int C_4JInput::GetValue(int, unsigned char, bool) { return 0; }
bool C_4JInput::ButtonPressed(int, unsigned char) { return false; }
bool C_4JInput::ButtonReleased(int, unsigned char) { return false; }
bool C_4JInput::ButtonDown(int, unsigned char) { return false; }
void C_4JInput::SetJoypadStickAxisMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetJoypadStickTriggerMap(int, unsigned int, unsigned int) {}
void C_4JInput::SetKeyRepeatRate(float, float) {}
void C_4JInput::SetDebugSequence(const char *, int (*)(LPVOID), LPVOID) {}
FLOAT C_4JInput::GetIdleSeconds(int) { return 0.0f; }
bool C_4JInput::IsPadConnected(int iPad) { return iPad == 0; }
float C_4JInput::GetJoypadStick_LX(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_LY(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_RX(int, bool) { return 0.0f; }
float C_4JInput::GetJoypadStick_RY(int, bool) { return 0.0f; }
unsigned char C_4JInput::GetJoypadLTrigger(int, bool) { return 0; }
unsigned char C_4JInput::GetJoypadRTrigger(int, bool) { return 0; }
void C_4JInput::SetMenuDisplayed(int, bool) {}
EKeyboardResult C_4JInput::RequestKeyboard(LPCWSTR, LPCWSTR, DWORD, UINT, int (*Func)(LPVOID, const bool), LPVOID lpParam, C_4JInput::EKeyboardMode)
{
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
	return EKeyboard_ResultAccept;
}
void C_4JInput::GetText(uint16_t *UTF16String)
{
	if (UTF16String != nullptr)
	{
		UTF16String[0] = 0;
	}
}
bool C_4JInput::VerifyStrings(WCHAR **, int iStringC, int (*Func)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		std::vector<HRESULT> resultCodes(iStringC, ERROR_SUCCESS);
		STRING_VERIFY_RESPONSE response = {};
		response.wNumStrings = static_cast<WORD>(iStringC);
		response.pStringResult = resultCodes.data();
		Func(lpParam, &response);
	}
	return true;
}
void C_4JInput::CancelQueuedVerifyStrings(int (*)(LPVOID, STRING_VERIFY_RESPONSE *), LPVOID) {}
void C_4JInput::CancelAllVerifyInProgress(void) {}

C4JStorage StorageManager;

C4JStorage::C4JStorage()
{
	m_pStringTable = nullptr;
}

void C4JStorage::Tick(void) {}
C4JStorage::EMessageResult C4JStorage::RequestMessageBox(UINT, UINT, UINT *, UINT, DWORD, int (*Func)(LPVOID, int, const C4JStorage::EMessageResult), LPVOID lpParam, C4JStringTable *, WCHAR *, DWORD)
{
	if (Func != nullptr)
	{
		Func(lpParam, 0, C4JStorage::EMessage_ResultAccept);
	}
	return C4JStorage::EMessage_ResultAccept;
}
C4JStorage::EMessageResult C4JStorage::GetMessageBoxResult()
{
	return C4JStorage::EMessage_ResultAccept;
}
bool C4JStorage::SetSaveDevice(int (*Func)(LPVOID, const bool), LPVOID lpParam, bool)
{
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
	return true;
}
void C4JStorage::Init(unsigned int, LPCWSTR, char *, int, int (*)(LPVOID, const ESavingMessage, int), LPVOID, LPCSTR) {}
void C4JStorage::ResetSaveData()
{
	s_saveData.clear();
}
void C4JStorage::SetDefaultSaveNameForKeyboardDisplay(LPCWSTR) {}
void C4JStorage::SetSaveTitle(LPCWSTR) {}
bool C4JStorage::GetSaveUniqueNumber(INT *piVal)
{
	if (piVal != nullptr)
	{
		*piVal = s_saveUniqueNumber;
	}
	return true;
}
bool C4JStorage::GetSaveUniqueFilename(char *pszName)
{
	if (pszName != nullptr)
	{
		strcpy_s(pszName, MAX_SAVEFILENAME_LENGTH, s_saveUniqueFilename);
	}
	return true;
}
void C4JStorage::SetSaveUniqueFilename(char *szFilename)
{
	if (szFilename != nullptr)
	{
		strcpy_s(s_saveUniqueFilename, MAX_SAVEFILENAME_LENGTH, szFilename);
	}
}
void C4JStorage::SetState(ESaveGameControlState, int (*Func)(LPVOID, const bool), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
}
void C4JStorage::SetSaveDisabled(bool bDisable)
{
	s_saveDisabled = bDisable;
}
bool C4JStorage::GetSaveDisabled(void)
{
	return s_saveDisabled;
}
unsigned int C4JStorage::GetSaveSize()
{
	return static_cast<unsigned int>(s_saveData.size());
}
void C4JStorage::GetSaveData(void *pvData, unsigned int *puiBytes)
{
	if (puiBytes != nullptr)
	{
		*puiBytes = static_cast<unsigned int>(s_saveData.size());
	}
	if (pvData != nullptr && !s_saveData.empty())
	{
		memcpy(pvData, s_saveData.data(), s_saveData.size());
	}
}
PVOID C4JStorage::AllocateSaveData(unsigned int uiBytes)
{
	s_saveData.assign(uiBytes, 0);
	return s_saveData.empty() ? nullptr : s_saveData.data();
}
void C4JStorage::SetSaveImages(PBYTE, DWORD, PBYTE, DWORD, PBYTE, DWORD) {}
C4JStorage::ESaveGameState C4JStorage::SaveSaveData(int (*Func)(LPVOID, const bool), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
	return C4JStorage::ESaveGame_Idle;
}
void C4JStorage::CopySaveDataToNewSave(PBYTE, DWORD, WCHAR *, int (*Func)(LPVOID, bool), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
}
void C4JStorage::SetSaveDeviceSelected(unsigned int uiPad, bool bSelected)
{
	if (uiPad < s_saveDeviceSelected.size())
	{
		s_saveDeviceSelected[uiPad] = bSelected;
	}
}
bool C4JStorage::GetSaveDeviceSelected(unsigned int iPad)
{
	return iPad < s_saveDeviceSelected.size() ? s_saveDeviceSelected[iPad] : false;
}
C4JStorage::ESaveGameState C4JStorage::DoesSaveExist(bool *pbExists)
{
	if (pbExists != nullptr)
	{
		*pbExists = !s_saveData.empty();
	}
	return C4JStorage::ESaveGame_Idle;
}
bool C4JStorage::EnoughSpaceForAMinSaveGame()
{
	return true;
}
void C4JStorage::SetSaveMessageVPosition(float) {}
C4JStorage::ESaveGameState C4JStorage::GetSavesInfo(int, int (*Func)(LPVOID, SAVE_DETAILS *, const bool), LPVOID lpParam, char *)
{
	if (Func != nullptr)
	{
		Func(lpParam, nullptr, true);
	}
	return C4JStorage::ESaveGame_Idle;
}
PSAVE_DETAILS C4JStorage::ReturnSavesInfo()
{
	return nullptr;
}
void C4JStorage::ClearSavesInfo() {}
C4JStorage::ESaveGameState C4JStorage::LoadSaveDataThumbnail(PSAVE_INFO, int (*Func)(LPVOID, PBYTE, DWORD), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, nullptr, 0);
	}
	return C4JStorage::ESaveGame_GetSaveThumbnail;
}
void C4JStorage::GetSaveCacheFileInfo(DWORD, XCONTENT_DATA &xContentData)
{
	ZeroMemory(&xContentData, sizeof(xContentData));
}
void C4JStorage::GetSaveCacheFileInfo(DWORD, PBYTE *ppbImageData, DWORD *pdwImageBytes)
{
	if (ppbImageData != nullptr)
	{
		*ppbImageData = nullptr;
	}
	if (pdwImageBytes != nullptr)
	{
		*pdwImageBytes = 0;
	}
}
C4JStorage::ESaveGameState C4JStorage::LoadSaveData(PSAVE_INFO, int (*Func)(LPVOID, const bool, const bool), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, true, true);
	}
	return C4JStorage::ESaveGame_Idle;
}
C4JStorage::ESaveGameState C4JStorage::DeleteSaveData(PSAVE_INFO, int (*Func)(LPVOID, const bool), LPVOID lpParam)
{
	s_saveData.clear();
	if (Func != nullptr)
	{
		Func(lpParam, true);
	}
	return C4JStorage::ESaveGame_Idle;
}
void C4JStorage::RegisterMarketplaceCountsCallback(int (*Func)(LPVOID, C4JStorage::DLC_TMS_DETAILS *, int), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		C4JStorage::DLC_TMS_DETAILS details = {};
		Func(lpParam, &details, 0);
	}
}
void C4JStorage::SetDLCPackageRoot(char *) {}
C4JStorage::EDLCStatus C4JStorage::GetDLCOffers(int, int (*Func)(LPVOID, int, DWORD, int), LPVOID lpParam, DWORD)
{
	if (Func != nullptr)
	{
		Func(lpParam, 0, 0, 0);
	}
	return C4JStorage::EDLC_NoOffers;
}
DWORD C4JStorage::CancelGetDLCOffers()
{
	return 0;
}
void C4JStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO &C4JStorage::GetOffer(DWORD)
{
	return s_offerData;
}
int C4JStorage::GetOfferCount()
{
	return 0;
}
DWORD C4JStorage::InstallOffer(int, uint64_t *, int (*Func)(LPVOID, int, int), LPVOID lpParam, bool)
{
	if (Func != nullptr)
	{
		Func(lpParam, 0, 0);
	}
	return 0;
}
DWORD C4JStorage::GetAvailableDLCCount(int)
{
	return 0;
}
C4JStorage::EDLCStatus C4JStorage::GetInstalledDLC(int, int (*Func)(LPVOID, int, int), LPVOID lpParam)
{
	if (Func != nullptr)
	{
		Func(lpParam, 0, 0);
	}
	return C4JStorage::EDLC_NoInstalledDLC;
}
XCONTENT_DATA &C4JStorage::GetDLC(DWORD)
{
	return s_contentData;
}
DWORD C4JStorage::MountInstalledDLC(int, DWORD, int (*Func)(LPVOID, int, DWORD, DWORD), LPVOID lpParam, LPCSTR)
{
	if (Func != nullptr)
	{
		Func(lpParam, 0, 0, 0);
	}
	return 0;
}
DWORD C4JStorage::UnmountInstalledDLC(LPCSTR)
{
	return 0;
}
void C4JStorage::GetMountedDLCFileList(const char *, std::vector<std::string> &fileList)
{
	fileList.clear();
}
std::string C4JStorage::GetMountedPath(std::string szMount)
{
	return szMount;
}
C4JStorage::ETMSStatus C4JStorage::ReadTMSFile(int, eGlobalStorage, C4JStorage::eTMS_FileType, WCHAR *, BYTE **ppBuffer, DWORD *pdwBufferSize, int (*)(LPVOID, WCHAR *, int, bool, int), LPVOID, int)
{
	if (ppBuffer != nullptr)
	{
		*ppBuffer = nullptr;
	}
	if (pdwBufferSize != nullptr)
	{
		*pdwBufferSize = 0;
	}
	return C4JStorage::ETMSStatus_Fail;
}
bool C4JStorage::WriteTMSFile(int, eGlobalStorage, WCHAR *, BYTE *, DWORD)
{
	return true;
}
bool C4JStorage::DeleteTMSFile(int, eGlobalStorage, WCHAR *)
{
	return true;
}
void C4JStorage::StoreTMSPathName(WCHAR *) {}
C4JStorage::ETMSStatus C4JStorage::TMSPP_ReadFile(int, C4JStorage::eGlobalStorage, C4JStorage::eTMS_FILETYPEVAL, LPCSTR, int (*)(LPVOID, int, int, PTMSPP_FILEDATA, LPCSTR), LPVOID, int)
{
	return C4JStorage::ETMSStatus_Fail;
}
unsigned int C4JStorage::CRC(unsigned char *, int)
{
	return 0;
}

extern "C" {

void IggyInit(IggyAllocator *) {}
Iggy *IggyPlayerCreateFromMemory(void const *, U32, IggyPlayerConfig *)
{
	return DummyIggy();
}
IggyLibrary IggyLibraryCreateFromMemoryUTF16(IggyUTF16 const *, void const *, U32, IggyPlayerConfig *)
{
	return 0;
}
void IggyPlayerDestroy(Iggy *) {}
void IggyLibraryDestroy(IggyLibrary) {}
void IggySetWarningCallback(Iggy_WarningFunction *, void *) {}
void IggySetTraceCallbackUTF8(Iggy_TraceFunctionUTF8 *, void *) {}
IggyProperties *IggyPlayerProperties(Iggy *)
{
	return &s_dummyIggyProperties;
}
void *IggyPlayerGetUserdata(Iggy *)
{
	return s_dummyIggyUserdata;
}
void IggyPlayerSetUserdata(Iggy *, void *userdata)
{
	s_dummyIggyUserdata = userdata;
}
void IggyPlayerInitializeAndTickRS(Iggy *)
{
	++s_dummyIggyProperties.frames_passed;
}
rrbool IggyPlayerReadyToTick(Iggy *)
{
	return true;
}
void IggyPlayerTickRS(Iggy *)
{
	++s_dummyIggyProperties.frames_passed;
}
void IggySetCustomDrawCallback(Iggy_CustomDrawCallback *, void *) {}
void IggySetTextureSubstitutionCallbacks(Iggy_TextureSubstitutionCreateCallback *, Iggy_TextureSubstitutionDestroyCallback *, void *) {}
void IggyPlayerSetDisplaySize(Iggy *, S32, S32) {}
void IggyPlayerDraw(Iggy *) {}
void IggyPlayerDrawTile(Iggy *, S32, S32, S32, S32, S32) {}
void IggyPlayerDrawTilesStart(Iggy *) {}
void IggyPlayerDrawTilesEnd(Iggy *) {}
void IggySetFontCachingCalculationBuffer(S32, void *, S32) {}
void IggySetAS3ExternalFunctionCallbackUTF16(Iggy_AS3ExternalFunctionUTF16 *, void *) {}
IggyName IggyPlayerCreateFastName(Iggy *, IggyUTF16 const *, S32)
{
	return 0;
}
IggyResult IggyPlayerCallMethodRS(Iggy *, IggyDataValue *result, IggyValuePath *, IggyName, S32, IggyDataValue *)
{
	if (result != nullptr)
	{
		ZeroMemory(result, sizeof(*result));
	}
	return IGGY_RESULT_SUCCESS;
}
IggyValuePath *IggyPlayerRootPath(Iggy *)
{
	s_dummyValuePath.f = DummyIggy();
	s_dummyValuePath.parent = nullptr;
	s_dummyValuePath.name = 0;
	s_dummyValuePath.ref = nullptr;
	s_dummyValuePath.index = 0;
	s_dummyValuePath.type = 0;
	return &s_dummyValuePath;
}
rrbool IggyValuePathMakeNameRef(IggyValuePath *result, IggyValuePath *parent, char const *)
{
	if (result != nullptr)
	{
		if (parent != nullptr)
		{
			*result = *parent;
			result->parent = parent;
		}
		else
		{
			ZeroMemory(result, sizeof(*result));
			result->f = DummyIggy();
		}
	}
	return true;
}
IggyResult IggyValueGetF64RS(IggyValuePath *, IggyName, char const *, F64 *result)
{
	if (result != nullptr)
	{
		*result = 0.0;
	}
	return IGGY_RESULT_SUCCESS;
}
IggyResult IggyValueGetBooleanRS(IggyValuePath *, IggyName, char const *, rrbool *result)
{
	if (result != nullptr)
	{
		*result = false;
	}
	return IGGY_RESULT_SUCCESS;
}
rrbool IggyValueSetBooleanRS(IggyValuePath *, IggyName, char const *, rrbool)
{
	return true;
}
void IggyMakeEventMouseMove(IggyEvent *event, S32, S32)
{
	if (event != nullptr)
	{
		ZeroMemory(event, sizeof(*event));
	}
}
void IggyMakeEventKey(IggyEvent *event, IggyKeyevent, IggyKeycode, IggyKeyloc)
{
	if (event != nullptr)
	{
		ZeroMemory(event, sizeof(*event));
	}
}
void IggyMakeEventFocusLost(IggyEvent *event)
{
	if (event != nullptr)
	{
		ZeroMemory(event, sizeof(*event));
	}
}
void IggyMakeEventFocusGained(IggyEvent *event, S32)
{
	if (event != nullptr)
	{
		ZeroMemory(event, sizeof(*event));
	}
}
rrbool IggyPlayerDispatchEventRS(Iggy *, IggyEvent *, IggyEventResult *result)
{
	if (result != nullptr)
	{
		ZeroMemory(result, sizeof(*result));
	}
	return true;
}
rrbool IggyPlayerGetFocusableObjects(Iggy *, IggyFocusHandle *current_focus, IggyFocusableObject *objs, S32 max_obj, S32 *num_obj)
{
	if (current_focus != nullptr)
	{
		*current_focus = IGGY_FOCUS_NULL;
	}
	if (num_obj != nullptr)
	{
		*num_obj = 0;
	}
	if (objs != nullptr && max_obj > 0)
	{
		objs[0] = s_dummyFocusableObject;
	}
	return true;
}
void IggyPlayerSetFocusRS(Iggy *, IggyFocusHandle, int) {}
rrbool IggyDebugGetMemoryUseInfo(Iggy *, IggyLibrary, char const *, S32, S32, IggyMemoryUseInfo *data)
{
	if (data != nullptr)
	{
		ZeroMemory(data, sizeof(*data));
	}
	return false;
}

}

void glTexGen(int coord, int mode, FloatBuffer *values)
{
	if (values == nullptr || values->_getDataPointer() == nullptr)
	{
		return;
	}

	float *data = values->_getDataPointer();
	RenderManager.StateSetTexGenCol(coord, data[0], data[1], data[2], data[3], mode == GL_EYE_PLANE);
}
void glReadPixels(int x, int y, int width, int height, int format, int type, ByteBuffer *buffer)
{
	if (buffer == nullptr)
	{
		return;
	}

	::glReadPixels(x, y, width, height, format, type, buffer->getBuffer());
}
void glGenTextures(IntBuffer *ib)
{
	if (ib == nullptr)
	{
		return;
	}

	for (unsigned int i = 0; i < ib->limit(); ++i)
	{
		ib->put(RenderManager.TextureCreate());
	}
}
void glLight(int light, int mode, FloatBuffer *values)
{
	if (values == nullptr || values->_getDataPointer() == nullptr)
	{
		return;
	}

	const int lightIndex = (light == GL_LIGHT0) ? 0 : ((light == GL_LIGHT1) ? 1 : -1);
	if (lightIndex < 0)
	{
		return;
	}

	float *data = values->_getDataPointer();
	if (mode == GL_POSITION)
	{
		RenderManager.StateSetLightDirection(lightIndex, data[0], data[1], data[2]);
	}
	else if (mode == GL_DIFFUSE)
	{
		RenderManager.StateSetLightColour(lightIndex, data[0], data[1], data[2]);
	}
}
void glLightModel(int mode, FloatBuffer *values)
{
	if (mode != GL_LIGHT_MODEL_AMBIENT || values == nullptr || values->_getDataPointer() == nullptr)
	{
		return;
	}

	float *data = values->_getDataPointer();
	RenderManager.StateSetLightAmbientColour(data[0], data[1], data[2]);
}
void glGetFloat(int type, FloatBuffer *buffer)
{
	if (buffer == nullptr || buffer->_getDataPointer() == nullptr)
	{
		return;
	}

	memcpy(buffer->_getDataPointer(), RenderManager.MatrixGet(type), sizeof(float) * 16);
}
void glTexCoordPointer(int size, int stride, FloatBuffer *buffer)
{
	if (buffer == nullptr)
	{
		return;
	}

	::glTexCoordPointer(size, GL_FLOAT, stride, buffer->_getDataPointer());
}
void glNormalPointer(int stride, ByteBuffer *buffer)
{
	if (buffer == nullptr)
	{
		return;
	}

	::glNormalPointer(GL_BYTE, stride, buffer->getBuffer());
}
void glColorPointer(int size, bool, int stride, ByteBuffer *buffer)
{
	if (buffer == nullptr)
	{
		return;
	}

	::glColorPointer(size, GL_UNSIGNED_BYTE, stride, buffer->getBuffer());
}
void glVertexPointer(int size, int stride, FloatBuffer *buffer)
{
	if (buffer == nullptr)
	{
		return;
	}

	::glVertexPointer(size, GL_FLOAT, stride, buffer->_getDataPointer());
}
void glTexImage2D(int, int level, int, int width, int height, int, int, int, ByteBuffer *data)
{
	if (data == nullptr)
	{
		return;
	}

	RenderManager.TextureData(width, height, data->getBuffer(), level);
}
void glDeleteTextures(IntBuffer *ib)
{
	if (ib == nullptr)
	{
		return;
	}

	for (unsigned int i = 0; i < ib->limit(); ++i)
	{
		RenderManager.TextureFree(ib->get(i));
	}
}
void glCallLists(IntBuffer *ib)
{
	if (ib == nullptr)
	{
		return;
	}

	for (unsigned int i = 0; i < ib->limit(); ++i)
	{
		RenderManager.CBuffCall(ib->get(i));
	}
}
void glFog(int param, FloatBuffer *values)
{
	if (param != GL_FOG_COLOR || values == nullptr || values->_getDataPointer() == nullptr)
	{
		return;
	}

	float *data = values->_getDataPointer();
	RenderManager.StateSetFogColour(data[0], data[1], data[2]);
}
void gluPerspective(float fovy, float aspect, float zNear, float zFar)
{
	RenderManager.MatrixPerspective(fovy, aspect, zNear, zFar);
}

void Display::update() {}
void Display::swapBuffers()
{
	if (g_pWindow != nullptr)
	{
		SDL_GL_SwapWindow(g_pWindow);
	}
}

void DefineActions() {}
void MemSect(int) {}
void IQNet_DispatchPacket(unsigned char, unsigned char, unsigned char *, unsigned int) {}

const int AddEntityPacket::ARROW;
const int AddEntityPacket::BOAT;
const int AddEntityPacket::EGG;
const int AddEntityPacket::ENDER_CRYSTAL;
const int AddEntityPacket::EYEOFENDERSIGNAL;
const int AddEntityPacket::FALLING;
const int AddEntityPacket::FIREWORKS;
const int AddEntityPacket::FISH_HOOK;
const int AddEntityPacket::ITEM;
const int AddEntityPacket::ITEM_FRAME;
const int AddEntityPacket::LEASH_KNOT;
const int AddEntityPacket::MINECART;
const int AddEntityPacket::PRIMED_TNT;
const int AddEntityPacket::SNOWBALL;
const int AddEntityPacket::THROWN_ENDERPEARL;
const int AddEntityPacket::THROWN_EXPBOTTLE;
const int AddEntityPacket::THROWN_POTION;

const int AnimatePacket::CRITICAL_HIT;
const int AnimatePacket::EAT;
const int AnimatePacket::MAGIC_CRITICAL_HIT;
const int AnimatePacket::SWING;
const int AnimatePacket::WAKE_UP;

const int ChestTile::TYPE_TRAP;
const int ClientCommandPacket::PERFORM_RESPAWN;
const int ContainerOpenPacket::BEACON;
const int ContainerOpenPacket::BREWING_STAND;
const int ContainerOpenPacket::DROPPER;
const int ContainerOpenPacket::ENCHANTMENT;
const int ContainerOpenPacket::FIREWORKS;
const int ContainerOpenPacket::FURNACE;
const int ContainerOpenPacket::HOPPER;
const int ContainerOpenPacket::HORSE;
const int ContainerOpenPacket::REPAIR_TABLE;
const int ContainerOpenPacket::TRADER_NPC;
const int ContainerOpenPacket::TRAP;
const int ContainerOpenPacket::WORKBENCH;
const BYTE EntityEvent::USE_ITEM_COMPLETE;
const int GameEventPacket::SUCCESSFUL_BOW_HIT;
const int Level::maxBuildHeight;
const int PotionBrewing::POTION_ID_SPLASH_DAMAGE;
const int QuartzBlockTile::TYPE_LINES_Y;
const int Sapling::TYPE_BIRCH;
const int Sapling::TYPE_DEFAULT;
const int Sapling::TYPE_EVERGREEN;
const int Sapling::TYPE_JUNGLE;
const int SetEntityLinkPacket::LEASH;
const int SetEntityLinkPacket::RIDING;
const int SharedConstants::NETWORK_PROTOCOL_VERSION;
const int SkullTileEntity::TYPE_WITHER;
const int TallGrass::FERN;
const int TileEditorOpenPacket::SIGN;
const int TileEntityDataPacket::TYPE_ADV_COMMAND;
const int TileEntityDataPacket::TYPE_BEACON;
const int TileEntityDataPacket::TYPE_MOB_SPAWNER;
const int TileEntityDataPacket::TYPE_SKULL;

// ============================================================
// Missing static constant definitions — ODR-use requires these
// ============================================================

#include "Minecraft.World/RemoveEntitiesPacket.h"
#include "Minecraft.World/Item.h"
#include "Minecraft.World/Tile.h"
#include "Minecraft.World/net.minecraft.world.level.tile.h"

// RemoveEntitiesPacket
const int RemoveEntitiesPacket::MAX_PER_PACKET;

// IUIScene_CreativeMenu::TabSpec
#include "Common/UI/IUIScene_CreativeMenu.h"
const int IUIScene_CreativeMenu::TabSpec::MAX_SIZE;

// Item static IDs (ODR-used by Villager, BucketItem, LeafTile, etc.)
const int Item::apple_Id;
const int Item::arrow_Id;
const int Item::beef_cooked_Id;
const int Item::beef_raw_Id;
const int Item::boat_Id;
const int Item::book_Id;
const int Item::boots_chain_Id;
const int Item::boots_diamond_Id;
const int Item::boots_iron_Id;
const int Item::boots_leather_Id;
const int Item::bread_Id;
const int Item::bucket_lava_Id;
const int Item::bucket_water_Id;
const int Item::chestplate_chain_Id;
const int Item::chestplate_diamond_Id;
const int Item::chestplate_iron_Id;
const int Item::chestplate_leather_Id;
const int Item::chicken_cooked_Id;
const int Item::chicken_raw_Id;
const int Item::clock_Id;
const int Item::coal_Id;
const int Item::compass_Id;
const int Item::cookie_Id;
const int Item::diamond_Id;
const int Item::emerald_Id;
const int Item::enderPearl_Id;
const int Item::expBottle_Id;
const int Item::eyeOfEnder_Id;
const int Item::fish_cooked_Id;
const int Item::fish_raw_Id;
const int Item::fishingRod_Id;
const int Item::flintAndSteel_Id;
const int Item::flint_Id;
const int Item::goldIngot_Id;
const int Item::hatchet_diamond_Id;
const int Item::hatchet_iron_Id;
const int Item::hatchet_wood_Id;
const int Item::helmet_chain_Id;
const int Item::helmet_diamond_Id;
const int Item::helmet_iron_Id;
const int Item::helmet_leather_Id;
const int Item::hoe_diamond_Id;
const int Item::hoe_iron_Id;
const int Item::ironIngot_Id;
const int Item::leggings_chain_Id;
const int Item::leggings_diamond_Id;
const int Item::leggings_iron_Id;
const int Item::leggings_leather_Id;
const int Item::map_Id;
const int Item::melon_Id;
const int Item::minecart_Id;
const int Item::paper_Id;
const int Item::pickAxe_diamond_Id;
const int Item::pickAxe_iron_Id;
const int Item::pickAxe_wood_Id;
const int Item::porkChop_cooked_Id;
const int Item::porkChop_raw_Id;
const int Item::potion_Id;
const int Item::redStone_Id;
const int Item::rotten_flesh_Id;
const int Item::saddle_Id;
const int Item::seeds_melon_Id;
const int Item::seeds_pumpkin_Id;
const int Item::seeds_wheat_Id;
const int Item::shears_Id;
const int Item::shovel_diamond_Id;
const int Item::shovel_iron_Id;
const int Item::shovel_wood_Id;
const int Item::skull_Id;
const int Item::stick_Id;
const int Item::sword_diamond_Id;
const int Item::sword_iron_Id;
const int Item::wheat_Id;

// Tile static IDs
const int Tile::bookshelf_Id;
const int Tile::furnace_Id;
const int Tile::glass_Id;
const int Tile::glowstone_Id;
const int Tile::goldenRail_Id;
const int Tile::leaves_Id;
const int Tile::rail_Id;
const int Tile::stone_Id;
const int Tile::stoneSlabHalf_Id;
const int Tile::torch_Id;
const int Tile::wood_Id;
const int Tile::wool_Id;
const int Tile::workBench_Id;

// ShutdownManager — header included via stdafx.h, inline bodies should resolve

// ============================================================
// SDL2 global window pointer (referenced by UIScene_AbstractContainerMenu)
// ============================================================
SDL_Window* g_pSDLWindow = nullptr;

// ============================================================
// SoundEngine music path
// ============================================================
char SoundEngine::m_szMusicPath[512] = {};
