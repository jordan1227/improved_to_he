#include "stdafx.h"

#include "Rain.h"
#include "igame_persistent.h"
#include "environment.h"

#include "../xr_3da/perlin.h"

#include "render.h"
#include "igame_level.h"
#include "xr_object.h"

#include <mutex>

// NLC .216: impact events ( Environment.h )
namespace
{
std::mutex nlc_impact_mx;
SNlcImpact nlc_impact_buf[NLC_IMPACT_MAX];
u32 nlc_impact_head{};
} // namespace

void nlc_impact_add(const Fvector& pos, float normal_y, u32 kind, float strength)
{
    if (normal_y < 0.7f || strength <= 0.f || pos.distance_to_sqr(Device.vCameraPosition) > _sqr(40.f))
        return;
    const float now = Device.fTimeGlobal;
    std::scoped_lock lock(nlc_impact_mx);
    for (const SNlcImpact& e : nlc_impact_buf)
        if (e.strength > 0.f && e.kind == kind && now - e.t0 >= 0.f && now - e.t0 < 0.2f && e.pos.distance_to_sqr(pos) < _sqr(0.25f))
            return;
    nlc_impact_buf[nlc_impact_head] = SNlcImpact{pos, now, std::min(strength, 2.f), kind};
    nlc_impact_head = (nlc_impact_head + 1) % NLC_IMPACT_MAX;
}

void nlc_impact_clear()
{
    std::scoped_lock lock(nlc_impact_mx);
    for (SNlcImpact& e : nlc_impact_buf)
        e = SNlcImpact{};
    nlc_impact_head = 0;
}

u32 nlc_impacts_copy(SNlcImpact* out, u32 max)
{
    const float now = Device.fTimeGlobal;
    std::scoped_lock lock(nlc_impact_mx);
    u32 n = 0;
    for (const SNlcImpact& e : nlc_impact_buf)
        if (e.strength > 0.f && now - e.t0 >= 0.f && now - e.t0 < 3.f && n < max)
            out[n++] = e;
    return n;
}

CPerlinNoise1D* RainPerlin;

// NLC .215: wind gust of the drops ( was 2 x Perlin at 0.3 / s added to the 0 ~ 1 weather wind: the drops swung between
// straight down and 35 degrees with their spawn point up to ~28 m upwind every ~3 s, read as waves of heavy rain and
// then little from the wind side ). Console rain_gust / rain_gust_speed; the old look is 2 / 0.3
float ps_rain_gust{0.3f};
float ps_rain_gust_speed{0.15f};
// NLC ( docs/RAIN_CLOUDS_DESIGN.md R2 / R3 ): spread of the drops around the rain axis ( x 2.5 ~ 7.5 deg ), and gust sheets
// ( the gust phase travels downwind at ~9 m/s, so heavier, more slanted bands sweep through; 0 = one gust for all drops )
float ps_rain_spread{1.f};
float ps_rain_sheets{1.f};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEffect_Rain::CEffect_Rain()
{
    state = stIdle;

    snd_Ambient.create("ambient\\rain", st_Effect, sg_Undefined);

    RainPerlin = xr_new<CPerlinNoise1D>(Random.randI(0, 0xFFFF));
    RainPerlin->SetOctaves(2);
    RainPerlin->SetAmplitude(0.66666f);

    p_create();
}

CEffect_Rain::~CEffect_Rain()
{
    xr_delete(RainPerlin);
    snd_Ambient.destroy();

    p_destroy();
}

// NLC: takes the drop's own pitch ( was the wind velocity with one gust for all drops )
void CEffect_Rain::Prepare(Fvector2& offset, Fvector3& axis, float pitch, float W_Direction)
{
    axis.setHP(W_Direction, pitch - PI_DIV_2);
    // Distance upwind at which a drop born source_offset above lands at the aim point
    const float dist = std::tan(pitch) * source_offset;
    // 0 is North
    float fixNorth = W_Direction - PI_DIV_2;
    // Set offset
    offset.set(dist * _cos(fixNorth), dist * _sin(fixNorth));
}

