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
#include <string>

// The codebase uses 'byte' everywhere as 'unsigned char'.
// In C++17, 'using namespace std' brings in std::byte (an enum class),
// making bare 'byte' ambiguous. We #define byte to uint8_t so all
// existing code works without modification, and std::byte stays
// qualified as std::byte — no collision.
#define byte uint8_t
#include <stddef.h>
#include <string.h>
#include <wchar.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
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
typedef BOOL*               PBOOL;
typedef const wchar_t*      LPCWSTR;
typedef const char*         LPCSTR;
typedef size_t              DWORD_PTR;
typedef long*               PLONG;
typedef int                 errno_t;
typedef void*               LPSECURITY_ATTRIBUTES;
typedef void*               LPOVERLAPPED;

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
typedef DWORD*              PDWORD;
typedef size_t              SIZE_T;
typedef uint16_t            USHORT;
typedef int64_t             INT64;
typedef uint64_t            UINT64;
typedef uint32_t            UINT32;
typedef int32_t             INT32;
typedef int32_t             __int32;
typedef uint32_t            __uint32;
typedef FILETIME*           PFILETIME;
typedef FILETIME*           LPFILETIME;
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

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
#define __cdecl
#define __forceinline       inline __attribute__((always_inline))

// MAX_PATH
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define CP_ACP 0
#define CP_UTF8 65001

#define FILE_ATTRIBUTE_DIRECTORY 0x10
#define FILE_ATTRIBUTE_NORMAL 0x80

#define GENERIC_READ 0x80000000
#define GENERIC_WRITE 0x40000000

#define FILE_SHARE_READ 0x00000001
#define FILE_SHARE_WRITE 0x00000002

#define CREATE_NEW 1
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define OPEN_ALWAYS 4
#define TRUNCATE_EXISTING 5

#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define FILE_END SEEK_END

#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_FLAG_OVERLAPPED 0x40000000
#define FILE_FLAG_NO_BUFFERING 0x20000000

#define INVALID_FILE_SIZE ((DWORD)-1)

#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_WRITECOMBINE     0x400

#define MEM_COMMIT           0x1000
#define MEM_RESERVE          0x2000
#define MEM_DECOMMIT         0x4000
#define MEM_RELEASE          0x8000
#define MEM_LARGE_PAGES  0x20000000

#define WAIT_OBJECT_0 0
#define WAIT_ABANDONED 0x00000080L
#define WAIT_TIMEOUT 258L
#define WAIT_FAILED 0xFFFFFFFF
#define INFINITE 0xFFFFFFFF
#define STILL_ACTIVE 259

#ifndef MAXLONG
#define MAXLONG 0x7fffffff
#endif

#define THREAD_BASE_PRIORITY_LOWRT  15
#define THREAD_BASE_PRIORITY_MAX    2
#define THREAD_BASE_PRIORITY_MIN   -2
#define THREAD_BASE_PRIORITY_IDLE  -15

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST + 1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST - 1)
#define THREAD_PRIORITY_ERROR_RETURN    MAXLONG
#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE

