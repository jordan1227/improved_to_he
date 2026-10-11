// WallmarksEngine.h: interface for the CWallmarksEngine class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

namespace WallmarksEngine
{
struct wallmark_static_queue_item;
struct wm_slot;
}

class CSkeletonWallmark;
class CKinematics;
struct wm_static_params; // xr_3da/Render.h

class CWallmarksEngine
{
public:
    typedef WallmarksEngine::wm_slot wm_slot;

public:
    struct static_wallmark
    {
        Fsphere bounds;
        xr_vector<FVF::LIT> verts;
        float ttl;
        // NLC: optional per-mark behaviour (blood pools); all zero = vanilla mark
        float ttl_max; // fade reference in seconds (r__wallmark_ttl unless shaped)
        float born; // Device.fTimeGlobal at creation (drying tint)
        float grow_start; // Device.fTimeGlobal when the spread started
        float grow_dur; // 0 = full size at once
        float dry_dur; // 0 = no tint
        u32 owner; // non-zero: a new mark of the same owner replaces this one
        bool shaped; // custom life: solid, then fades over the last quarter of it
        float faint; // 0..0.95: share of the opacity that is always taken away
        float wash; // 0..1: how much rain has washed the mark away
        bool rain; // open to the sky: rain washes it away
        bool washed; // fully washed away: kept empty so a refresh does not bring it back
    };
    DEFINE_VECTOR(static_wallmark*, StaticWMVec, StaticWMVecIt);
    DEFINE_VECTOR(wm_slot*, WMSlotVec, WMSlotVecIt);

private:
    xrCriticalSection lock;
    ref_geom hGeom;
    WMSlotVec marks;


    StaticWMVec static_pool;

    Fvector sml_normal;
    CFrustum sml_clipper;
    sPoly sml_poly_dest;
    sPoly sml_poly_src;

    CDB::Collector sml_collector;
    xr_vector<u32> sml_adjacency;

    xr_vector<WallmarksEngine::wallmark_static_queue_item> static_items_to_add;

private:
    wm_slot* FindSlot(const ref_shader& shader);
    wm_slot* AppendSlot(const ref_shader& shader);

private:
    void BuildMatrix(Fmatrix& dest, float invsz, const Fvector& from) const;
    void RecurseTri(u32 T, Fmatrix& mView, static_wallmark& W);

    static_wallmark* static_wm_allocate();
    void static_wm_destroy(static_wallmark* W);

    void static_wm_render(const static_wallmark* W, FVF::LIT*& V);

    void add_static_wallmark_internal(const WallmarksEngine::wallmark_static_queue_item& q);

public:
    CWallmarksEngine();
    ~CWallmarksEngine();

    void AddStaticWallmark(CDB::TRI* pTri, const Fvector* pVerts, const Fvector& contact_point, const ref_shader& sh, float wm_size, const wm_static_params* prm = nullptr);

    void AddSkeletonWallmark(Fmatrix* xf, CKinematics* obj, const ref_shader& sh, const Fvector& start, const Fvector& dir, float size);
    void AppendSkeletonWallmark(intrusive_ptr<CSkeletonWallmark> wm);

    void add_static_wallmarks_async();

    // render
    void Render();

    void Clear();
};
