-- NLC: anomaly haze ( docs/ANOMALY_HAZE_DESIGN.md, r4_rendertarget_phase_ssfx.cpp phase_ssfx_haze )
-- One draw per volume: rgb = in-scattered light, a = transmittance; dst = src + dst x a
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_haze")
		:fog(false)
		:zb(false, false)
		:blend(true, blend.one, blend.srcalpha)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10sampler("smp_nofilter")
end