#define MAXUINT_PTR  (~((uintptr_t)0))
#define MAXINT_PTR   ((intptr_t)(MAXUINT_PTR >> 1))
#define MININT_PTR   (~MAXINT_PTR)
#define MAXULONG_PTR (~((ULONG_PTR)0))
#define MAXLONG_PTR  ((LONG_PTR)(MAXULONG_PTR >> 1))
#define MINLONG_PTR  (~MAXLONG_PTR)

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
    if (!src) return 0;
    int required = 0;
    while (src[required] && (srcLen < 0 || required < srcLen)) required++;
    if (!dst || dstLen == 0) return required + 1;
    int i = 0;
    while (src[i] && (srcLen < 0 || i < srcLen) && i < dstLen - 1)
        { dst[i] = (char)(src[i] & 0x7F); i++; }
    dst[i] = '\0'; return i;
}
inline int MultiByteToWideChar(UINT cp, DWORD flags, LPCSTR src, int srcLen,
                                LPWSTR dst, int dstLen) {
    (void)cp; (void)flags;
    if (!src) return 0;
    int required = 0;
    while (src[required] && (srcLen < 0 || required < srcLen)) required++;
    if (!dst || dstLen == 0) return required + 1;
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
inline DWORD GetCurrentThreadId() {
#ifdef SYS_gettid
    return (DWORD)syscall(SYS_gettid);
#else
    return (DWORD)(uintptr_t)pthread_self();
#endif
}
inline DWORD GetTickCount() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)(ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL);
}
inline ULONGLONG GetTickCount64() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL + ts.tv_nsec / 1000000ULL;
}
inline LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    (void)flProtect;
    if ((flAllocationType & MEM_RESERVE) != 0 && lpAddress == nullptr) {
        void* memory = nullptr;
        if (posix_memalign(&memory, 4096, dwSize) != 0) {
            return nullptr;
        }
        return memory;
    }
    if ((flAllocationType & MEM_COMMIT) != 0) {
        return lpAddress;
    }
    return lpAddress;
}
inline BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    (void)dwSize;
    if (!lpAddress) return FALSE;
    if ((dwFreeType & (MEM_DECOMMIT | MEM_RELEASE)) != 0) {
        free(lpAddress);
    }
    return TRUE;
}
inline VOID GetSystemTime(LPSYSTEMTIME outTime) {
    if (!outTime) return;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t seconds = ts.tv_sec;
    struct tm utcTime;
    gmtime_r(&seconds, &utcTime);
    outTime->wYear = static_cast<WORD>(utcTime.tm_year + 1900);
    outTime->wMonth = static_cast<WORD>(utcTime.tm_mon + 1);
    outTime->wDayOfWeek = static_cast<WORD>(utcTime.tm_wday);
    outTime->wDay = static_cast<WORD>(utcTime.tm_mday);
    outTime->wHour = static_cast<WORD>(utcTime.tm_hour);
    outTime->wMinute = static_cast<WORD>(utcTime.tm_min);
    outTime->wSecond = static_cast<WORD>(utcTime.tm_sec);
    outTime->wMilliseconds = static_cast<WORD>(ts.tv_nsec / 1000000L);
}
inline VOID GetLocalTime(LPSYSTEMTIME outTime) {
    if (!outTime) return;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t seconds = tv.tv_sec;
    struct tm localTime;
    localtime_r(&seconds, &localTime);
    outTime->wYear = static_cast<WORD>(localTime.tm_year + 1900);
    outTime->wMonth = static_cast<WORD>(localTime.tm_mon + 1);
    outTime->wDayOfWeek = static_cast<WORD>(localTime.tm_wday);
    outTime->wDay = static_cast<WORD>(localTime.tm_mday);
    outTime->wHour = static_cast<WORD>(localTime.tm_hour);
    outTime->wMinute = static_cast<WORD>(localTime.tm_min);
    outTime->wSecond = static_cast<WORD>(localTime.tm_sec);
    outTime->wMilliseconds = static_cast<WORD>(tv.tv_usec / 1000);
}
inline BOOL SystemTimeToFileTime(CONST SYSTEMTIME* systemTime, LPFILETIME fileTime) {
    if (!systemTime || !fileTime) return FALSE;
    struct tm utcTime = {};
    utcTime.tm_year = systemTime->wYear - 1900;
    utcTime.tm_mon = systemTime->wMonth - 1;
    utcTime.tm_mday = systemTime->wDay;
    utcTime.tm_hour = systemTime->wHour;
    utcTime.tm_min = systemTime->wMinute;
    utcTime.tm_sec = systemTime->wSecond;
    time_t seconds = timegm(&utcTime);
    if (seconds < 0) return FALSE;
    const uint64_t windowsEpochOffsetSeconds = 11644473600ULL;
    const uint64_t ticks =
        (static_cast<uint64_t>(seconds) + windowsEpochOffsetSeconds) * 10000000ULL +
        static_cast<uint64_t>(systemTime->wMilliseconds) * 10000ULL;
    fileTime->dwLowDateTime = static_cast<DWORD>(ticks & 0xFFFFFFFFULL);
    fileTime->dwHighDateTime = static_cast<DWORD>((ticks >> 32) & 0xFFFFFFFFULL);
    return TRUE;
}
inline BOOL FileTimeToSystemTime(CONST FILETIME* fileTime, LPSYSTEMTIME systemTime) {
    if (!fileTime || !systemTime) return FALSE;
    const uint64_t ticks =
        (static_cast<uint64_t>(fileTime->dwHighDateTime) << 32) |
        static_cast<uint64_t>(fileTime->dwLowDateTime);
    const uint64_t windowsEpochOffsetSeconds = 11644473600ULL;
    const uint64_t totalSeconds = ticks / 10000000ULL;
    if (totalSeconds < windowsEpochOffsetSeconds) return FALSE;
    time_t unixSeconds = static_cast<time_t>(totalSeconds - windowsEpochOffsetSeconds);
    struct tm utcTime;
    gmtime_r(&unixSeconds, &utcTime);
    systemTime->wYear = static_cast<WORD>(utcTime.tm_year + 1900);
    systemTime->wMonth = static_cast<WORD>(utcTime.tm_mon + 1);
    systemTime->wDayOfWeek = static_cast<WORD>(utcTime.tm_wday);
    systemTime->wDay = static_cast<WORD>(utcTime.tm_mday);
    systemTime->wHour = static_cast<WORD>(utcTime.tm_hour);
    systemTime->wMinute = static_cast<WORD>(utcTime.tm_min);
    systemTime->wSecond = static_cast<WORD>(utcTime.tm_sec);
    systemTime->wMilliseconds = static_cast<WORD>((ticks % 10000000ULL) / 10000ULL);
    return TRUE;
}
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* outFrequency) {
    if (!outFrequency) return FALSE;
    outFrequency->QuadPart = 1000000000LL;
    return TRUE;
}
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* outCounter) {
    if (!outCounter) return FALSE;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    outCounter->QuadPart = ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return TRUE;
}
inline void OutputDebugStringA(LPCSTR text) {
    if (text) fputs(text, stderr);
}
inline void OutputDebugStringW(LPCWSTR text) {
    if (!text) return;
    char buffer[1024];
    WideCharToMultiByte(CP_UTF8, 0, text, -1, buffer, (int)sizeof(buffer), nullptr, nullptr);
    fputs(buffer, stderr);
}
#define OutputDebugString OutputDebugStringA

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[MAX_PATH];
    CHAR cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

