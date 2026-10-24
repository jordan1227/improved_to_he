#include "stdafx.h"
#include "igame_level.h"

#include "xr_area.h"
#include "xr_object.h"
#include "xrLevel.h"
#include "feel_sound.h"
#include "x_ray.h"
#include "GameFont.h"
#include "GameMtlLib.h"

using namespace collide;

void nlc_snd_occlusion_load();

extern BOOL g_bLoaded;

void IGame_Level::SoundEvent_Register(ref_sound_data_ptr S, float range)
{
    if (!g_bLoaded)
        return;
    if (!S)
        return;
    if (S->g_object && S->g_object->getDestroy())
    {
        S->g_object = nullptr;
        return;
    }
    if (nullptr == S->feedback)
        return;

    ZoneScoped;

    clamp(range, 0.1f, 500.f);

    const CSound_params* p = S->feedback->get_params();
    Fvector snd_position = p->position;
    if (S->feedback->is_2D())
    {
        snd_position.add(Sound->listener_position());
    }

    VERIFY(p && _valid(range));
    range = _min(range, p->max_ai_distance);
    VERIFY(_valid(snd_position));
    VERIFY(_valid(p->max_ai_distance));
    VERIFY(_valid(p->volume));

    // Query objects
    Fvector bb_size = {range, range, range};
    g_SpatialSpace->q_box(snd_ER, 0, STYPE_REACTTOSOUND, snd_position, bb_size);

    // Iterate
    xr_vector<ISpatial*>::iterator it = snd_ER.begin();
    xr_vector<ISpatial*>::iterator end = snd_ER.end();
    for (; it != end; ++it)
    {
        Feel::Sound* L = (*it)->dcast_FeelSound();
        if (nullptr == L)
            continue;
        CObject* CO = (*it)->dcast_CObject();
        VERIFY(CO);
        if (CO->getDestroy())
            continue;

        // Energy and signal
        VERIFY(_valid((*it)->spatial.sphere.P));
        float dist = snd_position.distance_to((*it)->spatial.sphere.P);
        if (dist > p->max_ai_distance)
            continue;
        VERIFY(_valid(dist));
        VERIFY(!fis_zero(p->max_ai_distance), S->handle->file_name());
        float Power = (1.f - dist / p->max_ai_distance) * p->volume;
        VERIFY(_valid(Power));
        if (Power > EPS_S)
        {
            const float occ = Sound->get_occlusion_to((*it)->spatial.sphere.P, snd_position);
            VERIFY(_valid(occ));
            Power *= occ;
            if (Power > EPS_S)
            {
                _esound_delegate D = {L, S, Power};
                snd_Events.push_back(D);
            }
        }
    }
    snd_ER.clear();
}

void IGame_Level::SoundEvent_Dispatch()
{
    ZoneScoped;

    while (!snd_Events.empty())
    {
        _esound_delegate& D = snd_Events.back();
        VERIFY(D.dest && D.source);
        if (D.source->feedback && D.source->g_object)
        {
            D.dest->feel_sound_new(D.source->g_object, D.source->g_type, D.source->g_userdata,
                                   D.source->feedback->is_2D() ? Device.vCameraPosition : D.source->feedback->get_params()->position, D.power);
        }
        snd_Events.pop_back();
    }
}

// Lain: added
void IGame_Level::SoundEvent_OnDestDestroy(Feel::Sound* obj)
{
    snd_Events.erase(std::remove_if(snd_Events.begin(), snd_Events.end(), [obj](const _esound_delegate& d) { return d.dest == obj; }), snd_Events.end());
}

void _sound_event(ref_sound_data_ptr S, float range)
{
    if (g_pGameLevel && S && S->feedback)
        g_pGameLevel->SoundEvent_Register(S, range);
}

