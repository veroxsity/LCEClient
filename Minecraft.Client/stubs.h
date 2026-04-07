#pragma once

// GL constants — on Windows these are fake stubs, on Linux GLEW provides real ones
#ifndef _LINUX64

const int GL_BYTE = 0;
const int GL_FLOAT = 0;
const int GL_UNSIGNED_BYTE = 0;

const int GL_COLOR_ARRAY = 0;
const int GL_VERTEX_ARRAY = 0;
const int GL_NORMAL_ARRAY = 0;
const int GL_TEXTURE_COORD_ARRAY = 0;

const int GL_COMPILE = 0;
const int GL_NORMALIZE = 0;
const int GL_RESCALE_NORMAL = 0;

const int GL_SMOOTH = 0;
const int GL_FLAT = 0;

const int GL_RGBA = 0;
const int GL_BGRA = 1;
const int GL_BGR = 0;

const int GL_SAMPLES_PASSED_ARB = 0;
const int GL_QUERY_RESULT_AVAILABLE_ARB = 0;
const int GL_QUERY_RESULT_ARB = 0;

const int GL_POLYGON_OFFSET_FILL = 0;

const int GL_FRONT = 0;
const int GL_BACK = 1;
const int GL_FRONT_AND_BACK = 2;

const int GL_COLOR_MATERIAL = 0;
const int GL_AMBIENT_AND_DIFFUSE = 0;

const int GL_TEXTURE1 = 0;
const int GL_TEXTURE0 = 1;

#endif // !_LINUX64

// On Linux64, GLEW (included via stdafx.h) provides all real GL function declarations.
// We only need the project-specific wrappers that take FloatBuffer*/ByteBuffer*/IntBuffer*.
// On Windows/other, we stub everything since there's no real GL.

#ifndef _LINUX64

void glFlush();
void glTexGeni(int,int,int);
void glTexGen(int,int,FloatBuffer *);
void glReadPixels(int,int, int, int, int, int, ByteBuffer *);
void glClearDepth(double);
void glCullFace(int);
void glDeleteLists(int,int);
void glGenTextures(IntBuffer *);
int glGenTextures();
int glGenLists(int);
void glLight(int, int,FloatBuffer *);
void glLightModel(int, FloatBuffer *);
void glGetFloat(int a, FloatBuffer *b);
void glTexCoordPointer(int, int, int, int);
void glTexCoordPointer(int, int, FloatBuffer *);
void glNormalPointer(int, int, int);
void glNormalPointer(int, ByteBuffer *);
void glEnableClientState(int);
void glDisableClientState(int);
void glColorPointer(int, bool, int, ByteBuffer *);
void glColorPointer(int, int, int, int);
void glVertexPointer(int, int, int, int);
void glVertexPointer(int, int, FloatBuffer *);
void glDrawArrays(int,int,int);
void glTranslatef(float,float,float);
void glRotatef(float,float,float,float);
void glNewList(int,int);
void glEndList(int vertexCount = 0);
void glCallList(int);
void glPopMatrix();
void glPushMatrix();
void glColor3f(float,float,float);
void glScalef(float,float,float);
void glMultMatrixf(float *);
void glColor4f(float,float,float,float);
void glDisable(int);
void glEnable(int);
void glBlendFunc(int,int);
void glDepthMask(bool);
void glNormal3f(float,float,float);
void glDepthFunc(int);
void glMatrixMode(int);
void glLoadIdentity();
void glBindTexture(int,int);
void glTexParameteri(int,int,int);
void glTexImage2D(int,int,int,int,int,int,int,int,ByteBuffer *);
void glDeleteTextures(IntBuffer *);
void glDeleteTextures(int);
void glCallLists(IntBuffer *);
void glGenQueriesARB(IntBuffer *);
void glColorMask(bool,bool,bool,bool);
void glBeginQueryARB(int,int);
void glEndQueryARB(int);
void glGetQueryObjectuARB(int,int,IntBuffer *);
void glShadeModel(int);
void glPolygonOffset(float,float);
void glLineWidth(float);
void glScaled(double,double,double);
void gluPerspective(float,float,float,float);
void glClear(int);
void glViewport(int,int,int,int);
void glAlphaFunc(int,float);
void glOrtho(float,float,float,float,float,float);
void glClearColor(float,float,float,float);
void glFogi(int,int);
void glFogf(int,float);
void glFog(int,FloatBuffer *);
void glColorMaterial(int,int);
void glMultiTexCoord2f(int, float, float);
void glClientActiveTexture(int);
void glActiveTexture(int);

