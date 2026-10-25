#include "stdafx.h"

#include <efx.h>

#include "../xrCDB/cl_intersect.h"
#include "SoundRender_Core.h"
#include "SoundRender_Emitter.h"
#include "SoundRender_Target.h"
#include "SoundRender_Source.h"

CSoundRender_Emitter* CSoundRender_Core::i_play(ref_sound* S, BOOL _loop, float delay, u32 flags)
{
    VERIFY(S->_p->feedback == 0);
    CSoundRender_Emitter* E = xr_new<CSoundRender_Emitter>();
    S->_p->feedback = E;
    E->start(S, _loop, delay, flags);
    s_emitters.push_back(E);
    return E;
}

void CSoundRender_Core::update(const Fvector& P, const Fvector& D, const Fvector& N) // called on seqFrameMT
{
    u32 it;

    if (0 == bReady)
        return;

    std::scoped_lock m{m_bLocked};

    bLocked = TRUE;
    Timer.time_factor(psSoundTimeFactor); //--#SM+#--
    float new_tm = Timer.GetElapsed_sec();
    fTimer_Delta = new_tm - fTimer_Value;
    float dt_sec = fTimer_Delta;
    fTimer_Value = new_tm;

    s_emitters_u++;

    // NLC: occlusion ray budget for this update (snd_occ_rays per 60 fps frame, so the rate per second does not depend on
    // the frame rate), room probe (AUDIO_DESIGN 2.3, 3.1, 13.2)
    nlc_rays_left = iFloor(float(psSoundOccRays) * clampr(dt_sec * 60.f, 0.5f, 2.f));
    nlc_rays_used = 0;
    nlc_due.clear();
    if (efx_next_commit > fTimer_Value + 0.05f) // the timer jumped back (load, pause): do not wait for it to catch up
        efx_next_commit = 0.f;
    {
        // NLC listener filter: linear fade toward the script's target
        std::scoped_lock l{nlc_lf_lock};
        const float step = clampr(dt_sec, 0.f, 0.5f) * nlc_lf_rate;
        auto move = [step](float& c, float t) { c = c < t ? _min(c + step, t) : _max(c - step, t); };
        move(nlc_lf_gain, psSoundMaskFilter ? nlc_lf_gain_t : 1.f);
        move(nlc_lf_hf, psSoundMaskFilter ? nlc_lf_hf_t : 1.f);
    }
    if (psSoundOccMode == 1)
        nlc_probe_update(dt_sec);

    if (nlc_dist_model_applied != psSoundDistModel)
    {
        nlc_dist_model_applied = psSoundDistModel;
        alDistanceModel(psSoundDistModel == 1 ? AL_NONE : AL_INVERSE_DISTANCE_CLAMPED);
    }

    // Firstly update emitters, which are now being rendered
    // Msg	("! update: r-emitters");
    for (it = 0; it < s_targets.size(); it++)
    {
        CSoundRender_Target* T = s_targets[it];
        CSoundRender_Emitter* E = T->get_emitter();
        if (E)
        {
            E->update(dt_sec);
            E->marker = s_emitters_u;
            E = T->get_emitter(); // update can stop itself
            if (E)
                T->priority = E->priority();
            else
                T->priority = -1;
        }
        else
        {
            T->priority = -1;
        }
    }

    // Update emmitters
    // Msg	("! update: emitters");
    for (it = 0; it < s_emitters.size(); it++)
    {
        CSoundRender_Emitter* pEmitter = s_emitters[it];
        if (pEmitter->marker != s_emitters_u)
        {
            pEmitter->update(dt_sec);
            pEmitter->marker = s_emitters_u;
        }
        if (!pEmitter->isPlaying())
        {
            // Stopped
            nlc_unqueue(pEmitter);
            xr_delete(pEmitter);
            s_emitters.erase(s_emitters.begin() + it);
            it--;
        }
    }

    // NLC: occlusion for the emitters that came due this update, by priority within the budget
    nlc_process_due();

    // Get currently rendering emitters
    // Msg	("! update: targets");
    s_targets_defer.clear();
    s_targets_pu++;
    // u32 PU				= s_targets_pu%s_targets.size();
    for (it = 0; it < s_targets.size(); it++)
    {
        CSoundRender_Target* T = s_targets[it];
        if (T->get_emitter())
        {
            // Has emmitter, maybe just not started rendering
            if (T->get_Rendering())
            {
                T->fill_parameters(this);
                T->update();
            }
            else
                s_targets_defer.push_back(T);
        }
    }

    // Commit parameters from pending targets
    if (!s_targets_defer.empty())
    {
        // Msg	("! update: start render - commit");
        s_targets_defer.erase(std::unique(s_targets_defer.begin(), s_targets_defer.end()), s_targets_defer.end());
        for (it = 0; it < s_targets_defer.size(); it++)
            s_targets_defer[it]->fill_parameters(this);
    }

    // update EAX or EFX
    if (psSoundFlags.test(ss_EAX) && (bEAX || bEFX))
    {
        static shared_str curr_env;

        if (bListenerMoved)
        {
            bListenerMoved = FALSE;
            e_target = get_environment(P);

            if (!curr_env.size() || curr_env != e_target->name)
            {
                curr_env = e_target->name;
                MsgDbg("~ current environment sound zone name [%s]", curr_env.c_str());
            }
        }

        if (!e_currentPaused)
            e_current.lerp(e_current, *e_target, dt_sec);
        else
            e_current.set_from(*e_target);

        if (bEAX)
        {
            i_eax_listener_set(&e_current);
            i_eax_commit_setting();
        }
        else if (bEFX && fTimer_Value >= efx_next_commit)
        {
            // NLC: probe room + authored zone, committed at most 20 times a second (AUDIO_DESIGN 3.1)
            efx_next_commit = fTimer_Value + 0.05f;

            EFXEAXREVERBPROPERTIES fin, zone;
            env_to_efx(e_current, zone);
            const bool zone_valid = geom_ENV && e_target != get_environment_def() && xr_strcmp(e_target->name, "identity") != 0;
            if (psSoundReverbMode == 1 && psSoundOccMode == 1)
            {
                // near-dry open air (a short, dark, quiet tail); long outdoor tails come from the echo slot on loud sounds
                EFXEAXREVERBPROPERTIES room, out = EFX_REVERB_PRESET_GENERIC;
                out.flDiffusion = 0.5f;
                out.flGain = 0.32f * nlc_cfg.outdoor_gain;
                out.flGainHF = 0.3f;
                out.flDecayTime = 0.9f;
                out.flDecayHFRatio = 0.5f;
                out.flReflectionsGain = 0.05f;
                out.flReflectionsDelay = 0.02f;
                out.flLateReverbGain = 0.1f;
                out.flLateReverbDelay = 0.03f;
                nlc_room_props(room);
                const float E = nlc_enc;
                auto mix = [](EFXEAXREVERBPROPERTIES& R, const EFXEAXREVERBPROPERTIES& A, const EFXEAXREVERBPROPERTIES& B, float t) {
                    const float* a = &A.flDensity;
                    const float* b = &B.flDensity;
                    float* r = &R.flDensity;
                    constexpr int nf = offsetof(EFXEAXREVERBPROPERTIES, iDecayHFLimit) / sizeof(float);
                    for (int i = 0; i < nf; ++i)
                        r[i] = a[i] + (b[i] - a[i]) * t;
                    R.iDecayHFLimit = t < 0.5f ? A.iDecayHFLimit : B.iDecayHFLimit;
                };
                mix(fin, out, room, E);
                if (zone_valid)
                    mix(fin, fin, zone, psSoundReverbZoneWeight);
            }
            else
                fin = zone;
            fin.flGain *= psSoundReverbGain;

            // skip the commit when nothing changed by more than 2 % (each commit re-runs the reverb update in OpenAL Soft)
            bool changed = !efx_last_valid || !efx_slots_attached;
            if (!changed)
            {
                const float* a = &fin.flDensity;
                const float* b = &efx_last.flDensity;
                constexpr int nf = offsetof(EFXEAXREVERBPROPERTIES, iDecayHFLimit) / sizeof(float);
                for (int i = 0; i < nf && !changed; ++i)
                    changed = _abs(a[i] - b[i]) > 0.02f * _max(_abs(b[i]), 0.01f);
            }
            efx_attach_slots(true);
            if (changed)
            {
                efx_apply(effect, fin);
                efx_last = fin;
                efx_last_valid = true;
                bEFX = i_efx_commit_setting();
            }

            if (psSoundReverbLog && fTimer_Value >= nlc_rv_log_time)
            {
                nlc_rv_log_time = fTimer_Value + 0.5f;
                Msg("[snd_rv] E=%.2f L=%.1f R=%.1f a=%.2f zone=%s(%s) decay=%.2f gain=%.3f refl=%.3f/%.3f late=%.3f", nlc_enc, nlc_mean_dist, nlc_room_size, nlc_absorb,
                    e_target->name.c_str(), zone_valid ? "on" : "off", fin.flDecayTime, fin.flGain, fin.flReflectionsGain, fin.flReflectionsDelay,
                    fin.flLateReverbGain);
            }

            if (!bEFX)
            {
                for (u32 it = 0; it < s_targets.size(); it++)
                {
                    CSoundRender_Target* T = s_targets[it];

                    T->alAuxInit(AL_EFFECTSLOT_NULL);

                    T->bEFX = false;
                }

                release_efx_objects();
            }
        }
    }
    else if (bEFX)
        efx_attach_slots(false);

    // update listener
    update_listener(P, D, N, dt_sec);

    // Start rendering of pending targets
    if (!s_targets_defer.empty())
    {
        // Msg	("! update: start render");
        for (it = 0; it < s_targets_defer.size(); it++)
            s_targets_defer[it]->render();
    }

    // Events
    update_events();

    bLocked = FALSE;
}

