-- NLC .85: SSS 22 rain refraction buffer ( r4_rendertarget_phase_ssfx.cpp phase_ssfx_rain )

-- Reduced volumetrics ( sun + spot in one target )
function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_rain")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ssfx_color_buffer", "$user$generic0")
	shader:dx10texture("volumetric_buffer", "$user$ssfx_vol_sun")
	shader:dx10texture("volumetric_buffer2", "$user$ssfx_vol_sun")
	shader:dx10sampler("smp_rtlinear")
end

-- Full-size volumetrics ( sun in generic2, spot in generic3 )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_rain")
		:fog(false)
		:zb(false, false)
	shader:dx10texture("ssfx_color_buffer", "$user$generic0")
	shader:dx10texture("volumetric_buffer", "$user$generic2")
	shader:dx10texture("volumetric_buffer2", "$user$generic3")
	shader:dx10sampler("smp_rtlinear")
end
