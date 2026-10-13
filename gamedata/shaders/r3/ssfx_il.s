-- NLC: SSS 23 temporal indirect light ( r4_rendertarget_phase_ssfx.cpp )

-- IL + temporal accumulation
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_il")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_diffuse", "$user$albedo")
	shader:dx10texture("s_accumulator", "$user$accum")
	shader:dx10texture("s_ssfx_velocity", "$user$velocity")
	shader:dx10texture("s_ssfx_zbuffer", "$user$zbuffer")
	shader:dx10texture("s_prev_pos", "$user$ssfx_prev_pos")
	shader:dx10texture("ssfx_il_history", "$user$ssfx_il")
	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_rtlinear")
	shader:dx10sampler("smp_linear")
end

-- Blur, pass 1 ( a -> b )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_il_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ao_image", "$user$ssfx_il_a")
	shader:dx10sampler("smp_rtlinear")
end

-- Blur, pass 2 ( b -> a )
function element_2(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_il_blur")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ao_image", "$user$ssfx_il_b")
	shader:dx10sampler("smp_rtlinear")
end