static u32 g_saved_event_count = 0;
void CSoundRender_Core::update_events()
{
    g_saved_event_count = s_events.size();
    for (u32 it = 0; it < s_events.size(); it++)
    {
        event& E = s_events[it];
        Handler(E.first, E.second);
    }
    s_events.clear();
}

void CSoundRender_Core::statistic(CSound_stats* dest, CSound_stats_ext* ext)
{
    // NLC: under the update lock - the sound task deletes finished emitters (snd_stats crashed in CStats::OnRender
    // during a big fight, nlc-3.589.148)
    std::scoped_lock m{m_bLocked};
    if (dest)
    {
        dest->_rendered = 0;
        for (u32 it = 0; it < s_targets.size(); it++)
        {
            CSoundRender_Target* T = s_targets[it];
            if (T->get_emitter() && T->get_Rendering())
                dest->_rendered++;
        }
        dest->_simulated = s_emitters.size();
        dest->_cache_hits = cache._stat_hit;
        dest->_cache_misses = cache._stat_miss;
        dest->_events = g_saved_event_count;
        cache.stats_clear();
    }
    if (ext)
    {
        for (u32 it = 0; it < s_emitters.size(); it++)
        {
            CSoundRender_Emitter* _E = s_emitters[it];
            CSound_stats_ext::SItem _I;
            _I._3D = !_E->b2D;
            _I._rendered = !!_E->target;
            _I.params = _E->p_source;
            _I.volume = _E->smooth_volume;
            _I.occ_db = _E->nlc_occ_db;
            _I.occ_hf = _E->nlc_occ_hf;
            if (_E->owner_data)
            {
                _I.name = _E->source()->fname;
                _I.game_object = _E->owner_data->g_object;
                _I.game_type = _E->owner_data->g_type;
                _I.type = _E->owner_data->s_type;
            }
            else
            {
                _I.game_object = 0;
                _I.game_type = 0;
                _I.type = st_Effect;
            }
            ext->append(_I);
        }
    }
}

