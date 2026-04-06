#pragma once

// Linux64 Phase 1 uses the shipped Iggy declarations so the shared UI code
// keeps compiling, but the runtime itself is stubbed out.
#include "../../../Windows64/Iggy/include/iggy.h"

inline void IggyFontInstallTruetypeUTF8(const void *, S32, const char *, S32, U32) {}
inline void IggyFontInstallTruetypeFallbackCodepointUTF8(const char *, S32, U32, S32) {}
inline void IggyFontInstallBitmapUTF8(const IggyBitmapFontProvider *, const char *, S32, U32) {}
inline void IggyFontRemoveUTF8(const char *, S32, U32) {}
inline void IggyFontSetIndirectUTF8(const char *, S32, U32, const char *, S32, U32) {}
inline void IggyFlushInstalledFonts() {}

inline void gdraw_D3D11_PreReset() {}
inline void gdraw_D3D11_PostReset() {}
inline void gdraw_D3D11_SetRendertargetSize(int, int) {}
