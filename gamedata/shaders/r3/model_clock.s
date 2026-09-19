local tex_lut = "shaders\\watch\\he_clock_digits"

function normal   (shader, t_base, t_second, t_detail)
	  shader:begin	("model_def_lplanes","model_clock")
      : fog         (false)
      : zb          (true,false)
      : blend       (true,blend.srcalpha,blend.one)
      : aref        (true,0)
      : sorting     (2, true)
      : scope_pass  (true)

	shader:dx10texture("s_base", t_base)
	shader:dx10texture("s_lut", tex_lut)
	shader:dx10texture("s_he_battery", "act\\watch\\bar_battery")
	shader:dx10texture("s_he_alarm", "act\\watch\\icon_alarm")
	shader:dx10texture("s_he_rad", "shaders\\watch\\he_radiation_digits")
	
	shader:dx10sampler("smp_base")	
	shader:dx10sampler("smp_nofilter")		
end