#else // _LINUX64: GLEW provides real GL functions. Only declare project-specific wrappers.

// Custom overloads taking project buffer types — signatures that don't exist in GL/GLEW
void glTexGen(int,int,FloatBuffer *);
void glReadPixels(int,int,int,int,int,int,ByteBuffer *);
void glGenTextures(IntBuffer *);
void glLight(int,int,FloatBuffer *);
void glLightModel(int,FloatBuffer *);
void glGetFloat(int a, FloatBuffer *b);
void glTexCoordPointer(int,int,FloatBuffer *);
void glNormalPointer(int,ByteBuffer *);
void glColorPointer(int,bool,int,ByteBuffer *);
void glVertexPointer(int,int,FloatBuffer *);
void glTexImage2D(int,int,int,int,int,int,int,int,ByteBuffer *);
void glDeleteTextures(IntBuffer *);
void glCallLists(IntBuffer *);
void glFog(int,FloatBuffer *);

// gluPerspective is not in GLEW (it's GLU) — keep our float version
void gluPerspective(float,float,float,float);

// Convenience wrappers matching the Windows single-ID overloads.
// The codebase calls these everywhere; on Linux the real GL functions
// take (count, pointer) pairs instead.
inline int glGenTextures()
{
	GLuint id;
	glGenTextures(1, &id);
	return static_cast<int>(id);
}

inline void glDeleteTextures(int id)
{
	GLuint uid = static_cast<GLuint>(id);
	glDeleteTextures(1, &uid);
}

// These are provided by GLEW as macro->function-pointer, so no declaration needed:
// glGenQueriesARB, glBeginQueryARB, glEndQueryARB, glGetQueryObjectuARB
// glClientActiveTexture, glActiveTexture, glMultiTexCoord2f
// glOrtho, glAlphaFunc, glColorMask, glDrawArrays, glFogi, glFogf
// glTexCoordPointer(int,int,int,int), glNormalPointer(int,int,int)
// glEnableClientState, glDisableClientState, glColorPointer(int,int,int,int)
// glVertexPointer(int,int,int,int), glMultMatrixf

// These are provided by GLEW as macro->function-pointer, so no declaration needed:
// glGenQueriesARB, glBeginQueryARB, glEndQueryARB, glGetQueryObjectuARB
// glClientActiveTexture, glActiveTexture, glMultiTexCoord2f

#endif // _LINUX64

#ifdef _LINUX64

