-- NLC: explicit elements (the old normal/l_special workflow compiles no shadow elements, so models with this shader,
-- among them the visible-body puppet sm\actor_legs\*, were never drawn into the shadow maps). Element ids:
-- 0 normal HQ, 1 normal LQ, 2 smap (sun), 3 smap (lights), 4 l_special. Keep normal/l_special defined: the resource
-- manager looks for them to decide that the script is a shader.
local function deferred(shader, t_base, t_second, t_detail)
    shader:begin    ("deffer_model_flat","deffer_base_flat")

	: fog (false)
	: zb  (true,true)

    shader:dx10stencil    (     true, cmp_func.always,
                            255 , 127,
                            stencil_op.keep, stencil_op.replace, stencil_op.keep)
    shader:dx10stencil_ref    (1)

    shader:dx10texture("s_base",    t_base)
    shader:dx10texture("s_bump",    t_base.."_bump")
    shader:dx10texture("s_bumpX",   t_base.."_bump#")

    shader:dx10sampler("smp_base")
	shader:dx10sampler ("smp_rtlinear")
end

local function self_light(shader, t_base, t_second, t_detail)
	shader:begin	("shadow_direct_model",	"accum_emissivel")
			: zb 		(true,true)
			: fog		(true)
			: emissive 	(true)
    shader:dx10texture("s_base", t_base)
    shader:dx10sampler("smp_base")
end

-- like Blender_Model_EbB / blender_deffer_model for SE_R2_SHADOW (non alpha-tested), but the pixel shader clips everything above
-- the puppet's pelvis (m_nlc_vb_shadow.x), see shadow_direct_model_legs.vs / shadow_legs.ps
local function shadow(shader, t_base, t_second, t_detail)
    shader:begin    ("shadow_direct_model_legs","shadow_legs")
			: fog		(false)
			: zb 		(true,true)
    shader:dx10texture("s_base", t_base)
    shader:dx10sampler("smp_base")
    shader:dx10sampler("smp_linear")
    shader:dx10color_write_enable(false, false, false, false)
end

function normal        (shader, t_base, t_second, t_detail) deferred(shader, t_base, t_second, t_detail) end
function l_special	(shader, t_base, t_second, t_detail) self_light(shader, t_base, t_second, t_detail) end

function element_0 (shader, t_base, t_second, t_detail) deferred(shader, t_base, t_second, t_detail) end
function element_1 (shader, t_base, t_second, t_detail) deferred(shader, t_base, t_second, t_detail) end
function element_2 (shader, t_base, t_second, t_detail) shadow(shader, t_base, t_second, t_detail) end
function element_3 (shader, t_base, t_second, t_detail) shadow(shader, t_base, t_second, t_detail) end
function element_4 (shader, t_base, t_second, t_detail) self_light(shader, t_base, t_second, t_detail) end