//----------------------------------------------------------------------
// Class	: CObjectSpace
// Purpose	: stores space slots
//----------------------------------------------------------------------
CObjectSpace::CObjectSpace()
#ifdef PROFILE_CRITICAL_SECTIONS
    : Lock(MUTEX_PROFILE_ID(CObjectSpace::Lock))
#endif // PROFILE_CRITICAL_SECTIONS
{
#ifdef DEBUG
    sh_debug.create("debug\\wireframe", "$null");
#endif
    m_BoundingVolume.invalidate();
}
//----------------------------------------------------------------------
CObjectSpace::~CObjectSpace()
{
    Sound->set_geometry_occ(nullptr);
    Sound->set_geometry_som(nullptr);
    Sound->set_geometry_env(nullptr);
    Sound->set_handler(nullptr);
#ifdef DEBUG
    sh_debug.destroy();
#endif
}
//----------------------------------------------------------------------
int CObjectSpace::GetNearest(xr_vector<ISpatial*>& q_spatial, xr_vector<CObject*>& q_nearest, const Fvector& point, float range, CObject* ignore_object)
{
    ZoneScoped;

    q_spatial.clear();
    // Query objects
    q_nearest.clear();
    Fsphere Q;
    Q.set(point, range);
    Fvector B;
    B.set(range, range, range);
    g_SpatialSpace->q_box(q_spatial, 0, STYPE_COLLIDEABLE, point, B);

    // Iterate
    xr_vector<ISpatial*>::iterator it = q_spatial.begin();
    xr_vector<ISpatial*>::iterator end = q_spatial.end();
    for (; it != end; ++it)
    {
        CObject* O = (*it)->dcast_CObject();
        if (nullptr == O)
            continue;
        if (O == ignore_object)
            continue;
        Fsphere mS = {O->spatial.sphere.P, O->spatial.sphere.R};
        if (Q.intersect(mS))
            q_nearest.push_back(O);
    }

    return q_nearest.size();
}

//----------------------------------------------------------------------
int CObjectSpace::GetNearest(xr_vector<CObject*>& q_nearest, const Fvector& point, float range, CObject* ignore_object)
{
    xr_vector<ISpatial*> r_spatial;
    return (GetNearest(r_spatial, q_nearest, point, range, ignore_object));
}

//----------------------------------------------------------------------
int CObjectSpace::GetNearest(xr_vector<CObject*>& q_nearest, ICollisionForm* obj, float range)
{
    CObject* O = obj->Owner();
    return GetNearest(q_nearest, O->spatial.sphere.P, range + O->spatial.sphere.R, O);
}

//----------------------------------------------------------------------
static void build_callback(Fvector* V, const size_t Vcnt, CDB::TRI* T, const size_t Tcnt, void* params) { g_pGameLevel->Load_GameSpecific_CFORM(T, Tcnt); }

void CObjectSpace::Load()
{
    static IReader* F{};

    F = FS.r_open(fsgame::level, fsgame::level_files::level_cform);
    R_ASSERT(F);

    hdrCFORM H;
    F->r(&H, sizeof(hdrCFORM));

    R_ASSERT(CFORM_CURRENT_VERSION == H.version);

    std::thread{[H, this] {
        auto* verts = reinterpret_cast<const Fvector*>(F->pointer());
        auto* tris = reinterpret_cast<const CDB::TRI*>(verts + H.vertcount);

        Static.build(verts, H.vertcount, tris, H.facecount, build_callback);

        FS.r_close(F);
    }}.detach();

    m_BoundingVolume.set(H.aabb);
    g_SpatialSpace->initialize(H.aabb);
    g_SpatialSpacePhysic->initialize(H.aabb);

    Sound->set_geometry_occ(&Static);
    Sound->set_handler(_sound_event);
    nlc_snd_occlusion_load();
}

