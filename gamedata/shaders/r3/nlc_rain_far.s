-- NLC: distant rain ( docs/RAIN_CLOUDS_DESIGN.md D1 / D2 / G3, r4_rendertarget_phase_ssfx.cpp phase_nlc_rain_far )
-- rgb = added light, a = transmittance; dst = src + dst x a
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "nlc_rain_far")
		:fog(false)
		:zb(false, false)
		:blend(true, blend.one, blend.srcalpha)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_fog_map", "$user$smap_rain")
	shader:dx10texture("s_diffuse", "$user$albedo") -- .237: gloss, ground hits only on wet surfaces
	-- .237: the veil takes the fog colour ( ssfx_fog_atmo.h, sky near the horizon )
	shader:dx10texture("s_fog_sky0", "$user$sky0")
	shader:dx10texture("s_fog_sky1", "$user$sky1")
	shader:dx10texture("s_fog_bg", "$user$nlc_sky_bg") -- .237: drawn sky + clouds ( SSFX_FOG_BG )
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
end