// Born
// NLC ( docs/RAIN_CLOUDS_DESIGN.md R2 / R3 ): in calm air the drops were the most scattered ( +-15 deg x ( 1.5 - wind ) )
// and fell at half speed. Now: one rain axis with a small spread that grows a little with the wind; the fall speed does
// not depend on the wind ( a slanted drop flies the longer path at the same vertical speed ); slow ( small ) drops lean
// more than fast ones; the gust is a wave travelling downwind, so slanted, heavier bands sweep through.
void CEffect_Rain::Born(Item& dest, float radius, float speed)
{
    ZoneScoped;

    const CEnvironment& env = g_pGamePersistent->Environment();
    const float Wind_Direction = -(env.CurrentEnv->wind_direction + env.nlc_wind_gust_offset); // + the gust swing ( drops only )

    // Wind Velocity [ From 0 ~ 1000 to 0 ~ 1 ]
    float Wind_Velocity = env.CurrentEnv->wind_velocity * 0.001f;
    clamp(Wind_Velocity, 0.0f, 1.0f);

    // Camera Position
    const Fvector& view = Device.vCameraPosition;

    // Random landing point ahead of where the player is facing
    const float r = radius * 0.5f;
    const Fvector FinalView = Fvector().mad(view, Device.vCameraDirection, 5.0f);
    const Fvector2 land{FinalView.x + ::Random.randF(-r, r), FinalView.z + ::Random.randF(-r, r)};

    // Gust ( NLC .215 rain_gust / rain_gust_speed ), its phase travelling downwind ( rain_sheets )
    const float along = land.x * env.nlc_downwind.x + land.y * env.nlc_downwind.z;
    const float Wind_Gust = RainPerlin->GetContinious((Device.fTimeGlobal - ps_rain_sheets * along / 9.f) * ps_rain_gust_speed) * ps_rain_gust;
    const float W = std::clamp(Wind_Velocity + Wind_Gust, 0.f, 1.f);

    // Vertical fall speed ( x 0.7: between the old calm x 0.5 and windy x 1 )
    const float fall = ::Random.randF(drop_speed_min, drop_speed_max) * speed * 0.7f;
    const float fall_mid = (drop_speed_min + drop_speed_max) * 0.5f * speed * 0.7f;
    const float lean_k = std::clamp(_sqrt(fall_mid / std::max(fall, 0.01f)), 0.8f, 1.25f);
    const float pitch = std::min(drop_max_angle * W * lean_k, deg2rad(45.f));

    Fvector Rain_Axis = {0, -1, 0};
    Fvector2 Rain_Offset;
    Prepare(Rain_Offset, Rain_Axis, pitch, Wind_Direction);

    // Spread around the axis
    dest.D.random_dir(Rain_Axis, deg2rad(2.5f + 5.f * W) * ps_rain_spread);
    // Set final destination
    dest.P.set(Rain_Offset.x + land.x, source_offset + view.y, Rain_Offset.y + land.y);
    // Set speed: the same vertical speed along the slanted path
    dest.fSpeed = fall / std::max(-dest.D.y, 0.5f);
    // Born

    float height = max_distance;
    const BOOL b_hit = RayPick(dest.P, dest.D, height, collide::rqtBoth);
    RenewItem(dest, height, b_hit);
}

BOOL CEffect_Rain::RayPick(const Fvector& s, const Fvector& d, float& range, collide::rq_target tgt)
{
    ZoneScoped;

    collide::rq_result RQ{};
    CObject* E = g_pGameLevel->CurrentViewEntity();
    const BOOL bRes = g_pGameLevel->ObjectSpace.RayPick(s, d, range, tgt, RQ, E);
    if (bRes)
        range = RQ.range;
    return bRes;
}