float CSoundRender_Core::get_occlusion_to(const Fvector& hear_pt, const Fvector& snd_pt, float dispersion)
{
    float occ_value = 1.f;

    if (0 != geom_SOM)
    {
        // Calculate RAY params
        Fvector pos, dir;
        pos.random_dir();
        pos.mul(dispersion);
        pos.add(snd_pt);
        dir.sub(pos, hear_pt);
        float range = dir.magnitude();
        dir.div(range);

        geom_DB.ray_query(CDB::OPT_CULL, geom_SOM, hear_pt, dir, range);
        u32 r_cnt = u32(geom_DB.r_count());
        CDB::RESULT* _B = geom_DB.r_begin();

        if (0 != r_cnt)
        {
            for (u32 k = 0; k < r_cnt; k++)
            {
                CDB::RESULT* R = _B + k;
                occ_value *= *(float*)&R->dummy;
            }
        }
    }
    return occ_value;
}

float CSoundRender_Core::get_occlusion(Fvector& P, float R, Fvector* occ)
{
    float occ_value = 1.f;

    // Calculate RAY params
    Fvector base = listener_position();
    Fvector pos, dir;
    float range;
    pos.random_dir();
    pos.mul(R);
    pos.add(P);
    dir.sub(pos, base);
    range = dir.magnitude();
    dir.div(range);

    if (0 != geom_MODEL)
    {
        bool bNeedFullTest = true;
        // 1. Check cached polygon
        float _u, _v, _range;
        if (CDB::TestRayTri(base, dir, occ, _u, _v, _range, true))
            if (_range > 0 && _range < range)
            {
                occ_value = psSoundOcclusionScale;
                bNeedFullTest = false;
            }
        // 2. Polygon doesn't picked up - real database query
        if (bNeedFullTest)
        {
            geom_DB.ray_query(CDB::OPT_ONLYNEAREST, geom_MODEL, base, dir, range);
            if (0 != geom_DB.r_count())
            {
                // cache polygon
                const CDB::RESULT* R = geom_DB.r_begin();
                const CDB::TRI& T = geom_MODEL->get_tris()[R->id];
                const Fvector* V = geom_MODEL->get_verts();
                occ[0].set(V[T.verts[0]]);
                occ[1].set(V[T.verts[1]]);
                occ[2].set(V[T.verts[2]]);
                occ_value = psSoundOcclusionScale;
            }
        }
    }
    if (0 != geom_SOM)
    {
        geom_DB.ray_query(CDB::OPT_CULL, geom_SOM, base, dir, range);
        u32 r_cnt = u32(geom_DB.r_count());
        CDB::RESULT* _B = geom_DB.r_begin();

        if (0 != r_cnt)
        {
            for (u32 k = 0; k < r_cnt; k++)
            {
                CDB::RESULT* R = _B + k;
                occ_value *= *(float*)&R->dummy;
            }
        }
    }
    return occ_value;
}

