#pragma once

// ============================================================
// Linux64_Compat.h
// Win32 type/macro compatibility shims for the Linux64 port.
// Included via stdafx.h when _LINUX64 is defined.
// ============================================================

// Prevent std::byte from polluting the global namespace — done via -D__cpp_lib_byte=0
// in CMakeLists.txt. No action needed here.

#include <stdint.h>
#include <stddef.h>

// The codebase uses 'byte' everywhere as 'unsigned char'.
// In C++17, 'using namespace std' brings in std::byte (an enum class),
// making bare 'byte' ambiguous. We #define byte to uint8_t so all
// existing code works without modification, and std::byte stays
// qualified as std::byte — no collision.
#define byte uint8_t
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// --------------- Basic Win32 integer typedefs ---------------
typedef unsigned char       BYTE;
typedef unsigned char       UCHAR;
typedef unsigned short      WORD;
typedef unsigned int        DWORD;
typedef unsigned int        UINT;
typedef int                 INT;
typedef long                LONG;
typedef short               SHORT;
typedef char                CHAR;
typedef void                VOID;
#define CONST const
typedef unsigned long       ULONG;
typedef long long           LONGLONG;
typedef unsigned long long  ULONGLONG;
typedef float               FLOAT;
typedef int                 BOOL;
typedef void*               LPVOID;
typedef void*               PVOID;
typedef const wchar_t*      LPCWSTR;
typedef const char*         LPCSTR;

// Pointer-sized types
typedef uintptr_t           ULONG_PTR;
typedef intptr_t            LONG_PTR;

// FILETIME (Windows timestamp struct — stubbed as a 64-bit value)
typedef struct { DWORD dwLowDateTime; DWORD dwHighDateTime; } FILETIME;

// Windows high-resolution timer type (used by PerformanceTimer)
typedef union { struct { DWORD LowPart; LONG HighPart; }; int64_t QuadPart; } LARGE_INTEGER;

// ZeroMemory — Windows macro equivalent
#define ZeroMemory(dest, size) memset(dest, 0, size)

// __debugbreak — MSVC intrinsic, maps to GCC trap
#ifndef __debugbreak
#define __debugbreak() __builtin_trap()
#endif

typedef const void*         LPCVOID;
typedef char*               LPSTR;
typedef const char*         LPCSTR;
typedef wchar_t*            LPWSTR;
typedef const wchar_t*      LPCWSTR;
typedef wchar_t             WCHAR;
typedef unsigned char*      PBYTE;
typedef DWORD*              LPDWORD;
typedef size_t              SIZE_T;
typedef uint16_t            USHORT;
typedef int64_t             INT64;
typedef uint64_t            UINT64;
typedef uint32_t            UINT32;
typedef int32_t             INT32;

// HRESULT / success/failure
typedef long                HRESULT;
#define S_OK                ((HRESULT)0x00000000L)
#define S_FALSE             ((HRESULT)0x00000001L)
#define E_FAIL              ((HRESULT)0x80004005L)
#define E_INVALIDARG        ((HRESULT)0x80070057L)
#define E_OUTOFMEMORY       ((HRESULT)0x8007000EL)
#define SUCCEEDED(hr)       (((HRESULT)(hr)) >= 0)
#define FAILED(hr)          (((HRESULT)(hr)) < 0)
#define HRESULT_SUCCEEDED   SUCCEEDED

// Handle types — opaque pointers on Linux
typedef void*               HANDLE;
typedef void*               HMODULE;
typedef void*               HINSTANCE;
typedef void*               HWND;
typedef void*               HICON;
typedef void*               HCURSOR;
typedef void*               HBRUSH;
typedef void*               HDC;
typedef void*               HMENU;
typedef void*               HGLRC;
typedef HANDLE              HTHREAD;
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

// BOOL values
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Calling conventions — no-op on Linux/x86-64
#define WINAPI
#define CALLBACK
#define APIENTRY
#define STDMETHODCALLTYPE
#define __forceinline       inline __attribute__((always_inline))

// MAX_PATH
#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(x) (void)(x)
#endif
#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

// --------------- Win32 thread / sync stubs ---------------
typedef struct {
    pthread_mutex_t mutex;
} CRITICAL_SECTION;

inline void InitializeCriticalSection(CRITICAL_SECTION* cs) { pthread_mutex_init(&cs->mutex, nullptr); }
inline void InitializeCriticalSectionAndSpinCount(CRITICAL_SECTION* cs, DWORD) { pthread_mutex_init(&cs->mutex, nullptr); }
inline void DeleteCriticalSection(CRITICAL_SECTION* cs)     { pthread_mutex_destroy(&cs->mutex); }
inline void EnterCriticalSection(CRITICAL_SECTION* cs)      { pthread_mutex_lock(&cs->mutex); }
inline void LeaveCriticalSection(CRITICAL_SECTION* cs)      { pthread_mutex_unlock(&cs->mutex); }

inline DWORD TlsAlloc() { pthread_key_t k; pthread_key_create(&k, nullptr); return (DWORD)k; }
inline void  TlsFree(DWORD idx)               { pthread_key_delete((pthread_key_t)idx); }
inline LPVOID TlsGetValue(DWORD idx)          { return pthread_getspecific((pthread_key_t)idx); }
inline BOOL  TlsSetValue(DWORD idx, LPVOID v) { return pthread_setspecific((pthread_key_t)idx, v) == 0; }

inline void Sleep(DWORD ms) { usleep(ms * 1000); }

