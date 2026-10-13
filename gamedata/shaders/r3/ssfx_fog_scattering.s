-- NLC: SSS 23 fog scattering ( r4_rendertarget_phase_ssfx.cpp, phase_ssfx_fog_scattering )

-- Scattering: scene + blurred scene by fog density
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_fog_scattering")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_image", "$user$generic0")
	shader:dx10texture("s_fog_blur", "$user$ssfx_fog_2")
	shader:dx10texture("s_position", "$user$position")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
end

-- Blur: scene -> 1/4
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_fog_scattering_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_image", "$user$generic0")
	shader:dx10sampler("smp_rtlinear")
end

-- Blur: 1/4 -> 1/2
function element_2(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_fog_scattering_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_image", "$user$ssfx_fog_4")
	shader:dx10sampler("smp_rtlinear")
end
