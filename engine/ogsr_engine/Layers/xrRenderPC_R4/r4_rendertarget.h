#pragma once

#include "../xrRender/ColorMapManager.h"

class light;
class light_Package;

// NLC: SSS slot of a light for the accumulation shaders ( -1 = none ), r4_rendertarget_phase_ssfx.cpp
float ssfx_sss_slot(const light* L);

static void dummy(){}

class CRenderTarget : public IRender_Target
{
private:
    u32 dwWidth[R__NUM_CONTEXTS];
    u32 dwHeight[R__NUM_CONTEXTS];
    u32 m_renderWidth{};
    u32 m_renderHeight{};
    u32 m_displayWidth{};
    u32 m_displayHeight{};
    bool m_temporalUpscaleInput{};
    bool m_resetTemporalHistory{true};
    u32 dwAccumulatorClearMark;
    u32 dwFlareClearMark;

public:
    u32 dwLightMarkerID;

#ifdef DEBUG
    struct dbg_line_t
    {
        Fvector P0, P1;
        u32 color;
    };
    xr_vector<dbg_line_t> dbg_lines;
    xr_vector<Fplane> dbg_planes;
#endif

    // Base targets
    xr_vector<ref_rt> rt_base;
    ref_rt rt_Base_Depth;

    // MRT-path

    ref_rt rt_Position; // 64bit,	fat	(x,y,z,?)				(eye-space)
    ref_rt rt_Color; // 64/32bit,fat	(r,g,b,specular-gloss)	(or decompressed MET-8-8-8-8)
    ref_rt rt_Velocity; // r2_RT_velocity

    ref_rt rt_zbuffer; // r2_RT_zbuffer
    ref_rt rt_tempzb, rt_tempzb_dof;

    //
    ref_rt rt_Accumulator; // 64bit		(r,g,b,specular)
    ref_rt rt_Generic_0; // 32bit		(r,g,b,a)				// post-process, intermidiate results, etc.
    ref_rt rt_Generic_0_prev; // r2_RT_generic0_prev
    ref_rt rt_Generic_1; // 32bit		(r,g,b,a)				// post-process, intermidiate results, etc.
    ref_rt rt_Generic_scene_scratch; // low-resolution scene ping-pong target
    ref_rt rt_Generic_combine; // r2_RT_generic_combine
    ref_rt rt_Postprocess_0; // full-resolution color after temporal upscaling
    ref_rt rt_Generic_0_temp;
    ref_rt rt_Generic_combine_scope;

    //  Second viewport
    ref_rt rt_second_vp; // 32bit		(r,g,b,a) 

    ref_rt rt_dof;
    ref_rt rt_blur_2, rt_blur_h_2, rt_blur_4, rt_blur_h_4, rt_blur_8, rt_blur_h_8;
    ref_rt rt_mask_drops_blur;

    ref_rt rt_pp_bloom;

    ref_rt rt_ssfx_bloom1;
    ref_rt rt_ssfx_bloom_emissive;
    ref_rt rt_ssfx_bloom_lens;
    ref_rt rt_ssfx_bloom_tmp2;
    ref_rt rt_ssfx_bloom_tmp4;
    ref_rt rt_ssfx_bloom_tmp8;
    ref_rt rt_ssfx_bloom_tmp16;
    ref_rt rt_ssfx_bloom_tmp32;
    ref_rt rt_ssfx_bloom_tmp64;
    ref_rt rt_ssfx_bloom_tmp32_2;
    ref_rt rt_ssfx_bloom_tmp16_2;
    ref_rt rt_ssfx_bloom_tmp8_2;
    ref_rt rt_ssfx_bloom_tmp4_2;

    //	Igor: for volumetric lights
    ref_rt rt_Generic_2; // 32bit		(r,g,b,a)				// post-process, intermidiate results, etc.
    ref_rt rt_Generic_3; // 32bit		(r,g,b,a)				// post-process, intermidiate results, etc.
    ref_rt rt_Bloom_1; // 32bit, dim/4	(r,g,b,?)
    ref_rt rt_LUM_64; // 64bit, 64x64,	log-average in all components
    ref_rt rt_LUM_8; // 64bit, 8x8,		log-average in all components