namespace linux64_detail {

enum class HandleKind {
    File,
    Find
};

struct HandleBase {
    explicit HandleBase(HandleKind kindIn) : kind(kindIn) {}
    HandleKind kind;
    virtual ~HandleBase() = default;
};

struct FileHandle final : HandleBase {
    explicit FileHandle(int fdIn) : HandleBase(HandleKind::File), fd(fdIn) {}
    int fd;
};

struct FindHandle final : HandleBase {
    FindHandle(DIR* dirIn, std::string directoryIn)
        : HandleBase(HandleKind::Find), dir(dirIn), directory(std::move(directoryIn)) {}
    DIR* dir;
    std::string directory;
};

inline std::string NormalisePath(const char* path) {
    std::string normalised = path ? path : "";
    for (char& ch : normalised) {
        if (ch == '\\') ch = '/';
    }
    return normalised;
}

inline std::string NormalisePath(const wchar_t* path) {
    std::string normalised;
    if (!path) return normalised;
    while (*path) {
        char ch = static_cast<char>(*path & 0x7F);
        normalised.push_back(ch == '\\' ? '/' : ch);
        ++path;
    }
    return normalised;
}

inline void FillFindData(const std::string& directory, const char* name, WIN32_FIND_DATAA* outData) {
    ZeroMemory(outData, sizeof(*outData));
    strncpy(outData->cFileName, name, MAX_PATH - 1);

    std::string fullPath = directory;
    if (!fullPath.empty() && fullPath.back() != '/') fullPath.push_back('/');
    fullPath += name;

    struct stat st;
    if (stat(fullPath.c_str(), &st) != 0) return;

    outData->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
    outData->nFileSizeHigh = static_cast<DWORD>((static_cast<uint64_t>(st.st_size) >> 32) & 0xFFFFFFFF);
    outData->nFileSizeLow = static_cast<DWORD>(static_cast<uint64_t>(st.st_size) & 0xFFFFFFFF);
}

inline bool FindNextEntry(FindHandle* handle, WIN32_FIND_DATAA* outData) {
    if (!handle || !outData) return false;

    for (;;) {
        dirent* entry = readdir(handle->dir);
        if (!entry) return false;
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        FillFindData(handle->directory, entry->d_name, outData);
        return true;
    }
}

inline int OpenFlagsFor(DWORD desiredAccess, DWORD creationDisposition) {
    int flags = 0;
    const bool canRead = (desiredAccess & GENERIC_READ) != 0;
    const bool canWrite = (desiredAccess & GENERIC_WRITE) != 0;

    if (canRead && canWrite) flags |= O_RDWR;
    else if (canWrite) flags |= O_WRONLY;
    else flags |= O_RDONLY;

    switch (creationDisposition) {
    case CREATE_NEW: flags |= O_CREAT | O_EXCL; break;
    case CREATE_ALWAYS: flags |= O_CREAT | O_TRUNC; break;
    case OPEN_EXISTING: break;
    case OPEN_ALWAYS: flags |= O_CREAT; break;
    case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    default: break;
    }

    return flags;
}

} // namespace linux64_detail

#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
inline DWORD GetFileAttributesA(LPCSTR path) {
    std::string normalised = linux64_detail::NormalisePath(path);
    struct stat st;
    if (stat(normalised.c_str(), &st) != 0) return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}
