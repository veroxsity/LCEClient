#pragma once
// Linux64 Iggy stub — all Flash UI calls are no-ops in Phase 1
typedef void* IggyPlayer;
inline void IggyFlushInstalledFonts() {}
inline void gdraw_D3D11_PreReset() {}
inline void gdraw_D3D11_PostReset() {}
inline void gdraw_D3D11_SetRendertargetSize(int, int) {}