    ref_rt rt_ssr1, rt_ssr2;

    // NLC: SSS 23 temporal passes ( r4_rendertarget_phase_ssfx.cpp ). Targets exist only when the pass is on.
    ref_rt rt_ssfx_prev_pos;
    ref_rt rt_ssfx_sss, rt_ssfx_sss_a, rt_ssfx_sss_b;
    ref_rt rt_ssfx_sss_ext, rt_ssfx_sss_ext2;
    ref_rt rt_ssfx_ao, rt_ssfx_ao_a, rt_ssfx_ao_b;
    ref_rt rt_ssfx_il, rt_ssfx_il_a, rt_ssfx_il_b;
    ref_rt rt_ssfx_il_local, rt_ssfx_il_local_a, rt_ssfx_il_local_b; // NLC: local-light bounce ( second IL output )
    ref_rt rt_ssfx_accum_pre; // NLC: accumulator snapshot before render_lights ( local light = accum - this )

    // NLC .207: analytic bounce of known lights ( docs/IL_DESIGN.md 4.3 ). The best SSFX_IL_LIGHTS_TOP lights of the
    // light package and ( .208 ) of the lights near the camera in sectors seen this frame, plus slots still fading out. Values are copied: a slot's key is only compared, never dereferenced.
    static constexpr u32 SSFX_IL_LIGHTS_TOP = 8;
    static constexpr u32 SSFX_IL_LIGHTS_MAX = 16; // must match ssfx_il_lights[] in ssfx_il.ps ( 2 x float4 per slot ); .208: 12 -> 16
    struct ssfx_il_light_slot
    {
        const void* key{}; // light pointer ( the omni light for OMNIPART pieces )
        Fvector pos{}, dir{}; // world
        Fvector col{}; // raw colour x LOD
        float range{}, fade{};
        bool spot{};
        // NLC .210 ( docs/IL_DESIGN.md 4.6 )
        Fvector centre{}; // fill centre, world ( spot: just in front of the point its beam hits )
        float reach{}; // before ssfx_il_lights_reach and the 12 m cap
        int vis{-1}; // index into ssfx_il_vis, -1 = none
    };
    ssfx_il_light_slot ssfx_il_slots[SSFX_IL_LIGHTS_MAX]{};
    u32 ssfx_il_slot_count{};

    // NLC .210: view-independent visibility of a slot's fill. An octahedral map of the distance to the static
    // geometry around the fill centre, traced on the CPU a few texels per frame and uploaded as shader constants
    // ( ssfx_il_vis[] in ssfx_il.ps ). Replaces the depth-buffer march for long rays: that one changed with the camera
    static constexpr u32 SSFX_IL_VIS_N = 16; // texels per side; must match SSFX_IL_VIS_N in ssfx_il.ps
    struct ssfx_il_vis_map
    {
        const void* key{}; // light pointer, only compared
        u32 used_frame{}; // last frame a slot referred to it
        bool valid{}, building{};
        Fvector origin{}, b_origin{}; // centre the map / the map in progress was traced from
        u32 b_next{}; // next texel of the map in progress
        float dist[SSFX_IL_VIS_N * SSFX_IL_VIS_N]{}, b_dist[SSFX_IL_VIS_N * SSFX_IL_VIS_N]{};
    };
    ssfx_il_vis_map ssfx_il_vis[SSFX_IL_LIGHTS_MAX]{};
    int ssfx_il_vis_acquire(const void* key);
    void ssfx_il_vis_update(u32 live_slots);
    void ssfx_il_collect_lights(const light_Package& LP);

