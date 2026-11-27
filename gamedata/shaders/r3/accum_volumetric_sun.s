function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("accum_volumetric_sun", "accum_volumetric_sun")
			: fog		(false)
			: zb 		(false,false)
			: blend		(true,blend.one,blend.one)
			: sorting	(2, false)
	shader:dx10texture	("s_smap", "$user$smap_sun_cascade_2") --лучи рендерятся на последнем каскаде
	shader:dx10texture	("s_position", "$user$position")
	shader:dx10texture	("jitter0", "$user$jitter_0")
	shader:dx10texture	("s_ssfx_clouds0", "$user$ssfx_clouds0") -- NLC K2: cloud shadows in the volumetric sun
	shader:dx10texture	("s_ssfx_clouds1", "$user$ssfx_clouds1")
	shader:dx10sampler	("smp_linear")

	shader:dx10sampler	("smp_base")
	shader:dx10sampler	("smp_jitter")
	shader:dx10sampler	("smp_smap")
	shader:dx10sampler	("smp_nofilter")
end