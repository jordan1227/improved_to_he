// BlenderDefault.cpp: implementation of the CBlender_BmmD class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "uber_deffer.h"
#include "blender_BmmD.h"

#include <mutex>

// NLC .212f: how soft ( porous, earth / grass ) each terrain detail texture is when wet, for deffer_terrain_high_flat.ps
// ( puddle shape, damp look, wet sheen ). Read from config\environment\terrain_wet.ltx: [terrain_wet_textures] exact
// names, else the longest matching key of [terrain_wet_keywords]; unknown textures keep the shader's colour test.
// The shader gets softness + 1 per splat channel ( ssfx_terrain_soft; 0 = unknown, also with an older exe ).
namespace
{
class nlc_terrain_soft_setup final : public R_constant_setup
{
public:
    Fvector4 v{};
    void setup(CBackend& cmd_list, R_constant* C) override { cmd_list.set_c(C, v); }
};

std::mutex nlc_terrain_soft_lock;
CInifile* nlc_terrain_soft_ini{};
bool nlc_terrain_soft_loaded{};
struct nlc_terrain_wet_pair
{
    nlc_terrain_soft_setup* soft;
    nlc_terrain_soft_setup* puddles;
};

xr_map<xr_string, nlc_terrain_wet_pair> nlc_terrain_soft_setups; // kept for the shaders' lifetime

// "softness[, puddles]" -> ( softness, puddles ); -1 = unknown / not given
Fvector2 nlc_terrain_wet_parse(LPCSTR val)
{
    if (!val || !val[0]) // a key without a value
        return {-1.f, -1.f};
    Fvector2 r{std::clamp(static_cast<float>(atof(val)), 0.f, 1.f), -1.f};
    if (LPCSTR comma = strchr(val, ','))
        r.y = std::clamp(static_cast<float>(atof(comma + 1)), 0.f, 1.f);
    return r;
}

Fvector2 nlc_terrain_wet_lookup(LPCSTR tex) // ( softness, puddles ), -1 = unknown
{
    if (!nlc_terrain_soft_ini || !tex || !tex[0])
        return {-1.f, -1.f};

    xr_string name = tex;
    for (char& c : name)
        c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

    if (nlc_terrain_soft_ini->section_exist("terrain_wet_textures"))
        for (const auto& [key, val] : nlc_terrain_soft_ini->r_section("terrain_wet_textures").Ordered_Data)
        {
            xr_string k = key.c_str();
            for (char& c : k)
                c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
            if (k == name)
                return nlc_terrain_wet_parse(val.c_str());
        }

    Fvector2 best{-1.f, -1.f};
    size_t best_len = 0;
    if (nlc_terrain_soft_ini->section_exist("terrain_wet_keywords"))
        for (const auto& [key, val] : nlc_terrain_soft_ini->r_section("terrain_wet_keywords").Ordered_Data)
        {
            xr_string k = key.c_str();
            for (char& c : k)
                c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
            if (!k.empty() && k.size() > best_len && name.find(k) != xr_string::npos)
            {
                best = nlc_terrain_wet_parse(val.c_str());
                best_len = k.size();
            }
        }
    return best;
}

// .213b: second value per texture = puddle amount ( 0 none, 1 full; grass soaks it up ), ssfx_terrain_puddle
nlc_terrain_wet_pair nlc_terrain_soft_get(LPCSTR r, LPCSTR g, LPCSTR b, LPCSTR a)
{
    std::scoped_lock lock(nlc_terrain_soft_lock);

    if (!nlc_terrain_soft_loaded)
    {
        nlc_terrain_soft_loaded = true;
        string_path fn;
        if (FS.exist(fn, "$game_config$", "environment\\terrain_wet.ltx"))
            nlc_terrain_soft_ini = xr_new<CInifile>(fn, TRUE, TRUE, FALSE);
        else
            Msg("! [nlc_terrain_wet] config\\environment\\terrain_wet.ltx not found: colour test only");
    }

    xr_string key = xr_string(r) + "|" + g + "|" + b + "|" + a;
    auto it = nlc_terrain_soft_setups.find(key);
    if (it != nlc_terrain_soft_setups.end())
        return it->second;

    const Fvector2 s[4]{nlc_terrain_wet_lookup(r), nlc_terrain_wet_lookup(g), nlc_terrain_wet_lookup(b), nlc_terrain_wet_lookup(a)};
    nlc_terrain_wet_pair p{xr_new<nlc_terrain_soft_setup>(), xr_new<nlc_terrain_soft_setup>()};
    p.soft->v.set(s[0].x + 1.f, s[1].x + 1.f, s[2].x + 1.f, s[3].x + 1.f); // -1 ( unknown ) -> 0
    p.puddles->v.set(s[0].y + 1.f, s[1].y + 1.f, s[2].y + 1.f, s[3].y + 1.f);
    nlc_terrain_soft_setups.emplace(key, p);
    Msg("~ [nlc_terrain_wet] %s=%.2f,%.2f %s=%.2f,%.2f %s=%.2f,%.2f %s=%.2f,%.2f ( softness,puddles; -1 = shader default )", r, s[0].x, s[0].y, g,
        s[1].x, s[1].y, b, s[2].x, s[2].y, a, s[3].x, s[3].y);
    return p;
}
} // namespace

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlender_BmmD::CBlender_BmmD()
{
    description.CLS = B_BmmD;
    xr_strcpy(oT2_Name, "$null");

    description.version = 3;
    xr_strcpy(oR_Name, "detail\\detail_grnd_grass"); //"$null");
    xr_strcpy(oG_Name, "detail\\detail_grnd_asphalt"); //"$null");
    xr_strcpy(oB_Name, "detail\\detail_grnd_earth"); //"$null");
    xr_strcpy(oA_Name, "detail\\detail_grnd_yantar"); //"$null");
}

