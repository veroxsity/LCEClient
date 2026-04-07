#ifdef _LINUX64
#include "stdafx.h"

#include "Linux64/Iggy/include/iggy.h"
#include "../Windows64/Iggy/gdraw/gdraw_wgl.h"

#include <cstdio>
#include <cstring>

#define GDRAW_ASSERTS
#define GDRAW_MULTISAMPLING
#define GDRAW_USE_glGetStringi
#define GDrawGLProgram GLuint
#define gdraw_GLx_(id) gdraw_GL_##id
#define GDRAW_GLx_(id) GDRAW_GL_##id
#define GDRAW_SHADERS "gdraw_gl_shaders.inl"

static void load_extensions(void)
{
	// Linux uses GLEW for function loading before UI startup.
}

static void clear_renderstate_platform_specific(void)
{
	glDisable(GL_ALPHA_TEST);
}

static void error_msg_platform_specific(const char *msg)
{
	if (msg != nullptr)
	{
		std::fprintf(stderr, "[Linux64][IggyGL] %s\n", msg);
	}
}

#include "../Windows64/Iggy/gdraw/gdraw_gl_shared.inl"

GDrawFunctions *gdraw_GL_CreateContext(S32 w, S32 h, S32 msaa_samples)
{
	static const TextureFormatDesc tex_formats[] = {
		{ IFT_FORMAT_rgba_8888,    1, 1,  4,   GL_RGBA,                          GL_RGBA,            GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_rgba_4444_LE, 1, 1,  2,   GL_RGBA4,                         GL_RGBA,            GL_UNSIGNED_SHORT_4_4_4_4 },
		{ IFT_FORMAT_rgba_5551_LE, 1, 1,  2,   GL_RGB5_A1,                       GL_RGBA,            GL_UNSIGNED_SHORT_5_5_5_1 },
		{ IFT_FORMAT_la_88,        1, 1,  2,   GL_LUMINANCE8_ALPHA8,             GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_la_44,        1, 1,  1,   GL_LUMINANCE4_ALPHA4,             GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_i_8,          1, 1,  1,   GL_INTENSITY8,                    GL_ALPHA,           GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_i_4,          1, 1,  1,   GL_INTENSITY4,                    GL_ALPHA,           GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_l_8,          1, 1,  1,   GL_LUMINANCE8,                    GL_LUMINANCE,       GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_l_4,          1, 1,  1,   GL_LUMINANCE4,                    GL_LUMINANCE,       GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_DXT1,         4, 4,  8,   GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0,                  GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_DXT3,         4, 4, 16,   GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0,                  GL_UNSIGNED_BYTE },
		{ IFT_FORMAT_DXT5,         4, 4, 16,   GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0,                  GL_UNSIGNED_BYTE },
		{ 0,                       0, 0,  0,   0,                                0,                  0 },
	};

	GDrawFunctions *funcs = nullptr;
	const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	GLint maxValue = 0;

	if (!hasext(extensions, "GL_ARB_multitexture") ||
		!hasext(extensions, "GL_ARB_texture_compression") ||
		!hasext(extensions, "GL_ARB_texture_mirrored_repeat") ||
		!hasext(extensions, "GL_ARB_texture_non_power_of_two") ||
		!hasext(extensions, "GL_ARB_vertex_buffer_object") ||
		!hasext(extensions, "GL_EXT_framebuffer_object") ||
		!hasext(extensions, "GL_ARB_shader_objects") ||
		!hasext(extensions, "GL_ARB_vertex_shader") ||
		!hasext(extensions, "GL_ARB_fragment_shader"))
	{
		return nullptr;
	}

	if (!hasext(extensions, "GL_EXT_framebuffer_multisample") && msaa_samples > 1)
	{
		return nullptr;
	}

	load_extensions();
	funcs = create_context(w, h);
	if (funcs == nullptr)
	{
		return nullptr;
	}

	gdraw->tex_formats = tex_formats;
	gdraw->has_mapbuffer = true;
	gdraw->has_depth24 = true;
	gdraw->has_texture_max_level = true;

	if (hasext(extensions, "GL_EXT_packed_depth_stencil"))
	{
		gdraw->has_packed_depth_stencil = true;
	}

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxValue);
	gdraw->has_conditional_non_power_of_two = maxValue < 8192;

	if (msaa_samples > 1)
	{
		glGetIntegerv(GL_MAX_SAMPLES, &maxValue);
		gdraw->multisampling = RR_MIN(msaa_samples, maxValue);
	}

	opengl_check();
	return funcs;
}

#endif