    ref_shader s_ssfx_sss, s_ssfx_ao, s_ssfx_il;
    // Build B
    ref_rt rt_ssfx_water, rt_ssfx_water_a, rt_ssfx_water_b, rt_ssfx_water_waves;
    ref_rt rt_ssfx_fog_4, rt_ssfx_fog_2;
    ref_rt rt_ssfx_vol_sun; // reduced-size volumetric target, sun + spot ( ssfx_volumetric_resolution > 1 )
    ref_rt rt_ssfx_ssr, rt_ssfx_ssr_b, rt_ssfx_ssr_hist;
    ref_rt rt_ssfx_scene_prev; // NLC .209: last frame's finished scene for SSR / water reflections ( ssfx_scene_prev.h )
    u32 ssfx_scene_prev_frame{}; // frame of the copy, 0 = none
    ref_rt rt_ssfx_pos_world; // NLC .210: position buffer before the HUD and the rain pass, read by the AO
    u32 ssfx_pos_world_frame{}; // frame of the copy, 0 = none
    ref_rt rt_ssfx_rain; // NLC .85
    ref_shader s_ssfx_rain;
    ref_shader s_ssfx_water, s_ssfx_fog, s_ssfx_volumetric, s_ssfx_ssr;
    ref_texture t_ssfx_clouds0, t_ssfx_clouds1; // current weather clouds for the cloud shadows ( set every frame )
    Fmatrix ssfx_m_current{}, ssfx_m_previous{}; // view-space -> clip, current / previous frame
    bool ssfx_history_valid{}; // false after creation / resize: passes ignore last frame
    u32 ssfx_water_frame{}, ssfx_ssr_frame{}, ssfx_sss_frame{};
    bool ssfx_ssr_composited{}; // NLC: an SSR pass changed rt_Generic_0 this frame ( glass needs a fresh copy )
    bool ssfx_light_pass_empty[2]{}; // NLC: light SSS pass skipped ( no lights ), its history already cleared // NLC: last frame each pass ran ( history cleared on resume )

    ref_rt rt_LUM_pool[2]; // 1xfp32,1x1, exp-result -> scaler

    ref_texture t_LUM_src; // source
    ref_texture t_LUM_dest; // destination & usage for current frame

    // env
    ref_texture t_envmap_0; // env-0
    ref_texture t_envmap_1; // env-1

    // sun smap
    ref_rt rt_smap_sun_cascade[R__NUM_SUN_CASCADES];
    // rain smap
    ref_rt rt_smap_rain;
    // lights smap
    ref_rt rt_smap_lights;

    ID3DTexture2D* t_noise_surf[TEX_jitter_count];
    ref_texture t_noise[TEX_jitter_count];

    ref_shader s_puddles;

private:
    // OCCq

    ref_shader s_occq;

    // Accum
    ref_shader s_accum_mask;
    ref_shader s_accum_sun_cascade[R__NUM_SUN_CASCADES];
    ref_shader s_accum_direct_volumetric;
    ref_shader s_accum_point;
    ref_shader s_accum_spot;
    ref_shader s_accum_volume;

    ref_shader s_flare;

    //	DX10 Rain
    ref_shader s_rain;

    ref_geom g_accum_point;
    ref_geom g_accum_spot;
    ref_geom g_accum_omnipart;
    ref_geom g_flare;

    ID3DVertexBuffer* g_accum_point_vb;
    ID3DIndexBuffer* g_accum_point_ib;

    ID3DVertexBuffer* g_accum_omnip_vb;
    ID3DIndexBuffer* g_accum_omnip_ib;

    ID3DVertexBuffer* g_accum_spot_vb;
    ID3DIndexBuffer* g_accum_spot_ib;

    // Bloom
    ref_geom g_bloom_build;
    ref_geom g_bloom_filter;
    ref_shader s_bloom;
    float f_bloom_factor;

    ref_shader s_ssfx_bloom;
    ref_shader s_ssfx_bloom_lens;
    ref_shader s_ssfx_bloom_upsample;
    ref_shader s_ssfx_bloom_downsample;

    // Luminance
    ref_shader s_luminance;
    float f_luminance_adapt;

    // ScreenTriangle
    ref_geom TriangleGeom;
    ID3DVertexBuffer* TriangleVB{};
    ID3DIndexBuffer* TriangleIB{};

    // Combine
    ref_geom g_combine;
    ref_geom g_combine_2UV;
    ref_geom g_combine_cuboid;
    ref_shader s_combine;
    ref_shader s_combine_volumetric;

    ref_shader s_blur;
    ref_shader s_ssr;
    ref_shader s_dof;
    ref_shader s_lut;
    ref_shader s_taa;
    ref_shader s_cas;
    ref_shader s_temporal_resolve;

