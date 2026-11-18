-- NLC .209p: nlc_rain_notransform_2uv keeps the pass inside the far plane ( see the .vs )
function element_1(shader, t_base, t_second, t_detail)
	shader:begin("nlc_rain_notransform_2uv", "rain_patch_normal")
		:fog(false)
		:zb(true, false, true)
	shader:dx10zfunc(cmp_func.greater)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_lmap", "sunmask")
	shader:dx10texture("s_smap", "$user$smap_rain")
	shader:dx10texture("s_fog_map", "$user$smap_rain") -- NLC .216: plain depth reads ( height of the cover )
	shader:dx10texture("s_diffuse", "$user$albedo")

	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_material")
	shader:dx10sampler("smp_linear")
	shader:dx10sampler("smp_base")
	
	jitter.jitter(shader)
	shader:dx10sampler("smp_smap")
	
	shader:dx10texture("s_water", "fx\\water_sbumpvolume") -- NLC .209i: the 2D ripple atlas of the terrain / water ( was water\\water_SBumpVolume )
	shader:dx10texture("s_waterFall", "water\\water_flowing_nmap")
end

function element_2(shader, t_base, t_second, t_detail)
	shader:begin("nlc_rain_notransform_2uv", "rain_apply_normal")
		:fog(false)
		:zb(true, false, true)
	shader:dx10zfunc(cmp_func.greater)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_lmap", "sunmask")
	shader:dx10texture("s_smap", "$user$smap_rain")

	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_material")
	shader:dx10sampler("smp_linear")
	
	jitter.jitter(shader)
	shader:dx10sampler("smp_smap")
	
	shader:dx10texture("s_patched_normal", "$user$accum")

	shader:dx10color_write_enable(true, true, false, false)
end

function element_3(shader, t_base, t_second, t_detail)
	shader:begin("nlc_rain_notransform_2uv", "rain_apply_gloss")
		:fog(false)
		:zb(true, false, true)
		:blend(true, blend.one, blend.one)
	shader:dx10zfunc(cmp_func.greater)
	shader:dx10texture("s_position", "$user$position")
	shader:dx10texture("s_lmap", "sunmask")
	shader:dx10texture("s_smap", "$user$smap_rain")

	shader:dx10sampler("smp_nofilter")
	shader:dx10sampler("smp_material")
	shader:dx10sampler("smp_linear")
	
	jitter.jitter(shader)
	shader:dx10sampler("smp_smap")
	
	shader:dx10texture("s_patched_normal", "$user$accum")
	
	shader:dx10setrs(19, 1)
	shader:dx10setrs(20, 3)
end