// NLC: sound occlusion and room-absorption table by game material (misc\snd_occlusion.ltx, docs/AUDIO_DESIGN.md 2.2),
// handed to xrSound at level load and by snd_occ_reload
static void nlc_parse_occ_mtl(LPCSTR v, SSndOccMtl& m, LPCSTR name)
{
    if (!v || !*v)
    {
        Msg("! [snd_occ] misc\\snd_occlusion.ltx: [%s] has no value, defaults kept", name);
        return;
    }
    if (strstr(v, "skip"))
    {
        m.skip = true;
        return;
    }
    float f[6] = {m.surface_db, m.db_per_m, m.hf_keep, m.hf_ref_m, m.default_thick, m.absorb};
    string256 buf;
    xr_strcpy(buf, v);
    for (char* c = buf; *c; ++c) // tolerate spaces around the commas
        if (*c == ',')
            *c = ' ';
    const int n = sscanf(buf, "%f %f %f %f %f %f", &f[0], &f[1], &f[2], &f[3], &f[4], &f[5]);
    if (n < 6)
        Msg("! [snd_occ] misc\\snd_occlusion.ltx: [%s] = [%s]: %d of 6 values read, the rest kept", name, v, n < 0 ? 0 : n);
    m.surface_db = _max(f[0], 0.f);
    m.db_per_m = _max(f[1], 0.f);
    m.hf_keep = clampr(f[2], 0.01f, 1.f);
    m.hf_ref_m = _max(f[3], 0.01f);
    m.default_thick = clampr(f[4], 0.f, 5.f);
    m.absorb = clampr(f[5], 0.f, 1.f);
    m.skip = false;
}

