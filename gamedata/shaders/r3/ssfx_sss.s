-- NLC: SSS 23 screen-space shadows ( r4_rendertarget_phase_ssfx.cpp )

local function gbuffer(shader)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_diffuse", "$user$albedo")
	shader:dx10texture("s_ssfx_velocity", "$user$velocity")
	shader:dx10texture("s_ssfx_zbuffer", "$user$zbuffer")
	shader:dx10texture("s_prev_pos", "$user$ssfx_prev_pos")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
	shader:dx10sampler("smp_linear")
end

-- Sun: trace + temporal accumulation
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_sss")
		:fog(false)
		:zb(false, false)
	gbuffer(shader)
	shader:dx10texture("sss_image", "$user$ssfx_sss")
end

-- Sun: disocclusion blur, pass 1 ( a -> b )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_sss_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("sss_image", "$user$ssfx_sss_a")
	shader:dx10sampler("smp_nofilter")
end

-- Sun: disocclusion blur, pass 2 ( b -> a )
function element_2(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_sss_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("sss_image", "$user$ssfx_sss_b")
	shader:dx10sampler("smp_nofilter")
end

-- Lights, slots 0-3
function element_3(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_sss_lights")
		:fog(false)
		:zb(false, false)
	gbuffer(shader)
	shader:dx10texture("sss_image", "$user$ssfx_sss_ext")
end

-- Lights, slots 4-7
function element_4(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_sss_lights")
		:fog(false)
		:zb(false, false)
	gbuffer(shader)
	shader:dx10texture("sss_image", "$user$ssfx_sss_ext2")
end