CBlender_BmmD::~CBlender_BmmD() {}

void CBlender_BmmD::Save(IWriter& fs)
{
    IBlenderXr::Save(fs);
    xrPWRITE_MARKER(fs, "Detail map");
    xrPWRITE_PROP(fs, "Name", xrPID_TEXTURE, oT2_Name);
    string64 oT2_xform; // xform for secondary texture
    xrPWRITE_PROP(fs, "Transform", xrPID_MATRIX, oT2_xform);
    xrPWRITE_PROP(fs, "R2-R", xrPID_TEXTURE, oR_Name);
    xrPWRITE_PROP(fs, "R2-G", xrPID_TEXTURE, oG_Name);
    xrPWRITE_PROP(fs, "R2-B", xrPID_TEXTURE, oB_Name);
    xrPWRITE_PROP(fs, "R2-A", xrPID_TEXTURE, oA_Name);
}

void CBlender_BmmD::Load(IReader& fs, u16 version)
{
    IBlenderXr::Load(fs, version);
    string64 oT2_xform; // xform for secondary texture
    if (version < 3)
    {
        xrPREAD_MARKER(fs);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oT2_Name);
        xrPREAD_PROP(fs, xrPID_MATRIX, oT2_xform);
    }
    else
    {
        xrPREAD_MARKER(fs);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oT2_Name);
        xrPREAD_PROP(fs, xrPID_MATRIX, oT2_xform);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oR_Name);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oG_Name);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oB_Name);
        xrPREAD_PROP(fs, xrPID_TEXTURE, oA_Name);
    }
}

void CBlender_BmmD::SaveIni(CInifile* ini_file, LPCSTR section)
{
    IBlenderXr::SaveIni(ini_file, section);

    ini_file->w_string(section, "detail_name", oT2_Name);
    ini_file->w_string(section, "r2_r", oR_Name);
    ini_file->w_string(section, "r2_g", oG_Name);
    ini_file->w_string(section, "r2_b", oB_Name);
    ini_file->w_string(section, "r2_a", oA_Name);
}

