-- NLC .216: splash on impacts in puddles ( dxRainRender::render_impacts ). The rain splash mesh and look; the pixel
-- shader shows it only where the G-buffer at the impact point is a terrain puddle
function normal(shader, t_base, t_second, t_detail)
	shader	: begin	("effects_rain","nlc_impact_splash")
			: zb	(true,false)
			: blend	(true,blend.srcalpha,blend.invsrcalpha)
			: aref 		(true,0)

	shader : dx10texture("s_base", "fx\\rain_splash")
	shader : dx10texture("s_rimage", "$user$generic_temp")
	shader : dx10texture("s_position", "$user$position")
	shader : dx10texture("s_diffuse", "$user$albedo")

	shader 	: dx10sampler	("smp_base")
	shader 	: dx10sampler	("smp_nofilter")
end
