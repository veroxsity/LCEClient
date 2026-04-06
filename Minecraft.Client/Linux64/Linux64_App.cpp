#ifdef _LINUX64
#include "stdafx.h"
#include "Linux64/Linux64_App.h"
#include "Minecraft.h"
#include "User.h"

CConsoleMinecraftApp app;

CConsoleMinecraftApp::CConsoleMinecraftApp() : CMinecraftApp() { m_bShutdown=false; }
void CConsoleMinecraftApp::SetRichPresenceContext(int iPad,int contextId) { ProfileManager.SetRichPresenceContextValue(iPad,CONTEXT_GAME_STATE,contextId); }
void CConsoleMinecraftApp::StoreLaunchData() {}
void CConsoleMinecraftApp::ExitGame() { m_bShutdown=true; }
void CConsoleMinecraftApp::FatalLoadError() {}
void CConsoleMinecraftApp::CaptureSaveThumbnail() {}
void CConsoleMinecraftApp::GetSaveThumbnail(PBYTE* pbData, DWORD* pdwSize) { if(pbData)*pbData=nullptr; if(pdwSize)*pdwSize=0; }
void CConsoleMinecraftApp::ReleaseSaveThumbnail() {}
void CConsoleMinecraftApp::GetScreenshot(int, PBYTE* pbData, DWORD* pdwSize) { if(pbData)*pbData=nullptr; if(pdwSize)*pdwSize=0; }
int  CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR*,int) { return 0; }
int  CConsoleMinecraftApp::LoadLocalTMSFile(WCHAR*)     { return 0; }
void CConsoleMinecraftApp::FreeLocalTMSFiles(int)       {}
int  CConsoleMinecraftApp::GetLocalTMSFileIndex(WCHAR*,bool,int) { return -1; }
void CConsoleMinecraftApp::DefaultOptionsCallback(LPVOID) {}

void CConsoleMinecraftApp::TemporaryCreateGameStart() {
    app.setLevelGenerationOptions(nullptr);
    Minecraft* pMinecraft=Minecraft::GetInstance();
    app.ReleaseSaveThumbnail();
    ProfileManager.SetLockedProfile(0);
    extern wchar_t g_Win64UsernameW[17];
    pMinecraft->user->name=g_Win64UsernameW;
    app.ApplyGameSettingsChanged(0);
}
#endif // _LINUX64