// Linux uses the platform RenderManager implementation directly for the legacy
// OpenGL-style helpers that the codebase expects. This keeps the other
// platforms untouched while letting Linux record/replay command buffers.
#define glTranslatef(x, y, z) RenderManager.MatrixTranslate((x), (y), (z))
#define glRotatef(angle, x, y, z) RenderManager.MatrixRotate((angle) * (PI / 180.0f), (x), (y), (z))
#define glPopMatrix() RenderManager.MatrixPop()
#define glPushMatrix() RenderManager.MatrixPush()
#define glScalef(x, y, z) RenderManager.MatrixScale((x), (y), (z))
#define glMultMatrixf(m) RenderManager.MatrixMult((m))
#define glMatrixMode(type) RenderManager.MatrixMode((type))
#define glLoadIdentity() RenderManager.MatrixSetIdentity()
#define glDeleteLists(first, count) RenderManager.CBuffDelete((first), (count))
#define glGenLists(count) RenderManager.CBuffCreate((count))
#define glNewList(index, mode) RenderManager.CBuffStart((index))
#define glEndList(...) RenderManager.CBuffEnd()
#define glCallList(index) static_cast<void>(RenderManager.CBuffCall((index)))
#define glClear(flags) RenderManager.Clear((flags))
#define glClearColor(r, g, b, a) do { float _rgba[4] = { (r), (g), (b), (a) }; RenderManager.SetClearColour(_rgba); } while (0)
#define glBindTexture(target, texture) RenderManager.TextureBind((texture))
#define glColor3f(r, g, b) RenderManager.StateSetColour((r), (g), (b), 1.0f)
#define glColor4f(r, g, b, a) RenderManager.StateSetColour((r), (g), (b), (a))
#define glDepthMask(enable) RenderManager.StateSetDepthMask((enable))
#define glBlendFunc(src, dst) RenderManager.StateSetBlendFunc((src), (dst))
#define glDepthFunc(func) RenderManager.StateSetDepthFunc((func))
#define glTexParameteri(target, param, value) RenderManager.TextureSetParam((param), (value))
#define glPolygonOffset(factor, units) RenderManager.StateSetDepthSlopeAndBias((factor) / 65536.0f, (units) / 65536.0f)
#define glLineWidth(width) RenderManager.StateSetLineWidth((width))
#define glColorMask(red, green, blue, alpha) RenderManager.StateSetWriteEnable((red), (green), (blue), (alpha))
#define glScaled(x, y, z) RenderManager.MatrixScale(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z))
#define glAlphaFunc(func, param) RenderManager.StateSetAlphaFunc((func), (param))
#define glOrtho(left, right, bottom, top, zNear, zFar) RenderManager.MatrixOrthogonal((left), (right), (bottom), (top), (zNear), (zFar))
#define glFogi(param, value) do { if ((param) == GL_FOG_MODE) RenderManager.StateSetFogMode((value)); } while (0)
#define glFogf(param, value) do { \
	if ((param) == GL_FOG_START) RenderManager.StateSetFogNearDistance((value)); \
	else if ((param) == GL_FOG_END) RenderManager.StateSetFogFarDistance((value)); \
	else if ((param) == GL_FOG_DENSITY) RenderManager.StateSetFogDensity((value)); \
} while (0)
#define glCullFace(dir) RenderManager.StateSetFaceCullCW((dir) == GL_BACK)
#define glMultiTexCoord2f(unit, u, v) RenderManager.StateSetVertexTextureUV((u) / 256.0f, (v) / 256.0f)

#define glDisable(state) do { \
	if ((state) == GL_TEXTURE_2D) RenderManager.TextureBind(-1); \
	else if ((state) == GL_BLEND) RenderManager.StateSetBlendEnable(false); \
	else if ((state) == GL_CULL_FACE) RenderManager.StateSetFaceCull(false); \
	else if ((state) == GL_DEPTH_TEST) RenderManager.StateSetDepthTestEnable(false); \
	else if ((state) == GL_ALPHA_TEST) RenderManager.StateSetAlphaTestEnable(false); \
	else if ((state) == GL_FOG) RenderManager.StateSetFogEnable(false); \
	else if ((state) == GL_LIGHTING) RenderManager.StateSetLightingEnable(false); \
	else if ((state) == GL_LIGHT0) RenderManager.StateSetLightEnable(0, false); \
	else if ((state) == GL_LIGHT1) RenderManager.StateSetLightEnable(1, false); \
} while (0)

#define glEnable(state) do { \
	if ((state) == GL_BLEND) RenderManager.StateSetBlendEnable(true); \
	else if ((state) == GL_CULL_FACE) RenderManager.StateSetFaceCull(true); \
	else if ((state) == GL_DEPTH_TEST) RenderManager.StateSetDepthTestEnable(true); \
	else if ((state) == GL_ALPHA_TEST) RenderManager.StateSetAlphaTestEnable(true); \
	else if ((state) == GL_FOG) RenderManager.StateSetFogEnable(true); \
	else if ((state) == GL_LIGHTING) RenderManager.StateSetLightingEnable(true); \
	else if ((state) == GL_LIGHT0) RenderManager.StateSetLightEnable(0, true); \
	else if ((state) == GL_LIGHT1) RenderManager.StateSetLightEnable(1, true); \
} while (0)

#endif // _LINUX64

#ifndef _LINUX64

class GL11
{
public:
	static const int GL_SMOOTH = 0;
	static const int GL_FLAT = 0;
	static void glShadeModel(int) {};
};

class ARBVertexBufferObject
{
public:
	static const int GL_ARRAY_BUFFER_ARB = 0;
	static const int GL_STREAM_DRAW_ARB = 0;
	static void glBindBufferARB(int, int) {}
	static void glBufferDataARB(int, ByteBuffer *, int) {}
	static void glGenBuffersARB(IntBuffer *) {}
};

#else // _LINUX64

