-- NLC .237: the sky + clouds behind the scene, blurred, for the fog colour ( docs/RAIN_CLOUDS_DESIGN.md 4.5,
-- r4_rendertarget_phase_ssfx.cpp phase_nlc_sky_bg, ssfx_fog_atmo.h SSFX_FOG_BG )
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "nlc_sky_bg")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_image", "$user$generic0")
	shader:dx10sampler("smp_rtlinear")
end
