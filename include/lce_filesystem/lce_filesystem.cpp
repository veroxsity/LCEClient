#include "stdafx.h"
#include "lce_filesystem.h"

#ifdef _WINDOWS64
#include <windows.h>
#endif // TODO: More os' filesystem handling for when the project moves away from only Windows

#ifdef _LINUX64
#include <dirent.h>
#include <sys/stat.h>
#include <string>
#endif

#include <stdio.h>

bool FileOrDirectoryExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES);
#elif defined(_LINUX64)
    struct stat st;
    return stat(path, &st) == 0;
#else
    #error "FileOrDirectoryExists not implemented for this platform"
    return false;
#endif
}

bool FileExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES && !(attribs & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(_LINUX64)
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
#else
    #error "FileExists not implemented for this platform"
    return false;
#endif
}

bool DirectoryExists(const char* path)
{
#ifdef _WINDOWS64
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(_LINUX64)
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
#else
    #error "DirectoryExists not implemented for this platform"
    return false;
#endif
}

bool GetFirstFileInDirectory(const char* directory, char* outFilePath, size_t outFilePathSize)
{
#ifdef _WINDOWS64
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", directory);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            // Found a file, copy its path to the output buffer
            snprintf(outFilePath, outFilePathSize, "%s\\%s", directory, findData.cFileName);
            FindClose(hFind);
            return true;
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    FindClose(hFind);
    return false; // No files found in the directory
#elif defined(_LINUX64)
    DIR* dir = opendir(directory);
    if (!dir)
    {
        return false;
    }

    struct dirent* entry = nullptr;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        std::string candidate = std::string(directory) + "/" + entry->d_name;
        struct stat st;
        if (stat(candidate.c_str(), &st) == 0 && S_ISREG(st.st_mode))
        {
            snprintf(outFilePath, outFilePathSize, "%s", candidate.c_str());
            closedir(dir);
            return true;
        }
    }

    closedir(dir);
    return false;
#else
    #error "GetFirstFileInDirectory not implemented for this platform"
    return false;
#endif
}
