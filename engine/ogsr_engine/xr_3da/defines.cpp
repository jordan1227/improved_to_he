#include "stdafx.h"

#ifdef DEBUG
ECORE_API BOOL bDebug = FALSE;
#endif

// Video
u32 psCurrentVidMode[2] = {1024, 768};

// release version always has "mt_*" enabled
Flags32 psDeviceFlags{rsDetails | rsAlwaysActive | rs_SSFX_HUD_RAINDROPS};
float ps_wm_wash_seconds = 240.f;
int ps_wm_rotfix = 0; // NLC: blood pools pass an explicit rotation (bpm); other wallmarks keep the vanilla random one