//////////////////////////////////////////////////
// NLC: occlusion and room probe (docs/AUDIO_DESIGN.md sections 2, 3 and 13). Runs on the sound update task.

// loss in dB along one segment: an entry face paired with the next exit face gives the thickness,
// unpaired faces (open meshes, one-sided props) take the material default
float CSoundRender_Core::nlc_path(const Fvector& A, const Fvector& B, float& hf, u32& occluders, bool& terrain_only)
{
    hf = 1.f;
    occluders = 0;
    terrain_only = false;
    const CDB::MODEL* model = geom_MODEL;
    if (!model)
        return 0.f;

    Fvector dir;
    dir.sub(B, A);
    const float range = dir.magnitude();
    if (range < 0.6f)
        return 0.f;
    dir.div(range);

    --nlc_rays_left;
    ++nlc_rays_used;
    nlc_DB.ray_query(0, model, A, dir, range);

    nlc_hits.clear();
    const CDB::TRI* tris = model->get_tris();
    const Fvector* verts = model->get_verts();
    for (const CDB::RESULT* R = nlc_DB.r_begin(); R != nlc_DB.r_end(); ++R)
    {
        if (R->range < 0.3f || R->range > range - 0.3f)
            continue;
        const CDB::TRI& T = tris[R->id];
        if (nlc_mtl(T.material).skip)
            continue;
        Fvector n;
        n.mknormal(verts[T.verts[0]], verts[T.verts[1]], verts[T.verts[2]]);
        nlc_hits.push_back({R->range, u16(T.material), n.dotproduct(dir) < 0.f});
    }
    if (nlc_hits.empty())
        return 0.f;

    // by range; within 5 mm an entry sorts first, so a two-sided plane (coplanar entry + exit) pairs with itself
    std::sort(nlc_hits.begin(), nlc_hits.end(), [](const nlc_hit& a, const nlc_hit& b) { return a.range < b.range; });
    for (size_t i = 1; i < nlc_hits.size(); ++i)
        if (nlc_hits[i].enter && !nlc_hits[i - 1].enter && nlc_hits[i].range - nlc_hits[i - 1].range < 0.005f)
            std::swap(nlc_hits[i], nlc_hits[i - 1]);

    const SSndOccConfig& C = nlc_cfg;
    float db = 0.f;
    bool all_terrain = true;
    auto add = [&](const SSndOccMtl& m, float thick) {
        db += m.surface_db + m.db_per_m * thick;
        hf *= powf(m.hf_keep, 1.f + thick / _max(m.hf_ref_m, 0.01f));
        all_terrain = all_terrain && m.terrain;
        ++occluders;
    };

    const nlc_hit* open = nullptr;
    for (const nlc_hit& h : nlc_hits)
    {
        if (db >= C.max_db)
            break;
        if (h.enter)
        {
            if (open)
                add(nlc_mtl(open->mtl), nlc_mtl(open->mtl).default_thick);
            open = &h;
        }
        else if (open)
        {
            const SSndOccMtl& m = nlc_mtl(open->mtl);
            const float t = h.range - open->range;
            if (t < 0.02f)
                add(m, m.default_thick); // a plane: real thickness unknown
            else if (t > C.max_thick)
            {
                // too far apart to be one wall (one-sided geometry): two separate surfaces
                add(m, m.default_thick);
                add(nlc_mtl(h.mtl), nlc_mtl(h.mtl).default_thick);
            }
            else
                add(m, _max(t, C.min_thick));
            open = nullptr;
        }
        else
            add(nlc_mtl(h.mtl), nlc_mtl(h.mtl).default_thick);
    }
    if (open && db < C.max_db)
    {
        // the segment ends inside a solid: the source sits in (or just under) it
        const SSndOccMtl& m = nlc_mtl(open->mtl);
        add(m, _min(m.default_thick, _max(range - open->range, C.min_thick)));
    }
    terrain_only = occluders > 0 && all_terrain;
    return _min(db, C.max_db);
}

void CSoundRender_Core::nlc_cache_clear()
{
    for (nlc_occ_cache_t& c : nlc_occ_cache)
        c.valid = false;
    nlc_enc_cache.clear();
    nlc_cache_model = geom_MODEL;
}

bool CSoundRender_Core::nlc_spatial_stereo(LPCSTR name) const
{
    for (const xr_string& s : nlc_cfg.spatial_stereo)
        if (!s.empty() && strstr(name, s.c_str()))
            return true;
    return false;
}

