
function normal   (shader, t_base, t_second, t_detail)
  shader:begin    ("model_distort4ghost","base_lplanes")
      : fog       (true)
      : zb        (false,false)
      : blend     (true,blend.srcalpha,blend.invsrcalpha)
      : aref      (true,0)
      : sorting   (3,true)
	shader: dx10texture ("s_base", t_base)
	shader: dx10sampler ("smp_base")
	shader: dx10sampler ("smp_rtlinear")
end