#include "stdafx.h"

#include <array>
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

namespace
{
constexpr std::array<float, 16> kIdentityMatrix = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

std::array<float, 16> s_modelMatrix = kIdentityMatrix;
std::array<float, 16> s_projectionMatrix = kIdentityMatrix;
std::array<float, 16> s_textureMatrix = kIdentityMatrix;
int s_currentMatrixMode = 0;
int s_nextCommandBufferId = 1;
int s_nextTextureId = 1;
int s_textureLevels = 1;
bool s_renderSuspended = false;

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

void SetIdentity(std::array<float, 16> &matrix)
{
	matrix = kIdentityMatrix;
}

void CopyIdentity(FloatBuffer *buffer)
{
	if (buffer == nullptr || buffer->_getDataPointer() == nullptr)
	{
		return;
	}

	memcpy(buffer->_getDataPointer(), kIdentityMatrix.data(), sizeof(float) * kIdentityMatrix.size());
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
void C4JRender::MatrixMode(int type) { s_currentMatrixMode = type; }
void C4JRender::MatrixSetIdentity()
{
	if (s_currentMatrixMode == GL_PROJECTION || s_currentMatrixMode == GL_PROJECTION_MATRIX)
	{
		SetIdentity(s_projectionMatrix);
		return;
	}

	if (s_currentMatrixMode == GL_TEXTURE)
	{
		SetIdentity(s_textureMatrix);
		return;
	}

	SetIdentity(s_modelMatrix);
}
void C4JRender::MatrixTranslate(float, float, float) {}
void C4JRender::MatrixRotate(float, float, float, float) {}
void C4JRender::MatrixScale(float, float, float) {}
void C4JRender::MatrixPerspective(float, float, float, float) {}
void C4JRender::MatrixOrthogonal(float, float, float, float, float, float) {}
void C4JRender::MatrixPop() {}
void C4JRender::MatrixPush() {}
void C4JRender::MatrixMult(float *) {}
const float *C4JRender::MatrixGet(int type)
{
	if (type == GL_PROJECTION || type == GL_PROJECTION_MATRIX)
	{
		return s_projectionMatrix.data();
	}

	if (type == GL_TEXTURE)
	{
		return s_textureMatrix.data();
	}

	return s_modelMatrix.data();
}
void C4JRender::Set_matrixDirty() {}
void C4JRender::Initialise(ID3D11Device *, IDXGISwapChain *) {}
void C4JRender::InitialiseContext() {}
void C4JRender::StartFrame() {}
void C4JRender::DoScreenGrabOnNextPresent() {}
void C4JRender::Present() {}
void C4JRender::Clear(int, D3D11_RECT *) {}
void C4JRender::SetClearColour(const float[4]) {}
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
void C4JRender::DrawVertices(C4JRender::ePrimitiveType, int, void *, C4JRender::eVertexType, C4JRender::ePixelShaderType) {}
void C4JRender::DrawVertexBuffer(C4JRender::ePrimitiveType, int, ID3D11Buffer *, C4JRender::eVertexType, C4JRender::ePixelShaderType) {}
void C4JRender::CBuffLockStaticCreations() {}
int C4JRender::CBuffCreate(int count)
{
	const int first = s_nextCommandBufferId;
	s_nextCommandBufferId += (count > 0) ? count : 1;
	return first;
}
void C4JRender::CBuffDelete(int, int) {}
void C4JRender::CBuffStart(int, bool) {}
void C4JRender::CBuffClear(int) {}
int C4JRender::CBuffSize(int) { return 0; }
void C4JRender::CBuffEnd() {}
bool C4JRender::CBuffCall(int, bool) { return false; }
void C4JRender::CBuffTick() {}
void C4JRender::CBuffDeferredModeStart() {}
void C4JRender::CBuffDeferredModeEnd() {}
int C4JRender::TextureCreate() { return s_nextTextureId++; }
void C4JRender::TextureFree(int) {}
void C4JRender::TextureBind(int) {}
void C4JRender::TextureBindVertex(int) {}
void C4JRender::TextureSetTextureLevels(int levels) { s_textureLevels = levels; }
int C4JRender::TextureGetTextureLevels() { return s_textureLevels; }
void C4JRender::TextureData(int, int, void *, int, eTextureFormat) {}
void C4JRender::TextureDataUpdate(int, int, int, int, void *, int) {}
void C4JRender::TextureSetParam(int, int) {}
void C4JRender::TextureDynamicUpdateStart() {}
void C4JRender::TextureDynamicUpdateEnd() {}
HRESULT C4JRender::LoadTextureData(const char *, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
	// Phase 2 stub: return a 1x1 magenta dummy pixel so texture loading
	// doesn't assert on zero-size arrays. Real loading in Phase 3.
	if (pSrcInfo != nullptr)
	{
		ZeroMemory(pSrcInfo, sizeof(*pSrcInfo));
		pSrcInfo->Width = 1;
		pSrcInfo->Height = 1;
	}
	if (ppDataOut != nullptr)
	{
		int *pixel = new int[1];
		pixel[0] = static_cast<int>(0xFFFF00FF); // ARGB magenta
		*ppDataOut = pixel;
	}
	return ERROR_SUCCESS;
}
HRESULT C4JRender::LoadTextureData(BYTE *, DWORD, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
	// Phase 2 stub: return a 1x1 magenta dummy pixel
	if (pSrcInfo != nullptr)
	{
		ZeroMemory(pSrcInfo, sizeof(*pSrcInfo));
		pSrcInfo->Width = 1;
		pSrcInfo->Height = 1;
	}
	if (ppDataOut != nullptr)
	{
		int *pixel = new int[1];
		pixel[0] = static_cast<int>(0xFFFF00FF); // ARGB magenta
		*ppDataOut = pixel;
	}
	return ERROR_SUCCESS;
}
HRESULT C4JRender::SaveTextureData(const char *, D3DXIMAGE_INFO *, int *) { return E_FAIL; }
HRESULT C4JRender::SaveTextureDataToMemory(void *, int, int *, int, int, int *) { return E_FAIL; }
void C4JRender::TextureGetStats() {}
ID3D11ShaderResourceView *C4JRender::TextureGetTexture(int) { return nullptr; }
void C4JRender::StateSetColour(float, float, float, float) {}
void C4JRender::StateSetDepthMask(bool) {}
void C4JRender::StateSetBlendEnable(bool) {}
void C4JRender::StateSetBlendFunc(int, int) {}
void C4JRender::StateSetBlendFactor(unsigned int) {}
void C4JRender::StateSetAlphaFunc(int, float) {}
void C4JRender::StateSetDepthFunc(int) {}
void C4JRender::StateSetFaceCull(bool) {}
void C4JRender::StateSetFaceCullCW(bool) {}
void C4JRender::StateSetLineWidth(float) {}
void C4JRender::StateSetWriteEnable(bool, bool, bool, bool) {}
void C4JRender::StateSetDepthTestEnable(bool) {}
void C4JRender::StateSetAlphaTestEnable(bool) {}
void C4JRender::StateSetDepthSlopeAndBias(float, float) {}
void C4JRender::StateSetFogEnable(bool) {}
void C4JRender::StateSetFogMode(int) {}
void C4JRender::StateSetFogNearDistance(float) {}
void C4JRender::StateSetFogFarDistance(float) {}
void C4JRender::StateSetFogDensity(float) {}
void C4JRender::StateSetFogColour(float, float, float) {}
void C4JRender::StateSetLightingEnable(bool) {}
void C4JRender::StateSetVertexTextureUV(float, float) {}
void C4JRender::StateSetLightColour(int, float, float, float) {}
void C4JRender::StateSetLightAmbientColour(float, float, float) {}
void C4JRender::StateSetLightDirection(int, float, float, float) {}
void C4JRender::StateSetLightEnable(int, bool) {}
void C4JRender::StateSetViewport(C4JRender::eViewportType) {}
void C4JRender::StateSetEnableViewportClipPlanes(bool) {}
void C4JRender::StateSetTexGenCol(int, float, float, float, float, bool) {}
void C4JRender::StateSetStencil(int, uint8_t, uint8_t, uint8_t) {}
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

void glTexGen(int, int, FloatBuffer *) {}
void glReadPixels(int, int, int, int, int, int, ByteBuffer *) {}
void glGenTextures(IntBuffer *ib)
{
	if (ib == nullptr)
	{
		return;
	}

	for (unsigned int i = 0; i < ib->limit(); ++i)
	{
		ib->put(static_cast<int>(s_nextTextureId++));
	}
}
void glLight(int, int, FloatBuffer *) {}
void glLightModel(int, FloatBuffer *) {}
void glGetFloat(int, FloatBuffer *buffer)
{
	CopyIdentity(buffer);
}
void glTexCoordPointer(int, int, FloatBuffer *) {}
void glNormalPointer(int, ByteBuffer *) {}
void glColorPointer(int, bool, int, ByteBuffer *) {}
void glVertexPointer(int, int, FloatBuffer *) {}
void glTexImage2D(int, int, int, int, int, int, int, int, ByteBuffer *) {}
void glDeleteTextures(IntBuffer *) {}
void glCallLists(IntBuffer *) {}
void glFog(int, FloatBuffer *) {}
void gluPerspective(float, float, float, float) {}

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
