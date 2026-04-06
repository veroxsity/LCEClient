// Linux64_Minecraft.cpp — SDL2 + OpenGL entry point for the Linux64 port
#ifdef _LINUX64
#include "stdafx.h"
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <locale.h>

#include "Linux64/KeyboardMouseInput.h"
#include "Linux64/Network/WinsockNetLayer.h"
#include "Minecraft.h"
#include "ClientConnection.h"
#include "ChatScreen.h"
#include "User.h"
#include "Tesselator.h"
#include "Options.h"
#include "Gui.h"
#include "Textures.h"
#include "Settings.h"
#include "GameRenderer.h"
#include "ConnectScreen.h"
#include "Minecraft.World/compression.h"
#include "Minecraft.World/OldChunkStorage.h"
#include "Minecraft.World/AABB.h"
#include "Minecraft.World/Vec3.h"
#include "Minecraft.World/Level.h"
#include "Minecraft.World/IntCache.h"
#include "Minecraft.World/Socket.h"
#include "Minecraft.World/ThreadName.h"

// Globals
SDL_Window*   g_pWindow  = nullptr;
SDL_GLContext g_GLContext = nullptr;

int   g_iScreenWidth  = 1920, g_iScreenHeight = 1080;
int   g_rScreenWidth  = 1920, g_rScreenHeight = 1080;
bool  g_bWidescreen   = true;
float g_iAspectRatio  = 16.f/9.f;
static bool g_isFullscreen = false;
static bool g_bResizeReady = false;

char    g_Win64Username[17]  = {};
wchar_t g_Win64UsernameW[17] = {};

// Profile data arrays expected by InitialiseMinecraftRuntime
#define NUM_PROFILE_VALUES   5
#define NUM_PROFILE_SETTINGS 4
static DWORD dwProfileSettingsA[NUM_PROFILE_VALUES] = {0,0,0,0,0};

// Helpers
static void GetExeDir(char* out, size_t outSize) {
    ssize_t len = readlink("/proc/self/exe", out, outSize-1);
    if (len<0) { out[0]='\0'; return; }
    out[len]='\0';
    char* slash = strrchr(out, '/');
    if (slash) *(slash+1)='\0';
}

static void LoadUsername() {
    char exeDir[PATH_MAX]={}, path[PATH_MAX];
    GetExeDir(exeDir,sizeof(exeDir));
    snprintf(path,sizeof(path),"%susername.txt",exeDir);
    FILE* f=fopen(path,"r");
    if (f) {
        char buf[128]={};
        if (fgets(buf,sizeof(buf),f)) {
            int len=(int)strlen(buf);
            while (len>0&&(buf[len-1]=='\n'||buf[len-1]=='\r'||buf[len-1]==' ')) buf[--len]='\0';
            if (len>0) strncpy(g_Win64Username,buf,sizeof(g_Win64Username)-1);
        }
        fclose(f);
    }
    if (!g_Win64Username[0]) strncpy(g_Win64Username,"Player",sizeof(g_Win64Username)-1);
    MultiByteToWideChar(0,0,g_Win64Username,-1,g_Win64UsernameW,17);
}

static bool LoadFullscreenOption() {
    char exeDir[PATH_MAX]={}, path[PATH_MAX];
    GetExeDir(exeDir,sizeof(exeDir));
    snprintf(path,sizeof(path),"%soptions.txt",exeDir);
    FILE* f=fopen(path,"r"); if (!f) return false;
    char line[256]; bool result=false;
    while (fgets(line,sizeof(line),f)) { int v=0; if (sscanf(line,"fullscreen=%d",&v)==1) { result=v!=0; break; } }
    fclose(f); return result;
}
static void SaveFullscreenOption(bool fs) {
    char exeDir[PATH_MAX]={}, path[PATH_MAX];
    GetExeDir(exeDir,sizeof(exeDir)); snprintf(path,sizeof(path),"%soptions.txt",exeDir);
    FILE* f=fopen(path,"w"); if (f) { fprintf(f,"fullscreen=%d\n",fs?1:0); fclose(f); }
}