bool CSoundRender_Core::nlc_no_occlude(LPCSTR name) const
{
    for (const xr_string& s : nlc_cfg.no_occlude)
        if (!s.empty() && strstr(name, s.c_str()))
            return true;
    return false;
}

// direct path plus leak paths around the occluder (source sides, above the source, listener sides); the least lossy wins.
// The raw result is cached per listener/source cell; caps for loud sounds and hills, and the strength, are applied after.
void CSoundRender_Core::nlc_occlusion(const Fvector& P, float dist, bool loud, bool leaks, float& gain, float& hf, float& db, u8& path, LPCSTR log_name)
{
    const SSndOccConfig& C = nlc_cfg;
    gain = 1.f;
    hf = 1.f;
    db = 0.f;
    path = 0;
    if (!geom_MODEL || dist < 1.f)
        return;
    if (nlc_cache_model != geom_MODEL)
        nlc_cache_clear();

    const Fvector L = listener_position();
    Fvector S = P;
    S.y += 0.25f; // keep sources on the ground from grazing it

    // cache lookup (a leak-less result does not serve a request with leaks)
    s32 lc[3] = {iFloor(L.x * 2.f), iFloor(L.y * 2.f), iFloor(L.z * 2.f)};
    s32 sc[3] = {iFloor(S.x * 2.f), iFloor(S.y * 2.f), iFloor(S.z * 2.f)};
    const u32 h = u32(lc[0] * 73856093 ^ lc[1] * 19349663 ^ lc[2] * 83492791 ^ sc[0] * 2654435761u ^ sc[1] * 40503 ^ sc[2] * 97);
    nlc_occ_cache_t& slot = nlc_occ_cache[h % nlc_occ_cache_size];
    const bool hit = slot.valid && fTimer_Value - slot.time < 0.3f && (slot.leaks || !leaks) && !memcmp(slot.l, lc, sizeof(lc)) && !memcmp(slot.s, sc, sizeof(sc));

    float best, best_hf = 1.f, direct = 0.f;
    bool terrain = false;
    u32 direct_n = 0;
    if (hit)
    {
        ++nlc_cache_hits;
        best = slot.db;
        best_hf = slot.hf;
        path = slot.path;
        terrain = slot.terrain;
        direct = best;
    }
    else
    {
        best = nlc_path(L, S, best_hf, direct_n, terrain);
        direct = best;
        bool leaks_done = !leaks;
        if (leaks && best > C.leak_db + 0.5f && dist < C.leak_max_dist)
        {
            leaks_done = true;
            Fvector dir;
            dir.sub(S, L).normalize_safe();
            Fvector side;
            side.crossproduct(dir, Fvector{0.f, 1.f, 0.f});
            if (side.magnitude() < 0.1f)
                side.set(1.f, 0.f, 0.f);
            else
                side.normalize();
            const float off = C.leak_side * clampr(dist / 4.f, 0.25f, 1.f);
            const float off_l = _min(off, 0.8f);

            Fvector from[5], to[5];
            from[0] = L;
            to[0].mad(S, side, off);
            from[1] = L;
            to[1].mad(S, side, -off);
            from[2] = L;
            to[2].set(S.x, S.y + C.leak_up, S.z);
            from[3].mad(L, side, off_l);
            to[3] = S;
            from[4].mad(L, side, -off_l);
            to[4] = S;

            // loud sounds always get their leak paths; others only within the budget
            for (u32 i = 0; i < 5 && (loud || nlc_rays_left > 0) && best > C.leak_db + 0.5f; ++i)
            {
                float phf;
                u32 pn;
                bool pt;
                const float pdb = nlc_path(from[i], to[i], phf, pn, pt) + C.leak_db;
                if (pdb < best)
                {
                    best = pdb;
                    best_hf = phf * C.leak_hf;
                    path = u8(i + 1);
                    terrain = pt;
                }
            }
        }
        slot.valid = true;
        memcpy(slot.l, lc, sizeof(lc));
        memcpy(slot.s, sc, sizeof(sc));
        slot.time = fTimer_Value;
        slot.db = best;
        slot.hf = best_hf;
        slot.path = path;
        slot.terrain = terrain;
        slot.leaks = leaks_done;
    }

    // caps: loud sounds stay audible through buildings (soft cap), a hill alone loses at most terrain_cap;
    // the capped-away dB become extra high-frequency loss (AUDIO_DESIGN 13.2 item 2)
    float raw = _min(best, C.max_db), soft = raw;
    if (loud && raw > C.loud_cap)
        soft = _min(C.loud_cap + C.loud_slope * (raw - C.loud_cap), C.loud_max);
    if (terrain)
        soft = _min(soft, C.terrain_cap);
    if (soft < raw)
        best_hf = _max(best_hf * powf(10.f, -(raw - soft) / _max(C.cap_hf_div, 1.f)), _min(best_hf, 0.05f));

    const float strength = C.strength * psSoundOccStrength;
    db = soft * strength;
    gain = powf(10.f, -db / 20.f);
    hf = clampr(powf(best_hf, strength), 0.02f, 1.f);

    // authored level.som occluders, as before
    if (geom_SOM)
    {
        Fvector dir;
        dir.sub(S, L);
        const float range = dir.magnitude();
        if (range > EPS_L)
        {
            dir.div(range);
            nlc_DB.ray_query(CDB::OPT_CULL, geom_SOM, L, dir, range);
            for (const CDB::RESULT* R = nlc_DB.r_begin(); R != nlc_DB.r_end(); ++R)
                gain *= *(float*)&R->dummy;
        }
    }

    if (log_name)
        Msg("[snd_occ] %s d=%.1f direct=%.1fdB(%u) best=%.1fdB path=%u%s%s%s -> -%.1fdB gain=%.3f hf=%.2f rays=%u", log_name, dist, direct, direct_n, best,
            path, hit ? " cached" : "", loud ? " loud" : "", terrain ? " terrain" : "", db, gain, hf, nlc_rays_used);
}

