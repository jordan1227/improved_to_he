-- @ Version: SCREEN SPACE SHADERS - UPDATE 17
-- @ Description: Rain - Shader Config
-- @ Author: https://www.moddb.com/members/ascii1457
-- @ Mod: https://www.moddb.com/mods/stalker-anomaly/addons/screen-space-shaders
-- @ NLC .237 ( docs/RAIN_CLOUDS_DESIGN.md 4.5 ): G-buffer position and gloss under the splash ( fade by the surface hit )

function normal(shader, t_base, t_second, t_detail)
	shader	: begin	("effects_rain","effects_rain_splash")
			: zb	(true,false)
			: blend	(true,blend.srcalpha,blend.invsrcalpha)
			: aref 		(true,0)
			 
	shader : dx10texture("s_base", "fx\\rain_splash")
	shader : dx10texture("s_rimage", "$user$generic_temp")
	shader : dx10texture("s_position", "$user$position") -- NLC .237
	shader : dx10texture("s_diffuse", "$user$albedo") -- NLC .237: gloss = wet

	shader 	: dx10sampler	("smp_base")
	shader 	: dx10sampler	("smp_nofilter")
end
