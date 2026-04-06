#pragma once
#ifdef _LINUX64
#include "../Common/Consoles_App.h"

class CConsoleMinecraftApp : public CMinecraftApp {
    struct ImageFileBuffer {
        void* m_pBuffer=nullptr; int m_bufferSize=0;
        bool  Allocated() const { return m_pBuffer!=nullptr; }
        void* GetBufferPointer() { return m_pBuffer; }
        int   GetBufferSize() const { return m_bufferSize; }
        void  Release() { free(m_pBuffer); m_pBuffer=nullptr; m_bufferSize=0; }
    } m_ThumbnailBuffer;
public:
    bool m_bShutdown=false;
    CConsoleMinecraftApp();
    virtual void SetRichPresenceContext(int iPad, int contextId);
    virtual void StoreLaunchData();
    virtual void ExitGame();
    virtual void FatalLoadError();
    virtual void CaptureSaveThumbnail();
    virtual void GetSaveThumbnail(PBYTE* pbData, DWORD* pdwSize);
    virtual void ReleaseSaveThumbnail();
    virtual void GetScreenshot(int iPad, PBYTE* pbData, DWORD* pdwSize);
    virtual int  LoadLocalTMSFile(WCHAR* wchTMSFile);
    virtual int  LoadLocalTMSFile(WCHAR* wchTMSFile, int eExt);
    virtual void FreeLocalTMSFiles(int eType);
    virtual int  GetLocalTMSFileIndex(WCHAR* wchTMSFile, bool bFilenameIncludesExtension, int eEXT=0);
    virtual void ReadBannedList(int iPad, int action=0, bool bCallback=false) {}
    static void DefaultOptionsCallback(LPVOID param);
    virtual void TemporaryCreateGameStart();
};

extern CConsoleMinecraftApp app;
#endif // _LINUX64