    ref_shader s_rain_drops;
    ref_shader s_gasmask_dudv;
    ref_shader s_fakescope; //crookr
    ref_shader s_nightvision;
    ref_shader s_heatvision;

    ref_rt rt_flares;
    ref_rt rt_fakescope; // crookr
public:
    ref_rt rt_heat;

    ref_shader s_postprocess;
    ref_geom g_postprocess;

private:
    float im_noise_time;
    u32 im_noise_shift_w;
    u32 im_noise_shift_h;

    float param_blur;
    float param_gray;
    float param_duality_h;
    float param_duality_v;
    float param_noise;
    float param_noise_scale;
    float param_noise_fps;
    u32 param_color_base;
    u32 param_color_gray;
    Fvector param_color_add;

    //	Color mapping
    float param_color_map_influence;
    float param_color_map_interpolate;
    ColorMapManager color_map_manager;

    //	Igor: used for volumetric lights
    bool m_bHasActiveVolumetric;

public:
    CRenderTarget();
    ~CRenderTarget();

    void reset_target_dimensions();

    void accum_point_geom_create();
    void accum_point_geom_destroy();
    void accum_omnip_geom_create();
    void accum_omnip_geom_destroy();
    void accum_spot_geom_create();
    void accum_spot_geom_destroy();

    void u_compute_texgen_screen(CBackend& cmd_list, Fmatrix& dest);

    ID3DRenderTargetView* get_base_rt() const { return rt_base[0/*HW.CurrentBackBuffer*/]->pRT; }
    ID3DDepthStencilView* get_base_zb() const { return rt_Base_Depth->pZRT[CHW::IMM_CTX_ID]; }

    void u_setrt(CBackend& cmd_list, const ref_rt& _1, const ref_rt& _2, const ref_rt& _3, const ref_rt& _4, ID3DDepthStencilView* zb);
    void u_setrt(CBackend& cmd_list, const ref_rt& _1, const ref_rt& _2, const ref_rt& _3, ID3DDepthStencilView* zb);
    void u_setrt(CBackend& cmd_list, const ref_rt& _1, const ref_rt& _2, ID3DDepthStencilView* zb);
    void u_setrt(CBackend& cmd_list, u32 W, u32 H, ID3DRenderTargetView* _1, ID3DRenderTargetView* _2, ID3DRenderTargetView* _3, ID3DDepthStencilView* zb);

    void u_calc_tc_noise(CBackend& cmd_list, Fvector2& p0, Fvector2& p1);
    void u_calc_tc_duality_ss(Fvector2& r0, Fvector2& r1, Fvector2& l0, Fvector2& l1);

    bool u_need_CM() const;

    void phase_scene_prepare();
    void phase_scene_begin(CBackend& cmd_list);
    void phase_occq(CBackend& cmd_list);
    void phase_wallmarks(CBackend& cmd_list);
    // only sun && rain
    void phase_smap_direct(CBackend& cmd_list, light* L, u32 sub_phase);
    void phase_smap_spot_clear(CBackend& cmd_list);
    void phase_smap_spot(CBackend& cmd_list, light* L) const;
    void phase_accumulator(CBackend& cmd_list);
    void phase_vol_accumulator(CBackend& cmd_list, bool cascade);
    void phase_blur(CBackend& cmd_list);
    void phase_dof(CBackend& cmd_list);
    void phase_lut(CBackend& cmd_list);
    void phase_gasmask_dudv(CBackend& cmd_list);
    void phase_nightvision(CBackend& cmd_list);
    void phase_heatvision(CBackend& cmd_list); //--DSR-- HeatVision
    void phase_heatvision_overlay(CBackend& cmd_list);

    void draw_rain(CBackend& cmd_list, light& RainSetup);

    bool need_to_render_sunshafts();

