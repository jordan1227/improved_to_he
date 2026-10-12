-- NLC: SSS 23 SSR ( r4_rendertarget_phase_ssfx.cpp, phase_ssfx_ssr ), replaces ogsr_ssr when on

-- Reflection + temporal accumulation
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_ssr")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_diffuse", "$user$albedo")
	shader:dx10texture("s_rimage", "$user$generic_temp")
	shader:dx10texture("s_vollight", "$user$generic2")
	shader:dx10texture("env_s0", "$user$env_s0")
	shader:dx10texture("env_s1", "$user$env_s1")
	shader:dx10texture("sky_s0", "$user$sky0")
	shader:dx10texture("sky_s1", "$user$sky1")
	shader:dx10texture("s_ssfx_velocity", "$user$velocity")
	shader:dx10texture("s_ssfx_zbuffer", "$user$zbuffer")
	shader:dx10texture("s_prev_pos", "$user$ssfx_prev_pos")
	shader:dx10texture("s_ssfx_ssr_history", "$user$ssfx_ssr_hist")
	shader:dx10sampler("smp_base")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_linear")
	shader:dx10sampler("smp_rtlinear")
end

-- Gloss blur, pass 1 ( ssfx_ssr -> ssfx_ssr_b )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_ssr_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ssr_image", "$user$ssfx_ssr")
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_diffuse", "$user$albedo")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
end

-- Gloss blur, pass 2 ( ssfx_ssr_b -> ssfx_ssr )
function element_2(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_ssr_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ssr_image", "$user$ssfx_ssr_b")
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_diffuse", "$user$albedo")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
end

-- Combine with the scene
function element_3(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_ssr_combine")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_image", "$user$generic_temp")
	shader:dx10texture("ssr_image", "$user$ssfx_ssr")
	shader:dx10sampler("smp_nofilter")
end