// On Linux, GL11 and ARBVertexBufferObject use real GL constants from GLEW
class GL11
{
public:
	static void glShadeModel(int) {};
};

class ARBVertexBufferObject
{
public:
	static void glBindBufferARB(int, int) {}
	static void glBufferDataARB(int, ByteBuffer *, int) {}
	static void glGenBuffersARB(IntBuffer *) {}
};

#endif // _LINUX64

class Level;
class Player;
class Textures;
class Font;
class MapItemSavedData;
class Mob;
class Particles
{
public:
	void render(float) {}
	void tick() {}
};

class BufferedImage;

class Graphics
{
public:
	void drawImage(BufferedImage *, int, int, void *) {}
	void dispose() {}
};

class ZipEntry {};
class InputStream;
class File;
class ZipFile
{
public:
	ZipFile(File *file) {}
	InputStream *getInputStream(ZipEntry *entry) { return nullptr; }
	ZipEntry *getEntry(const wstring& name) {return nullptr;}
	void close() {}
};

class ImageIO
{
public:
	static BufferedImage *read(InputStream *in) { return nullptr; }
};

class Keyboard
{
public:
	static void create() {}
	static void destroy() {}
#ifdef _WINDOWS64
	static bool isKeyDown(int keyCode);
#else
	static bool isKeyDown(int) { return false; }
#endif
	static wstring getKeyName(int) { return L"KEYNAME"; }
	static void enableRepeatEvents(bool) {}

	static const int KEY_A = 0;  static const int KEY_B = 1;  static const int KEY_C = 2;
	static const int KEY_D = 3;  static const int KEY_E = 4;  static const int KEY_F = 5;
	static const int KEY_G = 6;  static const int KEY_H = 7;  static const int KEY_I = 8;
	static const int KEY_J = 9;  static const int KEY_K = 10; static const int KEY_L = 11;
	static const int KEY_M = 12; static const int KEY_N = 13; static const int KEY_O = 14;
	static const int KEY_P = 15; static const int KEY_Q = 16; static const int KEY_R = 17;
	static const int KEY_S = 18; static const int KEY_T = 19; static const int KEY_U = 20;
	static const int KEY_V = 21; static const int KEY_W = 22; static const int KEY_X = 23;
	static const int KEY_Y = 24; static const int KEY_Z = 25;
	static const int KEY_SPACE = 26;  static const int KEY_LSHIFT = 27;
	static const int KEY_ESCAPE = 28; static const int KEY_BACK = 29;
	static const int KEY_RETURN = 30; static const int KEY_RSHIFT = 31;
	static const int KEY_UP = 32;    static const int KEY_DOWN = 33;
	static const int KEY_TAB = 34;
	static const int KEY_1 = 35; static const int KEY_2 = 36; static const int KEY_3 = 37;
	static const int KEY_4 = 38; static const int KEY_5 = 39; static const int KEY_6 = 40;
	static const int KEY_7 = 41; static const int KEY_8 = 42; static const int KEY_9 = 43;
	static const int KEY_F1 = 44;  static const int KEY_F3 = 45; static const int KEY_F4 = 46;
	static const int KEY_F5 = 47;  static const int KEY_F6 = 48; static const int KEY_F8 = 49;
	static const int KEY_F9 = 50;  static const int KEY_F11 = 51;
	static const int KEY_ADD = 52; static const int KEY_SUBTRACT = 53;
	static const int KEY_LEFT = 54; static const int KEY_RIGHT = 55;

#ifdef _WINDOWS64
	static int toVK(int keyConst);
#endif
};

class Mouse
{
public:
	static void create() {}
	static void destroy() {}
#ifdef _WINDOWS64
	static int getX();
	static int getY();
	static bool isButtonDown(int button);
#else
	static int getX() { return 0; }
	static int getY() { return 0; }
	static bool isButtonDown(int) { return false; }
#endif
};

class Display
{
public:
	static bool isActive() {return true;}
	static void update();
	static void swapBuffers();
	static void destroy() {}
};

class BackgroundDownloader
{
public:
	BackgroundDownloader(File workDir, Minecraft* minecraft) {}
	void start() {}
	void halt() {}
	void forceReload() {}
};

class Color
{
public:
	static int HSBtoRGB(float,float,float) {return 0;}
};