    void geom_volume(CBackend& cmd_list, light* L);
    void render_volume(CBackend& cmd_list, light* L);
    void accum_direct_cascade(CBackend& cmd_list, u32 sub_phase, Fmatrix& xform, Fmatrix& xform_prev, float fBias);
    void accum_direct_blend(CBackend& cmd_list);
    void accum_direct_volumetric(CBackend& cmd_list, u32 sub_phase, u32 Offset, u32 i_offset, const Fmatrix& mShadow);
    void accum_point(CBackend& cmd_list, light* L);
    void accum_spot(CBackend& cmd_list, light* L);
    void accum_volumetric(CBackend& cmd_list, light* L);
    void phase_bloom(CBackend& cmd_list);
    void phase_ssfx_bloom(CBackend& cmd_list);
    void phase_luminance(CBackend& cmd_list);
    void phase_combine(CBackend& cmd_list);
    void phase_pp(CBackend& cmd_list);
    void phase_combine_volumetric(CBackend& cmd_list);

    // NLC: SSS 23 temporal passes
    void ssfx_create_targets(u32 w, u32 h);
    void ssfx_begin_frame(); // matrices, once per frame before any ssfx pass
    void phase_ssfx_sss(CBackend& cmd_list); // sun screen-space shadows ( before sun accumulation )
    void phase_ssfx_sss_lights(CBackend& cmd_list, light_Package& LP); // up to 8 point/spot lights
    void phase_ssfx_ao(CBackend& cmd_list); // AO ( start of combine )
    void phase_ssfx_il(CBackend& cmd_list); // IL ( start of combine )
    void phase_ssfx_il_snapshot(CBackend& cmd_list, const light_Package& LP); // NLC: copy the accumulator before the point / spot lights, pick the IL lights
    void ssfx_end_frame(CBackend& cmd_list); // save position buffer for the next frame
    void phase_ssfx_water(CBackend& cmd_list); // water reflection prepass ( before the forward pass )
    void phase_ssfx_rain(CBackend& cmd_list); // NLC .85: rain refraction buffer ( before the forward pass )
    void phase_ssfx_fog_scattering(CBackend& cmd_list); // HDR, after volumetrics, before bloom
    ref_shader s_ssfx_haze; // NLC anomaly haze ( docs/ANOMALY_HAZE_DESIGN.md )
    bool phase_ssfx_haze(CBackend& cmd_list); // into rt_Generic_0; true when something was drawn ( render targets changed )
    bool phase_ssfx_ssr(CBackend& cmd_list); // SSS 23 SSR; false = use the OGSR SSR pass
    void ssfx_copy_pos_world(CBackend& cmd_list); // NLC .210: world G-buffer for the AO ( before the HUD )
    void phase_ssfx_scene_prev(CBackend& cmd_list); // NLC .209: keep the finished scene for the next frame's reflections
    float ssfx_scene_prev_strength() const; // ssfx_ssr_forward while last frame's copy exists, else 0
    bool ssfx_volumetric_reduced() const { return rt_ssfx_vol_sun && !ps_ssfx_compare; } // volumetrics into the reduced targets
    void ssfx_volumetric_restore_viewport(CBackend& cmd_list) const;

    void phase_rain_drops(CBackend& cmd_list);
    void phase_fakescope(CBackend& cmd_list); // crookr
    void phase_flares(CBackend& cmd_list);

    void render_flare(CBackend& cmd_list, light* L);

    virtual void set_blur(float f) { param_blur = f; }
    virtual void set_gray(float f) { param_gray = f; }
    virtual void set_duality_h(float f) { param_duality_h = _abs(f); }
    virtual void set_duality_v(float f) { param_duality_v = _abs(f); }
    virtual void set_noise(float f) { param_noise = f; }
    virtual void set_noise_scale(float f) { param_noise_scale = f; }
    virtual void set_noise_fps(float f) { param_noise_fps = _abs(f) + EPS_S; }
    virtual void set_color_base(u32 f) { param_color_base = f; }
    virtual void set_color_gray(u32 f) { param_color_gray = f; }
    virtual void set_color_add(const Fvector& f) { param_color_add = f; }

    virtual u32 get_width(CBackend& cmd_list) { return dwWidth[cmd_list.context_id]; }
    virtual u32 get_height(CBackend& cmd_list) { return dwHeight[cmd_list.context_id]; }