void nlc_snd_occlusion_load()
{
    if (!Sound)
        return;

    SSndOccConfig cfg;
    SSndOccMtl passable{1.f, 2.f, 0.85f, 1.f, 0.5f, 0.6f};
    xr_vector<xr_string> terrain_names;

    string_path fn;
    CInifile* ini = nullptr;
    constexpr LPCSTR S = "snd_occlusion";
    constexpr LPCSTR SM = "snd_occlusion_materials";
    constexpr LPCSTR SR = "snd_reverb";
    if (FS.exist(fn, "$game_config$", "misc\\snd_occlusion.ltx"))
        ini = xr_new<CInifile>(fn);
    else
        Msg("! [snd_occ] misc\\snd_occlusion.ltx missing, using built-in defaults");

    if (ini)
    {
        auto rf = [&](LPCSTR sect, LPCSTR key, float& v) {
            if (ini->line_exist(sect, key))
                v = ini->r_float(sect, key);
        };
        rf(S, "strength", cfg.strength);
        rf(S, "max_db", cfg.max_db);
        rf(S, "min_thick", cfg.min_thick);
        rf(S, "max_thick", cfg.max_thick);
        rf(S, "leak_db", cfg.leak_db);
        rf(S, "leak_hf", cfg.leak_hf);
        rf(S, "leak_side", cfg.leak_side);
        rf(S, "leak_up", cfg.leak_up);
        rf(S, "leak_max_dist", cfg.leak_max_dist);
        rf(S, "attack", cfg.attack);
        rf(S, "release", cfg.release);
        rf(S, "loud_cap", cfg.loud_cap);
        rf(S, "loud_slope", cfg.loud_slope);
        rf(S, "loud_max", cfg.loud_max);
        rf(S, "terrain_cap", cfg.terrain_cap);
        rf(S, "cap_hf_div", cfg.cap_hf_div);
        if (ini->line_exist(S, "default"))
            nlc_parse_occ_mtl(ini->r_string(S, "default"), cfg.def, "default");
        if (ini->line_exist(S, "passable"))
            nlc_parse_occ_mtl(ini->r_string(S, "passable"), passable, "passable");
        auto read_list = [&](LPCSTR key, xr_vector<xr_string>& out) {
            if (!ini->line_exist(S, key))
                return;
            LPCSTR v = ini->r_string(S, key);
            if (!v)
                return;
            string256 item;
            for (int i = 0, n = _GetItemCount(v); i < n; ++i)
            {
                _GetItem(v, i, item);
                _Trim(item);
                if (item[0])
                    out.emplace_back(item);
            }
        };
        read_list("terrain", terrain_names);
        read_list("no_occlude", cfg.no_occlude);
        read_list("spatial_stereo", cfg.spatial_stereo);

        rf(SR, "probe_len", cfg.probe_len);
        rf(SR, "enc_near", cfg.enc_near);
        rf(SR, "enc_far", cfg.enc_far);
        rf(SR, "roof_open_k", cfg.roof_open_k);
        rf(SR, "decay_k", cfg.decay_k);
        rf(SR, "decay_min", cfg.decay_min);
        rf(SR, "decay_max", cfg.decay_max);
        rf(SR, "room_gain", cfg.room_gain);
        rf(SR, "outdoor_gain", cfg.outdoor_gain);
        rf(SR, "send_mismatch", cfg.send_mismatch);
        rf(SR, "echo_gain", cfg.echo_gain);
        rf(SR, "amb_indoor_hf", cfg.amb_indoor_hf);
        rf(SR, "rain_indoor", cfg.rain_indoor);
        rf(SR, "hrtf_2d_gain", cfg.hrtf_2d_gain);
    }

    u32 listed = 0;
    cfg.mtl.resize(GMLib.CountMaterial());
    u32 idx = 0;
    for (GameMtlIt it = GMLib.FirstMaterial(); it != GMLib.LastMaterial(); ++it, ++idx)
    {
        const SGameMtl* M = *it;
        SSndOccMtl& m = cfg.mtl[idx];
        m = M->Flags.is(SGameMtl::flPassable) ? passable : cfg.def;
        if (M->Flags.is(SGameMtl::flLiquid))
            m.skip = true;
        if (ini && ini->line_exist(SM, M->m_Name.c_str()))
        {
            nlc_parse_occ_mtl(ini->r_string(SM, M->m_Name.c_str()), m, M->m_Name.c_str());
            ++listed;
        }
        for (const xr_string& t : terrain_names)
            if (!xr_strcmp(t.c_str(), M->m_Name.c_str()))
                m.terrain = true;
    }

    if (ini && ini->section_exist(SM))
    {
        for (const auto& item : ini->r_section(SM).Data)
            if (GMLib.GetMaterialIt(item.first.c_str()) == GMLib.LastMaterial())
                Msg("! [snd_occ] misc\\snd_occlusion.ltx: unknown material [%s]", item.first.c_str());
    }
    xr_delete(ini);

    Msg("* [snd_occ] %u game materials, %u listed in misc\\snd_occlusion.ltx, %u terrain, %u no_occlude", cfg.mtl.size(), listed, terrain_names.size(),
        cfg.no_occlude.size());
    Sound->set_occlusion_config(cfg);
}
//----------------------------------------------------------------------

void RayPickAsync::RayPickSubmit(const Fvector start, const Fvector dir, float range, collide::rq_target tgt, const CObject* ignore_object)
{
    this->start = start;
    this->dir = dir;
    this->range = range;
    this->tgt = tgt;
    this->ignore_object = ignore_object;

    future_ready = false;
    Device.add_to_seq_parallel(fastdelegate::MakeDelegate(this, &RayPickAsync::do_work_async));
}

bool RayPickAsync::Ready(collide::rq_result& R)
{
    if (result.valid())
        R = result;
    return result.valid();
}

void RayPickAsync::Discard()
{
    Device.remove_from_seq_parallel(fastdelegate::MakeDelegate(this, &RayPickAsync::do_work_async));
}

void RayPickAsync::do_work_async()
{
    if (!g_pGameLevel || g_pGameLevel->is_removing_objects())
        return;

    ZoneScoped;

    g_pGameLevel->ObjectSpace.RayPick(this->start, this->dir, this->range, this->tgt, result, this->ignore_object);

    future_ready = true;
}