// --------------- Win32 string helpers ---------------
#define _snprintf_s(buf, bufsz, count, fmt, ...) snprintf(buf, bufsz, fmt, ##__VA_ARGS__)
#define _TRUNCATE ((size_t)-1)
#define strncpy_s(dst, dstsz, src, cnt) strncpy(dst, src, (cnt) == _TRUNCATE ? (dstsz)-1 : (cnt))
#define strcpy_s(dst, dstsz, src)       strncpy(dst, src, (dstsz)-1)
#define sprintf_s(buf, sz, fmt, ...)    snprintf(buf, sz, fmt, ##__VA_ARGS__)
#define sscanf_s                         sscanf
#define fopen_s(fp, path, mode)         (*(fp) = fopen(path, mode), *(fp) ? 0 : errno)
#define wcscpy_s(dst, dstsz, src)       wcsncpy(dst, src, (dstsz)-1)
#define wcsncpy_s(dst, dstsz, src, cnt) wcsncpy(dst, src, (cnt) == _TRUNCATE ? (dstsz)-1 : (cnt))
#define swprintf_s(buf, sz, fmt, ...)   swprintf(buf, sz, fmt, ##__VA_ARGS__)
#define _wcsicmp                         wcscasecmp

// WideChar conversion stubs (ASCII-safe for Phase 1)
inline int WideCharToMultiByte(UINT cp, DWORD flags, LPCWSTR src, int srcLen,
                                LPSTR dst, int dstLen, LPCSTR def, BOOL* used) {
    (void)cp; (void)flags; (void)def; (void)used;
    if (!dst || dstLen == 0) return 0;
    int i = 0;
    while (src[i] && (srcLen < 0 || i < srcLen) && i < dstLen - 1)
        { dst[i] = (char)(src[i] & 0x7F); i++; }
    dst[i] = '\0'; return i;
}
inline int MultiByteToWideChar(UINT cp, DWORD flags, LPCSTR src, int srcLen,
                                LPWSTR dst, int dstLen) {
    (void)cp; (void)flags;
    if (!dst || dstLen == 0) return 0;
    int i = 0;
    while (src[i] && (srcLen < 0 || i < srcLen) && i < dstLen - 1)
        { dst[i] = (wchar_t)(unsigned char)src[i]; i++; }
    dst[i] = L'\0'; return i;
}

// --------------- XUSER / controller stubs ---------------
#define XUSER_MAX_COUNT 4
#define XUSER_INDEX_ANY 0xFF

// --------------- PIX profiling — no-ops ---------------
#define PIXBeginNamedEvent(col, name, ...) ((void)0)
#define PIXEndNamedEvent()                 ((void)0)
#define PIXAddNamedCounter(val, name, ...) ((void)0)

// --------------- OS helpers ---------------
inline DWORD GetModuleFileNameA(HINSTANCE, LPSTR buf, DWORD size) {
    ssize_t len = readlink("/proc/self/exe", buf, size - 1);
    if (len < 0) { buf[0] = '\0'; return 0; }
    buf[len] = '\0'; return (DWORD)len;
}
inline BOOL SetCurrentDirectoryA(LPCSTR path) { return chdir(path) == 0; }

#include <sys/stat.h>
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
inline DWORD GetFileAttributesA(LPCSTR path) {
    struct stat st;
    return stat(path, &st) == 0 ? 0 : INVALID_FILE_ATTRIBUTES;
}

#define SetProcessDPIAware()      ((void)0)
#define GetProcAddress(mod, name) ((void*)nullptr)
#define GetModuleHandleA(name)    ((HMODULE)nullptr)

// --------------- D3D forward decls (headers reference these) ---------------
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
typedef struct { long left, top, right, bottom; } D3D11_RECT;
typedef D3D11_RECT RECT;

// D3D11 blend enums used as GL constant equivalents in 4J_Render.h
enum D3D11_BLEND {
    D3D11_BLEND_ZERO             = 1,
    D3D11_BLEND_ONE              = 2,
    D3D11_BLEND_SRC_COLOR        = 3,
    D3D11_BLEND_INV_SRC_COLOR    = 4,
    D3D11_BLEND_SRC_ALPHA        = 5,
    D3D11_BLEND_INV_SRC_ALPHA    = 6,
    D3D11_BLEND_DEST_ALPHA       = 7,
    D3D11_BLEND_INV_DEST_ALPHA   = 8,
    D3D11_BLEND_DEST_COLOR       = 9,
    D3D11_BLEND_INV_DEST_COLOR   = 10,
    D3D11_BLEND_BLEND_FACTOR     = 14,
    D3D11_BLEND_INV_BLEND_FACTOR = 15,
};
enum D3D11_COMPARISON_FUNC {
    D3D11_COMPARISON_NEVER         = 1,
    D3D11_COMPARISON_LESS          = 2,
    D3D11_COMPARISON_EQUAL         = 3,
    D3D11_COMPARISON_LESS_EQUAL    = 4,
    D3D11_COMPARISON_GREATER       = 5,
    D3D11_COMPARISON_NOT_EQUAL     = 6,
    D3D11_COMPARISON_GREATER_EQUAL = 7,
    D3D11_COMPARISON_ALWAYS        = 8,
};

// --------------- Iggy no-ops ---------------
inline void IggyFlushInstalledFonts() {}

#ifndef _wtoi
inline int _wtoi(const wchar_t* s) { return (int)wcstol(s, nullptr, 10); }
#endif