// Window / GL init
static bool InitWindow() {
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER)!=0) {
        fprintf(stderr,"[Linux64] SDL_Init: %s\n",SDL_GetError()); return false;
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0,&dm)==0) { g_rScreenWidth=dm.w; g_rScreenHeight=dm.h; }
    g_pWindow=SDL_CreateWindow("Minecraft Legacy Console Edition",
        SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,g_rScreenWidth,g_rScreenHeight,
        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_SHOWN);
    if (!g_pWindow) { fprintf(stderr,"[Linux64] SDL_CreateWindow: %s\n",SDL_GetError()); return false; }
    g_GLContext=SDL_GL_CreateContext(g_pWindow);
    if (!g_GLContext) { fprintf(stderr,"[Linux64] GL context: %s\n",SDL_GetError()); return false; }
    SDL_GL_MakeCurrent(g_pWindow,g_GLContext);
    SDL_GL_SetSwapInterval(0);
    glewExperimental=GL_TRUE;
    GLenum ge=glewInit();
    if (ge!=GLEW_OK) { fprintf(stderr,"[Linux64] GLEW: %s\n",glewGetErrorString(ge)); return false; }
    printf("[Linux64] OpenGL %s | %s\n",glGetString(GL_VERSION),glGetString(GL_RENDERER));
    return true;
}

static void ToggleFullscreen() {
    g_isFullscreen=!g_isFullscreen;
    SDL_SetWindowFullscreen(g_pWindow,g_isFullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0);
    SaveFullscreenOption(g_isFullscreen);
}
static void HandleResize(int w,int h) {
    if (w<=0||h<=0||!g_bResizeReady) return;
    g_rScreenWidth=w; g_rScreenHeight=h;
    glViewport(0,0,w,h);
}
static int SDLButtonToMouseBtn(Uint8 btn) {
    switch(btn) {
        case SDL_BUTTON_LEFT:   return KeyboardMouseInput::MOUSE_LEFT;
        case SDL_BUTTON_RIGHT:  return KeyboardMouseInput::MOUSE_RIGHT;
        case SDL_BUTTON_MIDDLE: return KeyboardMouseInput::MOUSE_MIDDLE;
        default: return -1;
    }
}

// Forward declarations needed by the action setup
void DefineActions();

