-- NLC: SSS 23 water passes ( r4_rendertarget_phase_ssfx.cpp, phase_ssfx_water )

-- Reflection prepass: water geometry into $user$ssfx_water_a ( reduced size, no depth buffer )
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("water_regular", "ssfx_water_ssr")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_rimage", "$user$generic_temp")
	shader:dx10texture("sky_s0", "$user$sky0")
	shader:dx10texture("sky_s1", "$user$sky1")
	shader:dx10texture("s_bluenoise", "fx\\blue_noise")
	shader:dx10texture("s_ssfx_velocity", "$user$velocity")
	shader:dx10texture("s_ssfx_zbuffer", "$user$zbuffer")
	shader:dx10texture("s_ssfx_water_history", "$user$ssfx_water")
	shader:dx10sampler("smp_base")
	shader:dx10sampler("smp_linear")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
end

-- Blur, pass 1 ( a -> b )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_water_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("water_buffer", "$user$ssfx_water_a")
	shader:dx10sampler("smp_rtlinear")
end

-- Blur, pass 2 ( b -> a )
function element_2(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_water_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("water_buffer", "$user$ssfx_water_b")
	shader:dx10sampler("smp_rtlinear")
end

-- Animated wave height ( 512 x 512 ) for the parallax waves
function element_3(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_water_waves")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("water_waves", "fx\\ssfx_water_height")
	shader:dx10sampler("smp_linear")
end