void CSoundRender_Core::nlc_unqueue(CSoundRender_Emitter* E)
{
    for (size_t i = 0; i < nlc_due.size(); ++i)
        if (nlc_due[i].E == E)
        {
            nlc_due[i] = nlc_due.back();
            nlc_due.pop_back();
            --i;
        }
}

// serve due emitters by priority (loudness x proximity x staleness, loud / not yet evaluated first) within the ray budget
void CSoundRender_Core::nlc_process_due()
{
    if (!nlc_due.empty())
    {
        std::sort(nlc_due.begin(), nlc_due.end(), [](const nlc_due_t& a, const nlc_due_t& b) { return a.key > b.key; });
        for (const nlc_due_t& d : nlc_due)
        {
            if (nlc_rays_left <= 0)
                break;
            d.E->nlc_occ_compute(false);
        }
    }
    nlc_due.clear();
}

// nearest non-skipped static hit
bool CSoundRender_Core::nlc_probe_ray(const Fvector& P, const Fvector& dir, float range, float& dist, float& absorb)
{
    const CDB::MODEL* model = geom_MODEL;
    if (!model)
        return false;
    --nlc_rays_left;
    ++nlc_rays_used;
    nlc_DB.ray_query(0, model, P, dir, range);
    float best = flt_max;
    u32 mtl = 0;
    for (const CDB::RESULT* R = nlc_DB.r_begin(); R != nlc_DB.r_end(); ++R)
    {
        if (R->range >= best || R->range < 0.05f)
            continue;
        const u32 m = model->get_tris()[R->id].material;
        if (nlc_mtl(m).skip)
            continue;
        best = R->range;
        mtl = m;
    }
    if (best == flt_max)
        return false;
    dist = best;
    absorb = nlc_mtl(mtl).absorb;
    return true;
}

// 0 = open air, 1 = under a roof with walls both sides (AUDIO_DESIGN 3.2); computed once per sound, cached per 3 m cell
float CSoundRender_Core::nlc_source_enclosure(const Fvector& P)
{
    if (!geom_MODEL)
        return 0.f;
    if (nlc_cache_model != geom_MODEL)
        nlc_cache_clear();

    const u64 key = (u64(u32(iFloor(P.x / 3.f)) & 0x1FFFFF) << 42) | (u64(u32(iFloor(P.y / 3.f)) & 0x1FFFFF) << 21) | u64(u32(iFloor(P.z / 3.f)) & 0x1FFFFF);
    const auto it = nlc_enc_cache.find(key);
    if (it != nlc_enc_cache.end())
    {
        ++nlc_cache_hits;
        return it->second;
    }

    Fvector S = P;
    S.y += 0.3f;
    float d, a;
    float e = nlc_probe_ray(S, Fvector{0.f, 1.f, 0.f}, 30.f, d, a) ? 0.6f : 0.f;
    // fixed world axes (not the listener direction) so one value per cell serves every listener
    if (nlc_probe_ray(S, Fvector{1.f, 0.f, 0.f}, 15.f, d, a) || nlc_probe_ray(S, Fvector{-1.f, 0.f, 0.f}, 15.f, d, a))
        e += 0.2f;
    if (nlc_probe_ray(S, Fvector{0.f, 0.f, 1.f}, 15.f, d, a) || nlc_probe_ray(S, Fvector{0.f, 0.f, -1.f}, 15.f, d, a))
        e += 0.2f;
    if (nlc_enc_cache.size() > 4096)
        nlc_enc_cache.clear();
    nlc_enc_cache[key] = e;
    return e;
}

