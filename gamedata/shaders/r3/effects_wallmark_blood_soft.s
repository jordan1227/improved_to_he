-- NLC: blood on ground that soaks it in (earth, grass, sand...). Chosen by the engine from the
-- hit material (misclood_pool_surfaces.ltx). The blood is multiplied into the ground colour,
-- so the ground texture shows through, and no gloss is written (the ground keeps its own).

function normal(shader, t_base, t_second, t_detail)
	shader 	: begin("effects_wallmark_blood", "effects_wallmark_blood_soft")
			: sorting(3, false)
			: blend(true, blend.destcolor, blend.zero)
			: zb(true, false)
			: fog(false)
			: wmark(true)

	shader:dx10texture	("s_base", t_base)

	shader:dx10sampler	("smp_base")
	shader:dx10sampler	("smp_rtlinear")
	shader:dx10color_write_enable(true, true, true, false)
end