static Minecraft* InitialiseMinecraftRuntime() {
    app.loadMediaArchive();
    app.loadStringTable();

    InputManager.Initialise(1,3,MINECRAFT_ACTION_MAX,ACTION_MAX_MENU);
    g_KBMInput.Init();
    DefineActions();
    InputManager.SetJoypadMapVal(0,0);
    InputManager.SetKeyRepeatRate(0.3f,0.2f);

    ProfileManager.Initialise(TITLEID_MINECRAFT,
        app.m_dwOfferID, PROFILE_VERSION_10,
        NUM_PROFILE_VALUES, NUM_PROFILE_SETTINGS,
        dwProfileSettingsA,
        app.GAME_DEFINED_PROFILE_DATA_BYTES * XUSER_MAX_COUNT,
        &app.uiGameDefinedDataChangedBitmask);
    ProfileManager.SetDefaultOptionsCallback(&CConsoleMinecraftApp::DefaultOptionsCallback,(LPVOID)&app);

    g_NetworkManager.Initialise();

    for (int i=0;i<MINECRAFT_NET_MAX_PLAYERS;i++) {
        IQNet::m_player[i].m_smallId=(uint8_t)i;
        IQNet::m_player[i].m_isRemote=false;
        IQNet::m_player[i].m_isHostPlayer=(i==0);
        swprintf(IQNet::m_player[i].m_gamertag,32,L"Player%d",i);
    }
    wcsncpy(IQNet::m_player[0].m_gamertag,g_Win64UsernameW,32);

    WinsockNetLayer::Initialize();
    ProfileManager.SetDebugFullOverride(true);

    Tesselator::CreateNewThreadStorage(1024*1024);
    AABB::CreateNewThreadStorage();
    Vec3::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Compression::CreateNewThreadStorage();
    OldChunkStorage::CreateNewThreadStorage();
    Level::enableLightingCache();
    Tile::CreateNewThreadStorage();

    Minecraft::main();
    Minecraft* pMinecraft=Minecraft::GetInstance();
    if (!pMinecraft) return nullptr;

    app.InitGameSettings();
    app.InitialiseTips();
    return pMinecraft;
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL,"");
    { char exeDir[PATH_MAX]={}; GetExeDir(exeDir,sizeof(exeDir)); if (exeDir[0]) chdir(exeDir); }

    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i],"-name")==0&&i+1<argc) strncpy(g_Win64Username,argv[++i],sizeof(g_Win64Username)-1);
        else if (strcmp(argv[i],"-ip")==0&&i+1<argc) { strncpy(g_Win64MultiplayerIP,argv[++i],sizeof(g_Win64MultiplayerIP)-1); g_Win64MultiplayerJoin=true; }
        else if (strcmp(argv[i],"-port")==0&&i+1<argc) g_Win64MultiplayerPort=atoi(argv[++i]);
        else if (strcmp(argv[i],"-fullscreen")==0) g_isFullscreen=true;
    }
    LoadUsername();
    if (!InitWindow()) return 1;
    if (LoadFullscreenOption()&&!g_isFullscreen) ToggleFullscreen();

    Minecraft* pMinecraft=InitialiseMinecraftRuntime();
    if (!pMinecraft) { SDL_GL_DeleteContext(g_GLContext); SDL_DestroyWindow(g_pWindow); SDL_Quit(); return 1; }
    g_bResizeReady=true;

    bool running=true;
    static bool f3ComboUsed=false;

    while (running&&!app.m_bShutdown) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT: running=false; break;
            case SDL_WINDOWEVENT:
                if (event.window.event==SDL_WINDOWEVENT_RESIZED) HandleResize(event.window.data1,event.window.data2);
                else if (event.window.event==SDL_WINDOWEVENT_FOCUS_LOST) { g_KBMInput.ClearAllState(); g_KBMInput.SetWindowFocused(false); if(g_KBMInput.IsMouseGrabbed()) g_KBMInput.SetMouseGrabbed(false); }
                else if (event.window.event==SDL_WINDOWEVENT_FOCUS_GAINED) g_KBMInput.SetWindowFocused(true);
                break;
            case SDL_KEYDOWN:
                if (!event.key.repeat) g_KBMInput.OnKeyDown(event.key.keysym.scancode);
                if (event.key.keysym.scancode==SDL_SCANCODE_V&&(SDL_GetModState()&KMOD_CTRL)) {
                    Minecraft* pm=Minecraft::GetInstance();
                    ChatScreen* chat=pm&&pm->screen?dynamic_cast<ChatScreen*>(pm->screen):nullptr;
                    if (chat) chat->handlePasteRequest();
                }
                break;
            case SDL_KEYUP: g_KBMInput.OnKeyUp(event.key.keysym.scancode); break;
            case SDL_TEXTINPUT: { wchar_t wc=0; mbtowc(&wc,event.text.text,MB_CUR_MAX); if(wc>=0x20||wc==0x08||wc==0x0D) g_KBMInput.OnChar(wc); } break;
            case SDL_MOUSEBUTTONDOWN: { int b=SDLButtonToMouseBtn(event.button.button); if(b>=0) g_KBMInput.OnMouseButtonDown(b); } break;
            case SDL_MOUSEBUTTONUP:   { int b=SDLButtonToMouseBtn(event.button.button); if(b>=0) g_KBMInput.OnMouseButtonUp(b); } break;
            case SDL_MOUSEMOTION: g_KBMInput.OnMouseMove(event.motion.x,event.motion.y); g_KBMInput.OnRawMouseDelta(event.motion.xrel,event.motion.yrel); break;
            case SDL_MOUSEWHEEL: g_KBMInput.OnMouseWheel(event.wheel.y*120); break;
            default: break;
            }
        }

        if (SDL_GetWindowFlags(g_pWindow)&SDL_WINDOW_MINIMIZED) { SDL_Delay(100); continue; }

        g_KBMInput.Tick();
        app.UpdateTime();
        InputManager.Tick();

        const bool ctrlUsed=InputManager.IsPadConnected(0)&&(InputManager.ButtonPressed(0)||InputManager.GetJoypadStick_LX(0,false)!=0.f||InputManager.GetJoypadStick_LY(0,false)!=0.f);
        if (ctrlUsed) g_KBMInput.SetKBMActive(false);
        else if (g_KBMInput.HasAnyInput()) g_KBMInput.SetKBMActive(true);

        StorageManager.Tick();
        g_NetworkManager.DoWork();

        if (app.GetGameStarted()) {
            pMinecraft->applyFrameMouseLook();
            pMinecraft->run_middle();
            app.SetAppPaused(g_NetworkManager.IsLocalGame()&&g_NetworkManager.GetPlayerCount()==1&&ui.IsPauseMenuDisplayed(ProfileManager.GetPrimaryPad()));
        } else {
            pMinecraft->soundEngine->tick(nullptr,0.0f);
            pMinecraft->textures->tick(true,false);
            IntCache::Reset();
            if (app.GetReallyChangingSessionType()) pMinecraft->tickAllConnections();
        }
        pMinecraft->soundEngine->playMusicTick();

        // Phase 3: ui.tick(); ui.render(); RenderManager.Present();
        SDL_GL_SwapWindow(g_pWindow); // placeholder

        // Mouse grab
        const bool shouldCapture=app.GetGameStarted()&&!ui.GetMenuDisplayed(0)&&pMinecraft->screen==nullptr;
        if (!shouldCapture) { if(g_KBMInput.IsMouseGrabbed()) g_KBMInput.SetMouseGrabbed(false); }
        else if (!g_KBMInput.IsMouseGrabbed()&&g_KBMInput.IsWindowFocused()) g_KBMInput.SetMouseGrabbed(true);

        // Shortcuts
        if (g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_FULLSCREEN)) ToggleFullscreen();
        if (g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_TOGGLE_HUD)) {
            int pad=ProfileManager.GetPrimaryPad();
            uint8_t hud=app.GetGameSettings(pad,eGameSetting_DisplayHUD);
            app.SetGameSettings(pad,eGameSetting_DisplayHUD,hud?0:1);
            app.SetGameSettings(pad,eGameSetting_DisplayHand,hud?0:1);
        }
        if (g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_HOST_SETTINGS)&&!ui.GetMenuDisplayed(0))
            if (Minecraft::GetInstance()) ui.NavigateToScene(0,eUIScene_InGameInfoMenu);
        if (g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_CHAT)&&app.GetGameStarted()&&!ui.GetMenuDisplayed(0)&&pMinecraft->screen==nullptr)
            { g_KBMInput.ClearCharBuffer(); pMinecraft->setScreen(new ChatScreen()); }
        if (g_KBMInput.IsKeyPressed(KeyboardMouseInput::KEY_DEBUG_INFO)) f3ComboUsed=false;
        if (g_KBMInput.IsKeyReleased(KeyboardMouseInput::KEY_DEBUG_INFO)&&!f3ComboUsed)
            if (pMinecraft->options) pMinecraft->options->renderDebug=!pMinecraft->options->renderDebug;

        app.HandleXuiActions();
        Vec3::resetPool();
    }

    WinsockNetLayer::Shutdown();
    SDL_GL_DeleteContext(g_GLContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();
    return 0;
}

#endif // _LINUX64