// listener room probe: 24 directions (8 azimuths x -20/+10/+45 degrees) plus the sky, 3 rays at most 30 times a second
void CSoundRender_Core::nlc_probe_update(float dt)
{
    const SSndOccConfig& C = nlc_cfg;
    float E = 0.f, Lm = C.probe_len, Ln = C.probe_len, Lr = C.probe_len, Am = 0.1f;
    dt = clampr(dt, 0.f, 0.5f);

    if (!geom_MODEL)
        nlc_probe_valid = false;
    else
    {
        static Fvector dirs[nlc_probe_dirs];
        static bool dirs_ok = false;
        if (!dirs_ok)
        {
            dirs_ok = true;
            constexpr float el[3] = {-20.f, 10.f, 45.f};
            for (u32 az = 0; az < 8; ++az)
                for (u32 e = 0; e < 3; ++e)
                    dirs[az * 3 + e].setHP(deg2rad(az * 45.f + e * 15.f), deg2rad(el[e]));
            dirs[nlc_probe_dirs - 1].set(0.f, 1.f, 0.f);
        }
        if (!nlc_probe_valid)
        {
            nlc_probe_valid = true;
            nlc_probe_idx = 0;
            nlc_probe_accum = 1.f;
            for (u32 i = 0; i < nlc_probe_dirs; ++i)
            {
                nlc_probe_dist[i] = -1.f;
                nlc_probe_abs[i] = 0.1f;
            }
        }

        nlc_probe_accum += dt;
        if (nlc_probe_accum >= 1.f / 30.f)
        {
            nlc_probe_accum = 0.f;
            const Fvector L = listener_position();
            for (u32 k = 0; k < 3; ++k)
            {
                const u32 i = nlc_probe_idx++ % nlc_probe_dirs;
                const float range = (i == nlc_probe_dirs - 1) ? 80.f : C.probe_len;
                float d, a;
                if (nlc_probe_ray(L, dirs[i], range, d, a))
                {
                    nlc_probe_dist[i] = d;
                    nlc_probe_abs[i] = a;
                }
                else
                    nlc_probe_dist[i] = -1.f;
            }
        }

        const bool roof = nlc_probe_dist[nlc_probe_dirs - 1] >= 0.f;
        float wsum = 0.f, esum = 0.f, asum = 0.f;
        u32 an = 0, dn = 0;
        float ds[nlc_probe_dirs];
        for (u32 i = 0; i < nlc_probe_dirs - 1; ++i)
        {
            const bool down = (i % 3) == 0;
            const float w = down ? 0.3f : 1.f;
            wsum += w;
            const float d = nlc_probe_dist[i];
            if (d < 0.f)
            {
                // under a roof a miss is a long axis (tunnel, hall): it counts for the room size
                if (roof && !down)
                    ds[dn++] = C.probe_len;
                continue;
            }
            esum += w * clampr((C.enc_far - d) / _max(C.enc_far - C.enc_near, 1.f), 0.f, 1.f);
            asum += nlc_probe_abs[i];
            ++an;
            if (!down)
                ds[dn++] = d;
        }
        E = esum / wsum;
        if (!roof)
            E *= C.roof_open_k;
        if (dn)
        {
            std::sort(ds, ds + dn);
            const u32 trim = dn / 10;
            float s = 0.f;
            for (u32 i = trim; i < dn - trim; ++i)
                s += ds[i];
            Lm = s / float(dn - 2 * trim);
            Ln = ds[dn / 4];
            // room size for the decay: the trimmed mean is dominated by the near walls (a tunnel read 3 m and decayed in
            // 0.6 s), the cube mean by long axes and doorways; their geometric mean, distances capped at enc_far
            float cube = 0.f;
            for (u32 i = 0; i < dn; ++i)
            {
                const float d = _min(ds[i], C.enc_far);
                cube += d * d * d;
            }
            Lr = _sqrt(Lm * powf(cube / float(dn), 1.f / 3.f));
        }
        if (an)
            Am = asum / float(an);
    }

    // dt is clamped: the first update after a load can carry a huge or negative timer delta, and
    // expf(-dt / tau) then gives inf, 0 * inf = NaN, which stuck in the smoothing forever (nlc-3.589.147)
    const float k = 1.f - expf(-dt / 0.6f);
    nlc_enc += (E - nlc_enc) * k;
    nlc_mean_dist += (Lm - nlc_mean_dist) * k;
    nlc_near_dist += (Ln - nlc_near_dist) * k;
    nlc_room_size += (Lr - nlc_room_size) * k;
    nlc_absorb += (Am - nlc_absorb) * k;
    if (!_valid(nlc_enc) || !_valid(nlc_mean_dist) || !_valid(nlc_absorb) || !_valid(nlc_near_dist) || !_valid(nlc_room_size))
    {
        Msg("! [snd_rv] probe value invalid (E=%f L=%f a=%f, dt=%f), reset", nlc_enc, nlc_mean_dist, nlc_absorb, dt);
        nlc_enc = _valid(E) ? E : 0.f;
        nlc_mean_dist = _valid(Lm) ? Lm : C.probe_len;
        nlc_near_dist = _valid(Ln) ? Ln : C.probe_len;
        nlc_room_size = _valid(Lr) ? Lr : C.probe_len;
        nlc_absorb = _valid(Am) ? Am : 0.1f;
    }
}