inline DWORD GetFileAttributesW(LPCWSTR path) {
    std::string normalised = linux64_detail::NormalisePath(path);
    struct stat st;
    if (stat(normalised.c_str(), &st) != 0) return INVALID_FILE_ATTRIBUTES;
    return S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
}
inline DWORD GetFileAttributes(LPCSTR path) { return GetFileAttributesA(path); }
inline DWORD GetFileAttributes(LPCWSTR path) { return GetFileAttributesW(path); }

inline HANDLE CreateFileA(LPCSTR path,
                          DWORD desiredAccess,
                          DWORD,
                          LPSECURITY_ATTRIBUTES,
                          DWORD creationDisposition,
                          DWORD,
                          HANDLE) {
    std::string normalised = linux64_detail::NormalisePath(path);
    int fd = open(normalised.c_str(),
                  linux64_detail::OpenFlagsFor(desiredAccess, creationDisposition),
                  0666);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    return new linux64_detail::FileHandle(fd);
}
inline HANDLE CreateFileW(LPCWSTR path,
                          DWORD desiredAccess,
                          DWORD shareMode,
                          LPSECURITY_ATTRIBUTES securityAttributes,
                          DWORD creationDisposition,
                          DWORD flagsAndAttributes,
                          HANDLE templateFile) {
    return CreateFileA(linux64_detail::NormalisePath(path).c_str(),
                       desiredAccess,
                       shareMode,
                       securityAttributes,
                       creationDisposition,
                       flagsAndAttributes,
                       templateFile);
}
inline HANDLE CreateFile(LPCSTR path,
                         DWORD desiredAccess,
                         DWORD shareMode,
                         LPSECURITY_ATTRIBUTES securityAttributes,
                         DWORD creationDisposition,
                         DWORD flagsAndAttributes,
                         HANDLE templateFile) {
    return CreateFileA(path, desiredAccess, shareMode, securityAttributes, creationDisposition, flagsAndAttributes, templateFile);
}
inline HANDLE CreateFile(LPCWSTR path,
                         DWORD desiredAccess,
                         DWORD shareMode,
                         LPSECURITY_ATTRIBUTES securityAttributes,
                         DWORD creationDisposition,
                         DWORD flagsAndAttributes,
                         HANDLE templateFile) {
    return CreateFileW(path, desiredAccess, shareMode, securityAttributes, creationDisposition, flagsAndAttributes, templateFile);
}

inline BOOL ReadFile(HANDLE handle, LPVOID buffer, DWORD bytesToRead, LPDWORD bytesRead, LPOVERLAPPED) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    if (base->kind != linux64_detail::HandleKind::File) return FALSE;
    ssize_t result = read(static_cast<linux64_detail::FileHandle*>(base)->fd, buffer, bytesToRead);
    if (result < 0) return FALSE;
    if (bytesRead) *bytesRead = static_cast<DWORD>(result);
    return TRUE;
}
inline BOOL WriteFile(HANDLE handle, LPCVOID buffer, DWORD bytesToWrite, LPDWORD bytesWritten, LPOVERLAPPED) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    if (base->kind != linux64_detail::HandleKind::File) return FALSE;
    ssize_t result = write(static_cast<linux64_detail::FileHandle*>(base)->fd, buffer, bytesToWrite);
    if (result < 0) return FALSE;
    if (bytesWritten) *bytesWritten = static_cast<DWORD>(result);
    return TRUE;
}
inline DWORD GetFileSize(HANDLE handle, LPDWORD high) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return INVALID_FILE_SIZE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    if (base->kind != linux64_detail::HandleKind::File) return INVALID_FILE_SIZE;
    struct stat st;
    if (fstat(static_cast<linux64_detail::FileHandle*>(base)->fd, &st) != 0) return INVALID_FILE_SIZE;
    uint64_t size = static_cast<uint64_t>(st.st_size);
    if (high) *high = static_cast<DWORD>(size >> 32);
    return static_cast<DWORD>(size & 0xFFFFFFFF);
}
inline DWORD SetFilePointer(HANDLE handle, LONG low, PLONG high, DWORD moveMethod) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return INVALID_FILE_SIZE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    if (base->kind != linux64_detail::HandleKind::File) return INVALID_FILE_SIZE;
    int64_t offset = static_cast<uint32_t>(low);
    if (high) offset |= (static_cast<int64_t>(*high) << 32);
    off_t result = lseek(static_cast<linux64_detail::FileHandle*>(base)->fd, static_cast<off_t>(offset), static_cast<int>(moveMethod));
    if (result == static_cast<off_t>(-1)) return INVALID_FILE_SIZE;
    if (high) *high = static_cast<LONG>((static_cast<uint64_t>(result) >> 32) & 0xFFFFFFFF);
    return static_cast<DWORD>(static_cast<uint64_t>(result) & 0xFFFFFFFF);
}
inline BOOL CloseHandle(HANDLE handle) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    switch (base->kind) {
    case linux64_detail::HandleKind::File:
        close(static_cast<linux64_detail::FileHandle*>(base)->fd);
        delete static_cast<linux64_detail::FileHandle*>(base);
        return TRUE;
    case linux64_detail::HandleKind::Find:
        closedir(static_cast<linux64_detail::FindHandle*>(base)->dir);
        delete static_cast<linux64_detail::FindHandle*>(base);
        return TRUE;
    default:
        return FALSE;
    }
}