void CEffect_Rain::RenewItem(Item& dest, float height, BOOL bHit)
{
    dest.uv_set = Random.randI(2);
    if (bHit)
    {
        dest.dwTime_Life = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.dwTime_Hit = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.Phit.mad(dest.P, dest.D, height);
    }
    else
    {
        dest.dwTime_Life = Device.dwTimeGlobal + iFloor(1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.dwTime_Hit = Device.dwTimeGlobal + iFloor(2 * 1000.f * height / dest.fSpeed) - Device.dwTimeDelta;
        dest.Phit.set(dest.P);
    }
}

void CEffect_Rain::OnFrame()
{
    if (!g_pGameLevel)
        return;

    ZoneScoped;

    // Parse states
    float rain_density = g_pGamePersistent->Environment().CurrentEnv->rain_density;
    float wind_velocity = g_pGamePersistent->Environment().CurrentEnv->wind_velocity * 0.001f;
    clamp(wind_velocity, 0.0f, 1.0f);
    wind_velocity *= (rain_density > 0.0f ? 1.0f : 0.0f); // Only when raining
    // NLC O1 ( docs/RAIN_CLOUDS_DESIGN.md ): was 50 % rain_density + 50 % wind ( heavy night rain without wind played at
    // half volume and the loudness jumped with the weather's 0 / 1000 wind steps ). Now by the rain strength as the
    // weather file has it ( light 0.2 -> 0.44, normal 1 -> 0.8, heavy -> 1 ), the wind adds up to 0.2
    const float rain_raw = std::clamp(g_pGamePersistent->Environment().CurrentEnv->nlc_rain_raw, 0.f, 3.f);
    // ( faded in over rain_density 0 ~ 0.1, so a weather blend into rain does not start at 0.35 )
    float factor = std::clamp(0.35f + 0.45f * rain_raw + 0.2f * wind_velocity, 0.f, 1.f) * std::clamp(rain_density * 10.f, 0.f, 1.f);
    static float hemi_factor = 0.f;

    CObject* E = g_pGameLevel->CurrentViewEntity();
    if (E && E->renderable_ROS())
    {
        const float* hemi_cube = E->renderable_ROS()->get_luminocity_hemi_cube();
        float hemi_val = _max(hemi_cube[0], hemi_cube[1]);
        hemi_val = _max(hemi_val, hemi_cube[2]);
        hemi_val = _max(hemi_val, hemi_cube[3]);
        hemi_val = _max(hemi_val, hemi_cube[5]);

        float f = hemi_val;
        float t = Device.fTimeDelta;
        clamp(t, 0.001f, 1.0f);
        hemi_factor = hemi_factor * (1.0f - t) + f * t;
    }

    switch (state)
    {
    case stIdle:
        if (factor < EPS_L)
        {
            if (snd_Ambient._feedback())
                snd_Ambient.stop();
            return;
        }
        if (snd_Ambient._feedback())
        {
            snd_Ambient.stop();
            return;
        }
        snd_Ambient.play(nullptr, sm_Looped | sm_2D | sm_EnvAmbient); // NLC: dulled indoors by the room probe
        if (snd_Ambient._feedback()) //Simp: в редких случаях такое случается
            snd_Ambient.set_position({});
        snd_Ambient.set_range(source_offset, source_offset * 2.f);
        state = stWorking;
        break;
    case stWorking:
        if (factor < EPS_L)
        {
            snd_Ambient.stop();
            state = stIdle;
            return;
        }
        break;
    }

    // ambient sound
    if (snd_Ambient._feedback())
    {
        // NLC: the hemi (sky light) factor alone muted rain under any roof; indoors it keeps a floor by the listener's
        // enclosure and the sound engine dulls it (docs/AUDIO_DESIGN.md 13.2 item 7)
        float hemi_k = hemi_factor;
        if (psSoundOccMode)
            hemi_k = _max(hemi_k, ::Sound->nlc_rain_indoor() * ::Sound->nlc_listener_enclosure());
        snd_Ambient.set_volume(_max(0.1f * std::clamp(rain_density * 10.f, 0.f, 1.f), factor) * hemi_k); // NLC: the floor fades in too
    }
}

void CEffect_Rain::Render(CBackend& cmd_list, const bool scope)
{
    if (!g_pGameLevel)
        return;

    m_pRender->Render(cmd_list, *this, scope);
}

void CEffect_Rain::Calculate()
{
    if (!g_pGameLevel)
        return;

    m_pRender->Calculate(*this);
}

// startup _new_ particle system
void CEffect_Rain::Hit(Fvector& pos)
{
    // NLC .213b: splash chance 0.2 -> 0.3, scaled by the weather density ( heavy rain ~0.45 )
    // NLC G2 ( docs/RAIN_CLOUDS_DESIGN.md ): 0.45 ~ 1 ( light rain 0.5, normal 0.8, heavy 1 ); the pool holds ~650 at the
    // heavy-rain drop count and the shorter splash life
    const float raw = std::clamp(g_pGamePersistent->Environment().CurrentEnv->nlc_rain_raw, 0.f, 3.f);
    if (::Random.randF() > std::clamp(0.45f + 0.35f * raw, 0.45f, 1.f))
        return;

    Particle* P = p_allocate();
    if (nullptr == P)
        return;

    const Fsphere& bv_sphere = m_pRender->GetDropBounds();

    P->time = particles_time;
    P->mXForm.rotateY(::Random.randF(PI_MUL_2));
    P->mXForm.translate_over(pos);
    P->mXForm.transform_tiny(P->bounds.P, bv_sphere.P);
    P->bounds.R = bv_sphere.R * 1.6f; // NLC G1: the crown grows to x1.25 wide, x1.56 tall
}

// initialize particles pool
void CEffect_Rain::p_create()
{
    // pool
    particle_pool.resize(max_particles);
    for (u32 it = 0; it < particle_pool.size(); it++)
    {
        Particle& P = particle_pool[it];
        P.prev = it ? (&particle_pool[it - 1]) : nullptr;
        P.next = (it < (particle_pool.size() - 1)) ? (&particle_pool[it + 1]) : nullptr;
    }

    // active and idle lists
    particle_active = nullptr;
    particle_idle = &particle_pool.front();
}

// destroy particles pool
void CEffect_Rain::p_destroy()
{
    // active and idle lists
    particle_active = nullptr;
    particle_idle = nullptr;

    // pool
    particle_pool.clear();
}

// _delete_ node from _list_
void CEffect_Rain::p_remove(Particle* P, Particle*& LST)
{
    VERIFY(P);
    Particle* prev = P->prev;
    P->prev = nullptr;
    Particle* next = P->next;
    P->next = nullptr;
    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;
    if (LST == P)
        LST = next;
}

// insert node at the top of the head
void CEffect_Rain::p_insert(Particle* P, Particle*& LST)
{
    VERIFY(P);
    P->prev = nullptr;
    P->next = LST;
    if (LST)
        LST->prev = P;
    LST = P;
}

// determine size of _list_
int CEffect_Rain::p_size(Particle* P)
{
    if (nullptr == P)
        return 0;
    int cnt = 0;
    while (P)
    {
        P = P->next;
        cnt += 1;
    }
    return cnt;
}

// alloc node
CEffect_Rain::Particle* CEffect_Rain::p_allocate()
{
    Particle* P = particle_idle;
    if (nullptr == P)
        return nullptr;
    p_remove(P, particle_idle);
    p_insert(P, particle_active);
    return P;
}

// xr_free node
void CEffect_Rain::p_free(Particle* P)
{
    p_remove(P, particle_active);
    p_insert(P, particle_idle);
}
