-- NLC: reduced-size volumetric lights ( ssfx_volumetric_resolution > 1 ), upsample in phase_combine_volumetric

function element_0(shader, t_base, t_second, t_detail)
	shader:begin("stub_screen_space", "ssfx_volumetric_combine")
		:fog(false)
		:zb(false, false)
		:blend(true, blend.one, blend.one)
		:sorting(2, false)
	shader:dx10texture("s_vol_sun", "$user$ssfx_vol_sun")
	shader:dx10texture("s_position", "$user$position")
	shader:dx10sampler("smp_nofilter")
end