// NLC: snd_audio_report - one-shot dump of the audio state for runtime checks (docs/AUDIO_DESIGN.md 11)
void CSoundRender_Core::nlc_report()
{
    std::scoped_lock m{m_bLocked};

    Msg("[snd_report] occ_mode %s, strength %.2f (ltx %.2f), rays %d (last update used %u), reverb_mode %d, snd_efx %s, delay %s (%.0f m, %.0f m/s)",
        psSoundOccMode ? "on" : "off", psSoundOccStrength, nlc_cfg.strength, psSoundOccRays, nlc_rays_used, psSoundReverbMode,
        psSoundFlags.test(ss_EAX) ? "on" : "off", psSoundDelay ? "on" : "off", psSoundDelayMinDist, psSoundSpeed);
    Msg("[snd_report] EFX %s (%s), sends %d, echo slot %s, effects attached %s, materials %u, CFORM %s, snd_env zones %s, SOM %s",
        bEFX ? "on" : "off", efx_eax ? "EAX reverb" : "reverb", efx_sends, slot_echo ? "yes" : "no", efx_slots_attached ? "yes" : "no",
        nlc_cfg.mtl.size(), geom_MODEL ? "yes" : "no", geom_ENV ? "yes" : "no", geom_SOM ? "yes" : "no");
    Msg("[snd_report] listener probe: enclosure %.2f, mean distance %.1f m, near distance %.1f m, room size %.1f m, absorption %.2f, zone [%s]", nlc_enc,
        nlc_mean_dist, nlc_near_dist, nlc_room_size, nlc_absorb, e_target ? e_target->name.c_str() : "-");
    Msg("[snd_report] caps: max %.0f dB, loud %.0f/%.2f/%.0f dB, terrain %.0f dB; cache hits so far %u; filters %s", nlc_cfg.max_db, nlc_cfg.loud_cap,
        nlc_cfg.loud_slope, nlc_cfg.loud_max, nlc_cfg.terrain_cap, nlc_cache_hits, nlc_filters_active() ? "on (occlusion on the direct path)" : "off (occlusion in the gain)");
    if (efx_last_valid)
        Msg("[snd_report] reverb: gain %.3f gainHF %.2f decay %.2f s hf_ratio %.2f refl %.3f / %.3f s late %.3f / %.3f s density %.2f",
            efx_last.flGain, efx_last.flGainHF, efx_last.flDecayTime, efx_last.flDecayHFRatio, efx_last.flReflectionsGain, efx_last.flReflectionsDelay,
            efx_last.flLateReverbGain, efx_last.flLateReverbDelay, efx_last.flDensity);
    else
        Msg("[snd_report] reverb: not committed yet (snd_efx off?)");

    u32 rendered = 0;
    for (CSoundRender_Target* T : s_targets)
    {
        CSoundRender_Emitter* E = T->get_emitter();
        if (!E || !T->get_Rendering())
            continue;
        ++rendered;
        const float dist = E->b2D ? 0.f : listener_position().distance_to(E->p_source.position);
        Msg("[snd_report]   %s%s d=%.1f vol=%.2f occ=-%.1fdB (gain %.3f) hf=%.2f path=%u src_enc=%.2f%s%s%s%s%s", E->owner_data ? E->source()->fname.c_str() : "?",
            E->b2D ? " (2D)" : "", dist, E->smooth_volume, E->nlc_occ_db, E->occluder_volume, E->nlc_occ_hf, E->nlc_occ_path, E->nlc_src_enc,
            E->nlc_loud ? " loud" : "", E->nlc_delayed ? " delayed" : "", E->nlc_env_ambient ? " ambient" : "", E->nlc_no_occl ? " no_occlude" : "",
            E->nlc_occ_pending ? " pending" : "");
    }
    Msg("[snd_report] listener filter: gain %.2f hf %.2f (target %.2f / %.2f), %u exempt parts; HRTF %s, 2D sounds %s", nlc_lf_gain, nlc_lf_hf,
        nlc_lf_gain_t, nlc_lf_hf_t, nlc_lf_exempt_parts.size(), nlc_hrtf_on ? "on" : "off", psSoundHrtf2D ? "through HRTF" : "bypass HRTF");
    Msg("[snd_report] %u voices rendering, %u emitters", rendered, s_emitters.size());
}