void CBlender_BmmD::LoadIni(CInifile* ini_file, LPCSTR section)
{
    IBlenderXr::LoadIni(ini_file, section);

    strcpy_s(oT2_Name, ini_file->r_string(section, "detail_name"));
    strcpy_s(oR_Name, ini_file->r_string(section, "r2_r"));
    strcpy_s(oG_Name, ini_file->r_string(section, "r2_g"));
    strcpy_s(oB_Name, ini_file->r_string(section, "r2_b"));
    strcpy_s(oA_Name, ini_file->r_string(section, "r2_a"));
}

//////////////////////////////////////////////////////////////////////////
// R3
//////////////////////////////////////////////////////////////////////////
void CBlender_BmmD::Compile(CBlender_Compile& C)
{
    if (!ps_r2_ls_flags_ext.test(R2FLAGEXT_TERRAIN_PARALLAX))
    {
        CompileOLD(C);
        return;
    }

    IBlender::Compile(C);

    switch (C.iElement)
    {
    case SE_R2_NORMAL_HQ: // deffer
        string256 mask;
        strconcat(sizeof(mask), mask, C.L_textures[0].c_str(), "_mask");

        uber_deffer(C, true, "terrain", "terrain_high", false, oT2_Name[0] ? oT2_Name : 0);

        C.r_dx10Texture("s_mask", mask);
        C.r_dx10Texture("s_dt_r", oR_Name);
        C.r_dx10Texture("s_dt_g", oG_Name);
        C.r_dx10Texture("s_dt_b", oB_Name);
        C.r_dx10Texture("s_dt_a", oA_Name);
        C.r_dx10Texture("s_dn_r", strconcat(sizeof(mask), mask, oR_Name, "_bump"));
        C.r_dx10Texture("s_dn_g", strconcat(sizeof(mask), mask, oG_Name, "_bump"));
        C.r_dx10Texture("s_dn_b", strconcat(sizeof(mask), mask, oB_Name, "_bump"));
        C.r_dx10Texture("s_dn_a", strconcat(sizeof(mask), mask, oA_Name, "_bump"));
        C.r_dx10Texture("s_puddles_mask", strconcat(sizeof(mask), mask, C.L_textures[0].c_str(), "_puddles_mask"));
        C.r_dx10Texture("s_puddles_perlin", "fx\\puddles_perlin");
        C.r_dx10Texture("s_puddles_normal", "fx\\water_normal");
        C.r_dx10Texture("s_rainsplash", "fx\\water_sbumpvolume");
        {
            const auto wet = nlc_terrain_soft_get(oR_Name, oG_Name, oB_Name, oA_Name); // NLC .212f / .213b
            C.r_Constant("ssfx_terrain_soft", wet.soft);
            C.r_Constant("ssfx_terrain_puddle", wet.puddles);
        }

        C.r_dx10Sampler("smp_base");
        C.r_dx10Sampler("smp_linear");

        C.r_Stencil(TRUE, D3DCMP_ALWAYS, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        C.r_StencilRef(0x01);
        C.r_End();
        break;
    case SE_R2_NORMAL_LQ: // deffer
        uber_deffer(C, false, "base", "impl", false, oT2_Name[0] ? oT2_Name : nullptr, true);
        C.r_dx10Texture("s_lmap", C.L_textures[1]);
        C.r_dx10Sampler("smp_linear");

        C.r_Stencil(TRUE, D3DCMP_ALWAYS, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        C.r_StencilRef(0x01);

        C.r_End();
        break;
    case SE_R2_SHADOW: // smap
    case SE_R2_SHADOW_LIGHTS:
        C.r_Pass("shadow_direct_terrain", "shadow_direct_base", FALSE, TRUE, TRUE, FALSE);
        C.r_dx10Texture("s_base", C.L_textures[0]);
        C.r_dx10Sampler("smp_base");
        C.r_dx10Sampler("smp_linear");
        C.r_ColorWriteEnable(false, false, false, false);
        if (ps_r2_ls_flags.test(C.iElement == SE_R2_SHADOW_LIGHTS ? R2FLAG_SMAP_LIGHTS_2SIDE : R2FLAG_SMAP_2SIDE))
            C.r_CullMode(D3DCULL_NONE);
        C.r_End();
        break;
    }
}

void CBlender_BmmD::CompileOLD(CBlender_Compile& C)
{
    IBlender::Compile(C);
    // codepath is the same, only the shaders differ
    // ***only pixel shaders differ***
    string256 mask;
    strconcat(sizeof(mask), mask, C.L_textures[0].c_str(), "_mask");
    switch (C.iElement)
    {
    case SE_R2_NORMAL_HQ: // deffer
        uber_deffer(C, true, "impl", "impl", false, oT2_Name[0] ? oT2_Name : nullptr, true);

        C.r_dx10Texture("s_mask", mask);
        C.r_dx10Texture("s_lmap", C.L_textures[1]);

        C.r_dx10Texture("s_dt_r", oR_Name);
        C.r_dx10Texture("s_dt_g", oG_Name);
        C.r_dx10Texture("s_dt_b", oB_Name);
        C.r_dx10Texture("s_dt_a", oA_Name);

        C.r_dx10Texture("s_dn_r", strconcat(sizeof(mask), mask, oR_Name, "_bump"));
        C.r_dx10Texture("s_dn_g", strconcat(sizeof(mask), mask, oG_Name, "_bump"));
        C.r_dx10Texture("s_dn_b", strconcat(sizeof(mask), mask, oB_Name, "_bump"));
        C.r_dx10Texture("s_dn_a", strconcat(sizeof(mask), mask, oA_Name, "_bump"));
        /*
        C.r_dx10Texture("s_dnE_r", strconcat(sizeof(mask), mask, oR_Name, "_bump#"));
        C.r_dx10Texture("s_dnE_g", strconcat(sizeof(mask), mask, oG_Name, "_bump#"));
        C.r_dx10Texture("s_dnE_b", strconcat(sizeof(mask), mask, oB_Name, "_bump#"));
        C.r_dx10Texture("s_dnE_a", strconcat(sizeof(mask), mask, oA_Name, "_bump#"));
        */
        C.r_dx10Texture("s_puddles_normal", "fx\\water_normal");
        C.r_dx10Texture("s_puddles_perlin", "fx\\puddles_perlin");
        C.r_dx10Texture("s_puddles_mask", strconcat(sizeof(mask), mask, C.L_textures[0].c_str(), "_puddles_mask"));
        C.r_dx10Texture("s_rainsplash", "fx\\water_sbumpvolume");

        C.r_dx10Sampler("smp_base");
        C.r_dx10Sampler("smp_linear");

        C.r_Stencil(TRUE, D3DCMP_ALWAYS, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        C.r_StencilRef(0x01);

        C.r_End();
        break;
    case SE_R2_NORMAL_LQ: // deffer
        uber_deffer(C, false, "base", "impl", false, oT2_Name[0] ? oT2_Name : nullptr, true);
        C.r_dx10Texture("s_lmap", C.L_textures[1]);
        C.r_dx10Sampler("smp_linear");

        C.r_Stencil(TRUE, D3DCMP_ALWAYS, 0xff, 0x7f, D3DSTENCILOP_KEEP, D3DSTENCILOP_REPLACE, D3DSTENCILOP_KEEP);
        C.r_StencilRef(0x01);

        C.r_End();
        break;
    case SE_R2_SHADOW: // smap
    case SE_R2_SHADOW_LIGHTS:
        C.r_Pass("shadow_direct_base", "dumb", FALSE, TRUE, TRUE, FALSE);
        C.r_dx10Texture("s_base", C.L_textures[0]);
        C.r_dx10Sampler("smp_base");
        C.r_dx10Sampler("smp_linear");
        C.r_ColorWriteEnable(false, false, false, false);
        if (ps_r2_ls_flags.test(C.iElement == SE_R2_SHADOW_LIGHTS ? R2FLAG_SMAP_LIGHTS_2SIDE : R2FLAG_SMAP_2SIDE))
            C.r_CullMode(D3DCULL_NONE);
        C.r_End();
        break;
    }
}
