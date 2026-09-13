local tex_env0	= "$user$sky0"
local tex_env1	= "$user$sky1"

function normal   (shader, t_base, t_second, t_detail)
	  shader:begin	("model_def_lplanes","model_compass_icon")
      : fog         (false)
      : zb          (true,false)
      : blend       (true,blend.srcalpha,blend.one)
      : aref        (true,0)
      : sorting     (2, true)
      : scope_pass  (true)
	shader:dx10texture	("s_base",	t_base)
	shader:dx10texture	("s_he_band",	"act\\watch\\rad_band")
	shader:dx10sampler	("smp_base")
end