    u32 GetRenderWidth() const { return m_renderWidth; }
    u32 GetRenderHeight() const { return m_renderHeight; }
    u32 GetDisplayWidth() const { return m_displayWidth; }
    u32 GetDisplayHeight() const { return m_displayHeight; }
    u32 GetActiveWidth() const { return m_temporalUpscaleInput ? m_renderWidth : m_displayWidth; }
    u32 GetActiveHeight() const { return m_temporalUpscaleInput ? m_renderHeight : m_displayHeight; }
    u32 GetViewportWidth(CBackend& cmd_list) const { return dwWidth[cmd_list.context_id]; }
    u32 GetViewportHeight(CBackend& cmd_list) const { return dwHeight[cmd_list.context_id]; }
    float GetRenderSubrectScaleX() const { return static_cast<float>(m_renderWidth) / static_cast<float>(m_displayWidth); }
    float GetRenderSubrectScaleY() const { return static_cast<float>(m_renderHeight) / static_cast<float>(m_displayHeight); }
    bool IsTemporalUpscaleInput() const { return m_temporalUpscaleInput; }

    void SetTemporalRenderSize(u32 renderWidth, u32 renderHeight, u32 displayWidth, u32 displayHeight);
    void ConfigureTemporalRenderSize();
    void BeginTemporalUpscaleInput() { m_temporalUpscaleInput = true; }
    void EndTemporalUpscaleInput() { m_temporalUpscaleInput = false; }
    void ResetTemporalHistory();

    virtual void set_cm_imfluence(float f) { param_color_map_influence = f; }
    virtual void set_cm_interpolate(float f) { param_color_map_interpolate = f; }
    virtual void set_cm_textures(const shared_str& tex0, const shared_str& tex1) { color_map_manager.SetTextures(tex0, tex1); }

    //	Need to reset stencil only when marker overflows.
    //	Don't clear when render for the first time
    void reset_light_marker(CBackend& cmd_list, bool bResetStencil = false);
    void increment_light_marker(CBackend& cmd_list);

#ifdef DEBUG
    IC void dbg_addline(Fvector& P0, Fvector& P1, u32 c)
    {
        dbg_lines.push_back(dbg_line_t());
        dbg_lines.back().P0 = P0;
        dbg_lines.back().P1 = P1;
        dbg_lines.back().color = c;
    }
    IC void dbg_addplane(Fplane& P0, u32 c) { dbg_planes.push_back(P0); }
#else
    IC void dbg_addline(Fvector& P0, Fvector& P1, u32 c) {}
    IC void dbg_addplane(Fplane& P0, u32 c) {}
#endif

private:
    void RenderScreenTriangle(CBackend& cmd_list, const ref_rt& rt, ref_selement& sh, const std::function<void()>& lambda = dummy);
    // NLC: two render targets of the same size ( SV_Target0 / SV_Target1 )
    void RenderScreenTriangle2(CBackend& cmd_list, const ref_rt& rt0, const ref_rt& rt1, ref_selement& sh, const std::function<void()>& lambda = dummy);
    void RenderScreenQuad(CBackend& cmd_list, const u32 w, u32 const h, const ref_rt& rt, ref_selement& sh, const std::function<void()>& lambda = dummy);

    // Anti Aliasing
    ref_shader s_pp_antialiasing;
    ref_rt rt_smaa_edgetex;
    ref_rt rt_smaa_blendtex;

    bool PhaseAA(CBackend& cmd_list);
    bool Phase3DSSUpscale(CBackend& cmd_list, const bool lens_drawn); // NLC: !lens_drawn = only warm up the scope DLSS features

    void ProcessSMAA(CBackend& cmd_list);
    void ProcessTAA(CBackend& cmd_list);

    void InitDLSS();
    bool ProcessDLSS();
    void DestroyDLSS();

    void InitFSR();
    bool ProcessFSR();
    bool ProcessFSR_3DSS(const bool need_reset);
    void DestroyFSR();

    bool reset_3dss_rendertarget(const bool need_reset = false);

    void ProcessCAS(CBackend& cmd_list);
    void BeginPostprocess(CBackend& cmd_list, bool temporalOutput);

    void PhaseSSSS(CBackend& cmd_list);

    ref_rt rt_sunshafts_0; // ss0
    ref_rt rt_sunshafts_1; // ss1
    ref_rt rt_SunShaftsMask;
    ref_rt rt_SunShaftsMaskSmoothed;
    ref_rt rt_SunShaftsPass0;
    ref_shader s_ssss_mrmnwar;
    ref_shader s_ssss_ogse;

public:
    ID3D11Texture2D* m_ImguiTex{};
    ID3D11ShaderResourceView* m_ImguiSRV{};

