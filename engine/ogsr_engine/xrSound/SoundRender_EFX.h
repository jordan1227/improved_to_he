#pragma once

// NLC: EFX filter entry points shared by the core and the OpenAL targets (loaded in CSoundRender_Core::InitAlEFXAPI)
#include <efx.h>

namespace sndefx
{
inline LPALGENFILTERS GenFilters{};
inline LPALDELETEFILTERS DeleteFilters{};
inline LPALISFILTER IsFilter{};
inline LPALFILTERI Filteri{};
inline LPALFILTERF Filterf{};

inline bool filters_ok() { return GenFilters && DeleteFilters && IsFilter && Filteri && Filterf; }
} // namespace sndefx