inline BOOL DeleteFileA(LPCSTR path) {
    std::string normalised = linux64_detail::NormalisePath(path);
    return unlink(normalised.c_str()) == 0;
}
inline BOOL DeleteFileW(LPCWSTR path) {
    return DeleteFileA(linux64_detail::NormalisePath(path).c_str());
}
inline BOOL DeleteFile(LPCSTR path) { return DeleteFileA(path); }
inline BOOL DeleteFile(LPCWSTR path) { return DeleteFileW(path); }

inline BOOL CreateDirectoryA(LPCSTR path, LPSECURITY_ATTRIBUTES) {
    std::string normalised = linux64_detail::NormalisePath(path);
    return mkdir(normalised.c_str(), 0777) == 0 || errno == EEXIST;
}
inline BOOL CreateDirectoryW(LPCWSTR path, LPSECURITY_ATTRIBUTES attrs) {
    return CreateDirectoryA(linux64_detail::NormalisePath(path).c_str(), attrs);
}
inline BOOL RemoveDirectoryA(LPCSTR path) {
    std::string normalised = linux64_detail::NormalisePath(path);
    return rmdir(normalised.c_str()) == 0;
}
inline BOOL RemoveDirectoryW(LPCWSTR path) {
    return RemoveDirectoryA(linux64_detail::NormalisePath(path).c_str());
}

inline HANDLE FindFirstFileA(LPCSTR pattern, LPWIN32_FIND_DATAA outData) {
    std::string directory = linux64_detail::NormalisePath(pattern);
    const size_t wildcard = directory.find('*');
    if (wildcard != std::string::npos) directory.resize(wildcard);
    while (!directory.empty() && directory.back() == '/') directory.pop_back();
    if (directory.empty()) directory = ".";

    DIR* dir = opendir(directory.c_str());
    if (!dir) return INVALID_HANDLE_VALUE;

    auto* handle = new linux64_detail::FindHandle(dir, directory);
    if (!linux64_detail::FindNextEntry(handle, outData)) {
        closedir(dir);
        delete handle;
        return INVALID_HANDLE_VALUE;
    }
    return handle;
}
inline HANDLE FindFirstFileW(LPCWSTR pattern, LPWIN32_FIND_DATAA outData) {
    return FindFirstFileA(linux64_detail::NormalisePath(pattern).c_str(), outData);
}
inline HANDLE FindFirstFile(LPCSTR pattern, LPWIN32_FIND_DATAA outData) { return FindFirstFileA(pattern, outData); }
inline HANDLE FindFirstFile(LPCWSTR pattern, LPWIN32_FIND_DATAA outData) { return FindFirstFileW(pattern, outData); }
inline BOOL FindNextFileA(HANDLE handle, LPWIN32_FIND_DATAA outData) {
    if (!handle || handle == INVALID_HANDLE_VALUE) return FALSE;
    auto* base = static_cast<linux64_detail::HandleBase*>(handle);
    if (base->kind != linux64_detail::HandleKind::Find) return FALSE;
    return linux64_detail::FindNextEntry(static_cast<linux64_detail::FindHandle*>(base), outData);
}
inline BOOL FindNextFileW(HANDLE handle, LPWIN32_FIND_DATAA outData) { return FindNextFileA(handle, outData); }
inline BOOL FindNextFile(HANDLE handle, LPWIN32_FIND_DATAA outData) { return FindNextFileA(handle, outData); }

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
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11SamplerState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
typedef struct { long left, top, right, bottom; } D3D11_RECT;
typedef D3D11_RECT RECT;
typedef struct {
    FLOAT TopLeftX;
    FLOAT TopLeftY;
    FLOAT Width;
    FLOAT Height;
    FLOAT MinDepth;
    FLOAT MaxDepth;
} D3D11_VIEWPORT;

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