    // NLC: true PiP scope view ( docs/SCOPE_TRUE_PIP_DESIGN.md, r4_scope_pip.cpp ). A square set of the targets the
    // scope pass writes, swapped in for the main ones ( members and the views of their stock-named textures ) while a
    // scope pass records, so every phase and shader works on it unchanged.
    struct PipTargets
    {
        ref_rt depth, position, color, accum, generic0, generic0_temp, zbuffer, scratch;
        // B2 ( docs/SCOPE_TRUE_PIP_DESIGN.md 15 ): optional, each only when its main target exists
        ref_rt generic1; // distortion mask
        ref_rt ao, ao_a, ao_b, prev_pos; // AO history / result / blur, previous scope position buffer ( r_3dss_pip_ao )
        ref_rt fog4, fog2; // fog scattering
        ref_rt rain; // rain refraction buffer
        ref_rt vol_sun, generic2, generic3; // volumetrics: reduced target, or the full pair ( r_3dss_pip_volumetric )
        ref_rt velocity; // motion vectors of the scope G-buffer ( AO / SSR history )
        ref_rt ssr, ssr_b, ssr_hist; // reflections ( r_3dss_pip_ssr )
        u32 size{};
        u32 flags{}; // PIP_T_* of this set
    };
    enum : u32
    {
        PIP_T_AO = 1u << 0,
        PIP_T_VOL = 1u << 1,
        PIP_T_SSR = 1u << 2,
    };
    PipTargets m_pip;
    bool m_pip_in{}; // the scope set is swapped in
    bool m_pip_history{}; // the scope AO history / previous position are from the last scope frame
    u32 m_pip_ssr_frame{}; // last frame the scope SSR ran ( its history is cleared after a gap )
    bool m_pip_vol{}; // volumetrics in the scope view this frame ( targets exist, option on )
    bool m_pip_vol_active{}; // the scope's volumetric targets were cleared and written this frame

    u32 pip_want_flags() const; // PIP_T_* the options ask for and the main targets allow
    bool pip_create(u32 size, u32 flags); // false: a target failed ( VRAM ), nothing kept
    bool pip_ao() const { return m_pip_in && (m_pip.flags & PIP_T_AO) && ps_r_3dss_pip_ao && !ps_ssfx_compare; } // scope AO this pass
    bool pip_ssr() const { return m_pip_in && (m_pip.flags & PIP_T_SSR) && ps_r_3dss_pip_ssr && !ps_ssfx_compare; } // scope SSR this pass
    void phase_pip_ao(CBackend& cmd_list, const Fmatrix& m_current, const Fmatrix& m_previous); // scope set swapped in
    bool phase_pip_ssr(CBackend& cmd_list, const Fmatrix& m_current, const Fmatrix& m_previous); // scope set swapped in
    void pip_destroy();
    void pip_swap(CBackend& cmd_list_imm, bool in);
    void phase_pip_combine(CBackend& cmd_list, R_dsgraph_structure& scope, const Fmatrix& m_previous); // clouds .. tonemap, scope set swapped in
    void pip_composite_begin(); // the lens samples the scope image ( $user$generic_combine_scope views )
    void pip_composite_end();

private:
    void draw_combine_1(CBackend& cmd_list); // NLC: combine_1 full-screen pass ( main and scope combine )
    void draw_ssfx_ao(CBackend& cmd_list, const Fmatrix& m_current, const Fmatrix& m_previous, bool scope, bool camera_motion); // NLC: main and scope AO
    bool draw_ssfx_ssr(CBackend& cmd_list, const Fmatrix& m_current, const Fmatrix& m_previous, float scene_prev_k, u32& hist_frame); // NLC: main and scope SSR

    struct PipSaved
    {
        u32 render_w{}, render_h{}, display_w{}, display_h{}, view_w{}, view_h{};
        u32 accum_mark{}, light_marker{};
        bool volumetric{};
    } m_pip_saved;
    u32 m_pip_accum_mark{};
    u32 m_pip_light_marker{5};
};
