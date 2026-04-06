// Linux64_TextureLoader.cpp — stb_image-based LoadTextureData for the Linux64 port
// This replaces the D3DX texture loading that the Windows64 4J_Render_PC.lib provides.
#ifdef _LINUX64
#include "stdafx.h"
#include <cstring>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "Linux64/stb_image.h"

// Forward: NormalisePath from compat
namespace linux64_detail {
    std::string NormalisePath(const char* path);
}

// Implement both LoadTextureData overloads.
// The game expects ARGB pixel data (int*) and D3DXIMAGE_INFO with Width/Height.

HRESULT C4JRender::LoadTextureData(const char *pchFilename, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    if (pSrcInfo != nullptr) ZeroMemory(pSrcInfo, sizeof(*pSrcInfo));
    if (ppDataOut != nullptr) *ppDataOut = nullptr;

    if (pchFilename == nullptr) return E_FAIL;

    // Normalise path separators
    std::string normalised = linux64_detail::NormalisePath(pchFilename);

    int w = 0, h = 0, channels = 0;
    unsigned char *pixels = stbi_load(normalised.c_str(), &w, &h, &channels, 4); // force RGBA

    if (pixels == nullptr)
    {
        fprintf(stderr, "[Linux64] stbi_load failed: %s (%s)\n", stbi_failure_reason(), normalised.c_str());
        return E_FAIL;
    }

    // Convert from RGBA to ARGB (int array, game expects ARGB)
    int *argb = new int[w * h];
    for (int i = 0; i < w * h; i++)
    {
        unsigned char r = pixels[i * 4 + 0];
        unsigned char g = pixels[i * 4 + 1];
        unsigned char b = pixels[i * 4 + 2];
        unsigned char a = pixels[i * 4 + 3];
        argb[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    stbi_image_free(pixels);

    if (pSrcInfo != nullptr)
    {
        pSrcInfo->Width = w;
        pSrcInfo->Height = h;
    }
    if (ppDataOut != nullptr)
    {
        *ppDataOut = argb;
    }
    else
    {
        delete[] argb;
    }
    return ERROR_SUCCESS;
}

HRESULT C4JRender::LoadTextureData(BYTE *pData, DWORD dwDataSize, D3DXIMAGE_INFO *pSrcInfo, int **ppDataOut)
{
    if (pSrcInfo != nullptr) ZeroMemory(pSrcInfo, sizeof(*pSrcInfo));
    if (ppDataOut != nullptr) *ppDataOut = nullptr;

    if (pData == nullptr || dwDataSize == 0) return E_FAIL;

    int w = 0, h = 0, channels = 0;
    unsigned char *pixels = stbi_load_from_memory(pData, static_cast<int>(dwDataSize), &w, &h, &channels, 4);

    if (pixels == nullptr)
    {
        fprintf(stderr, "[Linux64] stbi_load_from_memory failed: %s\n", stbi_failure_reason());
        return E_FAIL;
    }

    int *argb = new int[w * h];
    for (int i = 0; i < w * h; i++)
    {
        unsigned char r = pixels[i * 4 + 0];
        unsigned char g = pixels[i * 4 + 1];
        unsigned char b = pixels[i * 4 + 2];
        unsigned char a = pixels[i * 4 + 3];
        argb[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    stbi_image_free(pixels);

    if (pSrcInfo != nullptr)
    {
        pSrcInfo->Width = w;
        pSrcInfo->Height = h;
    }
    if (ppDataOut != nullptr)
    {
        *ppDataOut = argb;
    }
    else
    {
        delete[] argb;
    }
    return ERROR_SUCCESS;
}

#endif // _LINUX64
