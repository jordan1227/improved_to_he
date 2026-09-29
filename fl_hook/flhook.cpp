#include <windows.h>
#include <d3d11.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#if defined(_MSC_VER) && !defined(__clang__)
#include <intrin.h>
#define FL_NOINLINE __declspec(noinline)
#define FL_RETURN_ADDRESS() _ReturnAddress()
#else
#define FL_NOINLINE __attribute__((noinline))
#define FL_RETURN_ADDRESS() __builtin_return_address(0)
#endif

#define RVA_EXECCMD         0x000837a0ULL
#define RVA_STEAL_END       0x000837b2ULL
#define RVA_LOADBUF         0x00c1fcb0ULL
#define RVA_PUSHSTR         0x00c18fb0ULL
#define RVA_PCALL           0x00c1a260ULL
#define RVA_PRINTOUT        0x0042a9a0ULL
#define RVA_GAISPACE        0x01203a18ULL
#define RVA_RELOADINI       0x0077cf30ULL
#define RVA_MSG             0x00090140ULL
#define RVA_STR_TABLE_LOAD    0x003CBAD0ULL
#define RVA_STR_TABLE_ANCHOR  0x00DF85E8ULL
#define RVA_RT_CREATE       0x0018F260ULL
#define RVA_GETFONT_GRAFFITI  0x00528C80ULL
#define RVA_FONT_OUT          0x0005CA40ULL
#define RVA_HW              0x0110B2330ULL
#define OFF_HW_PDEVICE      0x10
#define OFF_CRT_PRT         0x28
#define RVA_XR_FS           0x01224098ULL
#define RVA_UPDATE_PATH     0x000A2F30ULL
#define RVA_FILE_LIST_OPEN  0x000A0290ULL
#define RVA_FILE_LIST_CLOSE 0x000A0730ULL
#define RVA_R_OPEN          0x000A1C20ULL
#define RVA_R_CLOSE         0x000A2580ULL
#define OFF_SCRENG          0x38
#define OFF_LUAVM           0x98

#define RVA_RDEV_INSTANCE   0x0013A890ULL
#define RVA_CREATE_TEXTURE  0x001E9620ULL
#define RVA_TEX_UNLOAD      0x0025BE80ULL
#define RVA_TEX_LOAD        0x0025AD60ULL
#define OFS_RDEV_RESOURCES  0x08

#define RVA_RESTRICT          0x00421D50ULL
#define RVA_RESTRICT_END      0x00421D63ULL
#define RVA_RO_NET_SPAWN      0x006E7620ULL
#define RVA_RO_NET_SPAWN_END  0x006E762EULL
#define RVA_GPGAMELEVEL       0x011FACA8ULL
#define RVA_SHARED_SET        0x0000B020ULL

#define RVA_RUN_ATK_CHECK     0x00AD1A10ULL
#define RVA_RUN_ATK_CHECK_END 0x00AD1A20ULL

#define RVA_AMBUSH_COVER      0x00B1EA40ULL
#define RVA_AMBUSH_COVER_END  0x00B1EA4FULL
#define RVA_RTDCAST           0x00D56956ULL
#define RVA_RTTI_CGAMEOBJECT  0x010BEB70ULL
#define RVA_RTTI_CAI_STALKER  0x010C0DD8ULL
#define RVA_SETUP_CE_CLOSE    0x00712190ULL
#define RVA_BEST_COVER_CLOSE  0x00712530ULL
#define RVA_AI_SPACE_FN       0x002E7250ULL
#define RVA_COVER_LAMBDA_VFT  0x00EDB988ULL
#define RVA_MI_FREE_EX        0x00B6E0D0ULL
#define RVA_LUA_RAWGETI       0x00C19B30ULL
#define RVA_LUA_SETTOP2       0x00C18250ULL
#define RVA_ALIFE_NOWAY      0x00A93E08ULL
#define RVA_ALIFE_DROPPATH   0x00A93EBAULL
#define RVA_PUSH_COVER_ARG    0x00B2B360ULL
#define RVA_LUABIND_PCALL     0x00C0CED0ULL
#define OFF_SGO_GAME_OBJECT   0x08
#define OFF_STALKER_CE_CLOSE  0xA38
#define OFF_AISPACE_COVER_MGR 0x30
#define OFF_FUNCTOR_LUA_STATE 0x00
#define OFF_FUNCTOR_REF       0x10
#define OFF_LUA_STATE_TOP     0x28
#define COVER_MODE_CLOSE      1
#define COVER_MODE_FAR        2
#define COVER_MODE_KILLER_ID  3
#define RVA_RTTI_CENTITY_ALIVE 0x010BE9B0ULL
#define RVA_LUA_PUSHINTEGER   0x00C18E60ULL
#define RVA_LUA_SETFIELD      0x00C19E70ULL
#define OFF_ENTITY_KILLER_ID  0x308
#define LUA_GLOBALSINDEX      (-10002)
#define RVA_SETUP_CE_BASE     0x0070A500ULL
#define RVA_BEST_COVER_FAR    0x00B615C0ULL
#define OFF_STALKER_CE_FAR    0xA30
#define OFF_EV_ACTUALITY      0x38
#define OFF_EV_ENEMY_POS      0x80
#define OFF_EV_MIN_DISTANCE   0x8C
#define OFF_EV_MAX_DISTANCE   0x90
#define OFF_EV_DEVIATION      0x98
#define RVA_DEVICE_TIME       0x0122378CULL

#define RVA_G_ACTOR            0x012039F0ULL
#define RVA_GMLIB_MTL_FIRST    0x01203C88ULL
#define RVA_GMLIB_MTL_LAST     0x01203C90ULL
#define RVA_LUA_PUSHCCLOSURE   0x00C190E0ULL
#define OFF_EA_MATERIAL_MGR    0x368
#define OFF_MM_MY_IDX          0x14
#define OFF_MM_LAST_IDX        0x48
#define OFF_SGAMEMTL_NAME      0x04
#define OFF_STRVALUE_TEXT      0x14

#define RVA_ACTOR_RENDER      0x002E0860ULL
#define RVA_ACTOR_RENDER_END  0x002E086EULL
#define RVA_ACTOR_ONHUDDRAW   0x002E09F0ULL
#define RVA_ACTOR_ONHUDDRAW_END 0x002E0A00ULL
#define RVA_G_PLAYER_HUD      0x01203A00ULL
#define RVA_RENDER            0x011FB170ULL
#define OFF_IR_TO_ACTOR       0x90
#define OFF_ACTOR_CAM_ACTIVE  0x810
#define OFF_PH_SCRIPT_PART    0x48
#define OFF_PH_SCRIPT_ITEM    0x70
#define OFF_PH_ITEM_POS       0xC0
#define OFF_VT_ADD_VISUAL     0x60
#define OFF_VT_DCAST_RVIS     0x120

#define RVA_DEVICE            0x01223760ULL
#define OFF_DEV_TIME_GLOBAL   0x2C
#define OFF_PH_ATTACHED_0     0x250
#define OFF_AHI_PARENT_ITEM   0x48
#define OFF_AHI_ITEM_XFORM    0x1D5
#define RVA_LUA_TONUMBER      0x00C18810ULL
#define RVA_LUA_PUSHBOOLEAN   0x00C19260ULL

#define RVA_FIRETRACE         0x004F9F10ULL
#define RVA_FIRETRACE_END     0x004F9F26ULL
#define RVA_HUD_PLAYMOTION    0x004F37D0ULL
#define RVA_HUD_PLAYMOTION_END 0x004F37DEULL
#define RVA_INI_LINE_EXIST    0x000AA960ULL
#define RVA_PSETTINGS         0x011FB1C0ULL
#define KNIFE_ENGINE_STAMP    0x6A15800Cu
#define KNIFE_ENGINE_IMAGE    0x01344000u
#define OFF_HUD_TO_COBJECT    0x2D0
#define OFF_COBJECT_SECTION   0x10C
#define RVA_GL_ONSHOT         0x005FE370ULL
#define RVA_GL_ONSHOT_END     0x005FE380ULL
#define OFF_WPN_PARENT        0x27C
#define OFF_WPN_ADDON_FLAGS   0x928
#define OFF_WPN_GRENADE_MODE  0x1228
#define RVA_LUA_GETFIELD      0x00C19A30ULL
#define RVA_LUA_GETTOP        0x00C18240ULL
#define RVA_LUA_TYPE          0x00C18560ULL
#define RVA_LUA_TOLSTRING     0x00C188F0ULL
#define LUA_TFUNCTION         6

#define RVA_ACTOR_KEYPRESS      0x00373860ULL
#define RVA_ACTOR_KEYPRESS_END  0x00373871ULL
#define ACT_TORCH               19
#define ACT_TORCH_MODE          62
#define OFF_IRCV_INVENTORY      0x98
#define OFF_INV_ACTIVE_SLOT     0x88
#define OFF_INV_SLOTS_FIRST     0x60
#define OFF_INV_SLOTS_LAST      0x68
#define INV_SLOT_STRIDE         0x20
#define OFF_INV_SLOT_ITEM       0x08
#define INV_SLOT_MAX            64

#define OFF_PH_MODEL          0x228
#define OFF_PH_MODEL_2        0x230
#define OFF_VT_DCAST_PKIN     0x128
#define OFF_VT_SET_RFLAG      0x000
#define OFF_VT_RCHILDCOUNT    0x010
#define OFF_VT_GETDEBUGNAME   0x018
#define WM_MAX_IDX            64

#define RVA_ROTJUMP_NULLWRITE 0x00ACFB03ULL

#define OFF_RA_MAN            0x08
#define OFF_RA_OBJECT         0x10
#define OFF_RA_ACTIVE         0x18
#define OFF_RA_MIN_DIST       0x60
#define OFF_RA_MAX_DIST       0x64
#define OFF_RA_TIME_NEXT      0x70
#define OFF_BM_ENEMY          0xA98
#define OFF_OBJ_POS           0xC8
#define OFF_OBJ_XFORM_K_X     0xB8
#define OFF_OBJ_XFORM_K_Z     0xC0
#define OFF_MAN_MOVEMENT      0x78
#define OFF_MOV_VEL_CUR       0x40
#define BOAR_RETRIG_MS        2200u
#define BOAR_FACE_COS2        0.329f
#define BOAR_MIN_VEL          2.0f

#define OFF_LVL_OBJECTS     0x00A0
#define OFF_OBJ_ID          0x0100
#define OFF_OBJ_NAME        0x0104
#define OFF_MGR_DEF_IN      0x0020
#define OFF_RO_OBJECT       0x0008
#define OFF_SE_NAME_REPL    0x0040
#define OFF_SE_NAME         0x0068
#define STRV_VALUE          20

#define RVA_UI_ADDITEM       0x00731C50ULL
#define RVA_UI_ADDITEM_CONT  0x00731C62ULL
#define RVA_SECURITY_COOKIE  0x010AB940ULL
#define UI_LBI_TAG           0x510
#define UI_LBI_PDATA         0x518
#define UI_TAG_CHARGE_TORCH  77

#define RVA_RAYPICK        0x00044070ULL
#define RVA_ADD_STATIC_WM  0x0014D9E0ULL
#define RVA_CREATE_WMA     0x0016FE60ULL
#define RVA_APPENDMARK     0x001CB820ULL
#define RVA_RENDERFACTORY  0x010ABFE0ULL
#define RVA_GMLIB_FIRST    0x01203C88ULL
#define RVA_GMLIB_LAST     0x01203C90ULL
#define OFF_LVL_OBJSPACE   0x80150
#define OFF_LVL_TRIS       0x80160
#define OFF_LVL_VERTS      0x80170
#define BP_TRI_SIZE        16
#define BP_TRI_DUMMY       0x0C
#define BP_TRI_MATMASK     0x3FFF
#define OFF_MTL_FLAGS      0x14
#define MTL_FL_BLOODMARK   0x10
#define BP_RQ_STATIC       2
#define RVA_WM_ROT_ANGLE   0x001C92D3ULL

#define RVA_SR_GAME_OBJECT1     0x00B090C0ULL
#define RVA_SR_GAME_OBJECT1_END 0x00B090DCULL
#define RVA_LB_DEF_U32          0x00699DC0ULL
#define RVA_SGO_CAST_INVITEM    0x00697270ULL
#define OFF_INVITEM_COST        0xE0

#define FS_ListFiles   1u
#define FS_ListFolders 2u

struct IReader {
    void*   vtbl;
    void*   unk08;
    char*   data;
    size_t  Pos;
    size_t  Size;
};

struct VecP {
    char** first;
    char** last;
    char** endd;
};

typedef int  (*loadbuf_t)(void* L, const char* buf, size_t sz, const char* name, const char* mode);
typedef const char* (*pushstr_t)(void* L, const char* s);
typedef int  (*pcall_t)(void* L, int nargs, int nresults, int errfunc);
typedef char (*printout_t)(void* L, const char* name, int status);
typedef void (*execcmd_t)(void* self, const char* cmd, char record, char allow_disabled);
typedef void* (*reload_ini_t)(void);
typedef void (*msg_t)(const char* fmt, ...);
typedef const char* (*update_path_t)(void* fs, char dest[520], const char* root, const char* append);
typedef void* (*file_list_open_t)(void* fs, const char* path, unsigned flags);
typedef void (*file_list_close_t)(void* fs, void** pvec);
typedef IReader* (*r_open_t)(void* fs, const char* path, const char* fname);
typedef void (*r_close_t)(void* fs, IReader** pr);
typedef void (*restrict_t)(void* mgr, unsigned short id, void* out_r, void* in_r);
typedef int  (*ro_spawn_t)(void* self, void* data);
typedef void* (*rdev_instance_t)(void);
typedef void* (*create_texture_t)(void* resources, const char* name);
typedef void  (*tex_unload_t)(void* tex);
typedef void  (*tex_load_t)(void* tex, const char* name);
typedef void* (*shared_set_t)(void* dst, const char* s);
typedef unsigned char (*run_atk_check_t)(void* self);
typedef void (*actor_render_t)(void* self, unsigned context_id, void* root);
typedef void (*actor_onhuddraw_t)(void* self, void* hud, unsigned context_id, void* root);
typedef void (*actor_kbpress_t)(void* self, unsigned cmd);
typedef void* (*dcast_rvis_t)(void* self);
typedef void (*add_visual_t)(void* render, unsigned context_id, void* root,
                             void* visual, void* xform);
typedef unsigned char (*raypick_t)(void* objspace, const float* start, const float* dir,
                                   float range, int tgt, void* result, void* ignore);
typedef void  (*add_static_wm_t)(void* render, void* array, const float* pos,
                                 float size, void* tri, void* verts);
typedef void* (*create_wma_t)(void* factory);
typedef void  (*appendmark_t)(void* array, const char* texture);
typedef void* (*dcast_pkin_t)(void* self);
typedef void  (*set_rflag_t)(void* kin, unsigned id, bool state);
typedef unsigned (*rchildcount_t)(void* kin);
typedef void* (*getdebugname_t)(void* visual, void* out_shared_str);

typedef const void* (*ambush_cover_t)(void* self, const float* pos, const float* enemy,
                                      float radius, float min_distance, const void* cb);
typedef void* (*rtdcast_t)(void* p, long vfdelta, void* srct, void* dstt, int isref);
typedef void* (*sr_go1_t)(void* ret, void* src);
typedef void* (*lb_def_u32_t)(void* cls, const char* name, void* memfn);
typedef void* (*sgo_cast_invitem_t)(void* script_obj);
typedef void  (*setup_ce_close_t)(void* ev, const float* enemy, float mn, float mx,
                                  float dev, const void* fn);
typedef const void* (*best_cover_close_t)(void* mgr, const float* pos, float radius, void* ev);
typedef void* (*ai_space_fn_t)(void);
typedef void  (*mi_free_ex_t)(void* p, void* unused);
typedef void  (*lua_rawgeti_t)(void* L, int idx, int n);
typedef void  (*lua_settop2_t)(void* L, int idx);
typedef void  (*push_cover_arg_t)(void* L, void* tuple);
typedef int   (*luabind_pcall_t)(void* L, int nargs, int nres);
typedef void  (*func_delete_this_t)(void* self, unsigned char dealloc);
typedef void  (*setup_ce_base_t)(void* ev, const void* fn);
typedef void  (*lua_pushinteger_t)(void* L, long long n);
typedef void  (*lua_setfield_t)(void* L, int idx, const char* k);
typedef void  (*lua_pushcclosure_t)(void* L, int (*fn)(void*), int n);
typedef double (*lua_tonumber_t)(void* L, int idx);
typedef void  (*lua_pushboolean_t)(void* L, int b);
typedef void  (*lua_getfield_t)(void* L, int idx, const char* k);
typedef int   (*lua_gettop_t)(void* L);
typedef int   (*lua_type_t)(void* L, int idx);
typedef const char* (*lua_tolstring_t)(void* L, int idx, size_t* len);
typedef void  (*fire_trace_t)(void* self, const float* P, const float* D);
typedef unsigned (*hud_playmotion_t)(void* self, const char* key, bool mix_in,
                                     unsigned state, bool random, float speed);
typedef bool (*ini_line_exist_t)(void* ini, const char* section, const char* key);
typedef void  (*gl_onshot_t)(void* self);
typedef const void* (*best_cover_far_t)(void* mgr, const float* pos, float radius, void* ev,
                                        const void* restrictor);

static uintptr_t g_base = 0;
static ambush_cover_t g_orig_ambush = 0;
static execcmd_t g_orig = 0;
static void* g_console = 0;
static restrict_t g_orig_restrict = 0;
static ro_spawn_t g_orig_ro_spawn = 0;
static run_atk_check_t g_orig_run_atk = 0;
static actor_render_t g_orig_actor_render = 0;
static actor_onhuddraw_t g_orig_actor_onhud = 0;
static actor_kbpress_t g_orig_actor_kbpress = 0;
static fire_trace_t g_orig_fire_trace = 0;
static hud_playmotion_t g_orig_hud_playmotion = 0;
static gl_onshot_t  g_orig_gl_onshot = 0;
static int g_torch_lua = 1;
static int g_torch_blocked_n = 0;
static void* g_actor_ircv = 0;
static void* ircv_active_item(void* self);
static int g_bp_ui = 0;
static int g_skip_body_n = 0;
static int g_skip_hands_n = 0;

static int g_wm_calls = 0;
static int g_wm_applied = 0;
static int g_wm_skipped = 0;
static int g_wm_last_state = -1;
static int g_wm_nomodel = 0;
static int g_wm_mismatch = 0;
static int g_wm_skipped_model = 0;
static int g_wm_skipped_name = 0;

static int   g_boar_ok = 0;
static int   g_boar_fail = 0;
static float g_boar_last_dist = -1.f;
static int   g_alife_noway_patched = 0;
static float g_boar_last_vel = -1.f;
static int   g_boar_last_reason = 0;

static const char WRAP_PRE[] =
    "local function script_name() return '";
static const char WRAP_MID[] =
    "' end; local this; module('";
static const char WRAP_POST[] =
    "', package.seeall, function(m) this = m end); ";

static const char BOOT[] =
    "local a=... local i=a:match(\"^%s*([%w_.]+)%s*$\") "
    "if i then local b=i:gsub(\"%.script$\",\"\") "
    "local m,f=b:match(\"^([%w_]+)%.([%w_]+)$\") "
    "if f then _G[m][f]() else return _G[b] end "
    "else assert(loadstring(a))() end";

static inline void* script_engine()
{
    uintptr_t* pAi = (uintptr_t*)(g_base + RVA_GAISPACE);
    if (!pAi) return 0;
    uintptr_t ai = *pAi;
    if (!ai) return 0;
    return *(void**)(ai + OFF_SCRENG);
}

static inline void* lua_state()
{
    void* se = script_engine();
    if (!se) return 0;
    return *(void**)((uintptr_t)se + OFF_LUAVM);
}

static inline void* locator_fs()
{
    return *(void**)(g_base + RVA_XR_FS);
}

static void log_msg(const char* s)
{
    ((msg_t)(g_base + RVA_MSG))("%s", s);
}

static void log_fmt1(const char* fmt, const char* a)
{
    ((msg_t)(g_base + RVA_MSG))(fmt, a);
}

static void log_fmt1i(const char* fmt, int a)
{
    ((msg_t)(g_base + RVA_MSG))(fmt, a);
}

static void log_fmt2i(const char* fmt, int a, int b)
{
    ((msg_t)(g_base + RVA_MSG))(fmt, a, b);
}

static void run_fl(const char* args)
{
    void* L = lua_state();
    if (!L) return;
    loadbuf_t  loadbuf  = (loadbuf_t)(g_base + RVA_LOADBUF);
    pushstr_t  pushstr  = (pushstr_t)(g_base + RVA_PUSHSTR);
    pcall_t    pcall    = (pcall_t)(g_base + RVA_PCALL);
    printout_t printout = (printout_t)(g_base + RVA_PRINTOUT);

    int rc = loadbuf(L, BOOT, sizeof(BOOT) - 1, "@fl", 0);
    if (rc) { printout(L, "@fl", rc); return; }
    pushstr(L, args);
    rc = pcall(L, 1, 0, 0);
    if (rc) printout(L, "@fl", rc);
}

static const char* gmtl_name(unsigned idx)
{
    char** first = *(char***)(g_base + RVA_GMLIB_MTL_FIRST);
    char** last  = *(char***)(g_base + RVA_GMLIB_MTL_LAST);
    if (!first || !last || last <= first) return 0;
    if (idx >= (unsigned)(last - first)) return 0;
    char* mtl = first[idx];
    if (!mtl) return 0;
    char* s = *(char**)(mtl + OFF_SGAMEMTL_NAME);
    if (!s) return 0;
    return s + OFF_STRVALUE_TEXT;
}

static int step_material_read(const char** ground, const char** self, int* gid)
{
    if (!g_base) return 0;
    void* actor = *(void**)(g_base + RVA_G_ACTOR);
    if (!actor) return 0;
    char* mm = *(char**)((char*)actor + OFF_EA_MATERIAL_MGR);
    if (!mm) return 0;
    unsigned g = *(unsigned short*)(mm + OFF_MM_LAST_IDX);
    unsigned s = *(unsigned short*)(mm + OFF_MM_MY_IDX);
    const char* gn = gmtl_name(g);
    if (!gn) return 0;
    *ground = gn;
    *self   = gmtl_name(s);
    *gid    = (int)g;
    return 1;
}

#define HUDRC_MAX_POS   0.05f
#define HUDRC_MAX_ROT   0.25f
#define HUDRC_WATCHDOG  250u

static int      g_hudrc_on     = 0;
static float    g_hudrc_pos[3] = {0.f, 0.f, 0.f};
static float    g_hudrc_rot[3] = {0.f, 0.f, 0.f};
static void*    g_hudrc_owner  = 0;
static unsigned g_hudrc_stamp  = 0;
static int      g_hudrc_frames = 0;
static int      g_hudrc_clears = 0;
static int      g_hudrc_expired = 0;
static int      g_hudrc_rej_nohud = 0;
static int      g_hudrc_rej_item  = 0;
static int      g_hudrc_rej_bad   = 0;
static int      g_hudrc_lock   = 0;
static int      g_hudrc_rej_lock = 0;
static float    g_hudrc_saved[16];
static float*   g_hudrc_saved_at = 0;

static inline unsigned hudrc_time_ms()
{
    if (!g_base) return 0;
    return *(unsigned*)(g_base + RVA_DEVICE + OFF_DEV_TIME_GLOBAL);
}

static inline int hudrc_finite(float v)
{
    return v == v && (v - v) == 0.f;
}

static inline float hudrc_clamp(float v, float lim)
{
    return v > lim ? lim : (v < -lim ? -lim : v);
}

static void* hudrc_item()
{
    if (!g_base) return 0;
    void** pph = (void**)(g_base + RVA_G_PLAYER_HUD);
    void* ph = pph ? *pph : 0;
    if (!ph) return 0;
    void* it = *(void**)((char*)ph + OFF_PH_ATTACHED_0);
    if (!it) return 0;
    if (!*(void**)((char*)it + OFF_AHI_PARENT_ITEM)) return 0;
    return it;
}

// CHudItem is at +0x430 and CObject at +0x160 in this engine build (PDB).
// Only the actor's attached wpn_knife_m1 may use the addon combo keys.
static void* g_knife_combo_owner = 0;
static unsigned g_knife_combo_last_ms = 0;
static unsigned g_knife_combo_index = 0;
static unsigned g_knife_combo_logged = 0;
static void* g_knife_return_owner = 0;
static unsigned g_knife_return_start_ms = 0;
static unsigned g_knife_return_duration = 0;
static unsigned g_knife_return_index = 0;
static unsigned g_knife_return_logged = 0;
static unsigned g_knife_sound_errors = 0;
static unsigned g_knife_sound_missing = 0;
static int g_knife_sound_busy = 0;

static bool knife_combo_is_m1(void* hud_item)
{
    void* attached = hudrc_item();
    if (!attached || *(void**)((char*)attached + OFF_AHI_PARENT_ITEM) != hud_item)
        return false;
    const char* value = *(const char**)((char*)hud_item - OFF_HUD_TO_COBJECT +
                                       OFF_COBJECT_SECTION);
    return value && strcmp(value + STRV_VALUE, "wpn_knife_m1") == 0;
}

static bool knife_combo_has_key(const char* key)
{
    void* ini = *(void**)(g_base + RVA_PSETTINGS);
    return ini && ((ini_line_exist_t)(g_base + RVA_INI_LINE_EXIST))(
        ini, "wpn_knife_m1_hud", key) != 0;
}

// The knife's native class does not play its configured draw/holster sounds.
// Delegate just those two events to the optional Lua module after motion start.
static void knife_notify_motion(int event)
{
    if (g_knife_sound_busy) return;
    void* L = lua_state();
    if (!L) return;
    g_knife_sound_busy = 1;
    const int top = ((lua_gettop_t)(g_base + RVA_LUA_GETTOP))(L);
    ((lua_getfield_t)(g_base + RVA_LUA_GETFIELD))(
        L, LUA_GLOBALSINDEX, "fl_on_knife_motion");
    if (((lua_type_t)(g_base + RVA_LUA_TYPE))(L, -1) == LUA_TFUNCTION) {
        ((lua_pushinteger_t)(g_base + RVA_LUA_PUSHINTEGER))(L, event);
        if (((pcall_t)(g_base + RVA_PCALL))(L, 1, 0, 0) &&
            g_knife_sound_errors++ < 3u) {
            const char* error = ((lua_tolstring_t)(g_base + RVA_LUA_TOLSTRING))(L, -1, 0);
            ((msg_t)(g_base + RVA_MSG))("!! [knife_sound] %s", error ? error : "?");
        }
    } else if (g_knife_sound_missing++ == 0u) {
        ((msg_t)(g_base + RVA_MSG))("! [knife_sound] Lua callback unavailable");
    }
    ((lua_settop2_t)(g_base + RVA_LUA_SETTOP2))(L, top);
    g_knife_sound_busy = 0;
}

static unsigned hkHudPlayMotion(void* self, const char* key, bool mix_in,
                                unsigned state, bool random, float speed)
{
    if (!g_orig_hud_playmotion) return 0;
    if (!self || !key || !knife_combo_is_m1(self))
        return g_orig_hud_playmotion(self, key, mix_in, state, random, speed);

    if (strcmp(key, "anm_hide") == 0 || strcmp(key, "anm_hide_fast") == 0) {
        g_knife_combo_owner = 0;
        g_knife_return_owner = 0;
        const unsigned duration = g_orig_hud_playmotion(
            self, key, mix_in, state, random, speed);
        if (duration && strcmp(key, "anm_hide") == 0) knife_notify_motion(2);
        return duration;
    }
    if (strcmp(key, "anm_show") == 0 || strcmp(key, "anm_show_empty") == 0 ||
        strcmp(key, "anm_show_fast") == 0) {
        g_knife_return_owner = 0;
        const unsigned duration = g_orig_hud_playmotion(
            self, key, mix_in, state, random, speed);
        if (duration && strcmp(key, "anm_show_fast") != 0) knife_notify_motion(1);
        return duration;
    }

    const bool primary = strcmp(key, "anm_attack") == 0;
    const bool secondary = strcmp(key, "anm_attack2") == 0;
    if (!primary && !secondary) {
        if ((strcmp(key, "anm_idle") == 0 || strcmp(key, "anm_idle_aim") == 0) &&
            g_knife_return_owner == self) {
            static const char* return_keys[3] = {
                "anm_hit12idle", "anm_hit22idle", "anm_hit32idle"
            };
            const unsigned elapsed = hudrc_time_ms() - g_knife_return_start_ms;
            const unsigned threshold = g_knife_return_duration > 80u
                ? g_knife_return_duration - 80u : 0u;
            const unsigned index = g_knife_return_index;
            g_knife_return_owner = 0;
            if (elapsed >= threshold &&
                elapsed <= g_knife_return_duration + 400u &&
                index < 3u && knife_combo_has_key(return_keys[index])) {
                const unsigned duration = g_orig_hud_playmotion(
                    self, return_keys[index], mix_in, state, random, speed);
                if (duration) {
                    if (g_knife_return_logged++ < 12u)
                        ((msg_t)(g_base + RVA_MSG))(
                            "~ [knife_combo] %s -> %s (%u ms)",
                            key, return_keys[index], duration);
                    return duration;
                }
            }
        } else {
            g_knife_return_owner = 0;
        }
        return g_orig_hud_playmotion(self, key, mix_in, state, random, speed);
    }

    static const char* primary_keys[4] = {
        "anm_attack", "anm_attack_svariant1", "anm_attack_svariant2", "anm_attack_svariant3"
    };
    static const char* secondary_keys[4] = {
        "anm_attack2", "anm_attack2_svariant1", "anm_attack2_svariant2", "anm_attack2_svariant3"
    };
    const unsigned now = hudrc_time_ms();
    const unsigned next = (g_knife_combo_owner == self &&
                           (unsigned)(now - g_knife_combo_last_ms) <= 1000u)
                              ? (g_knife_combo_index + 1u) % 4u : 0u;
    const char* chosen = (primary ? primary_keys : secondary_keys)[next];
    if (next != 0u && !knife_combo_has_key(chosen))
        chosen = key;

    g_knife_return_owner = 0;
    const unsigned duration = g_orig_hud_playmotion(
        self, chosen, mix_in, state, random, speed);
    if (duration) {
        g_knife_combo_owner = self;
        g_knife_combo_last_ms = now;
        g_knife_combo_index = chosen == key ? 0u : next;
        g_knife_return_owner = primary && g_knife_combo_index < 3u ? self : 0;
        g_knife_return_start_ms = now;
        g_knife_return_duration = duration;
        g_knife_return_index = g_knife_combo_index;
        if (g_knife_combo_logged++ < 12u)
            ((msg_t)(g_base + RVA_MSG))("~ [knife_combo] %s -> %s (%u ms)",
                                         key, chosen, duration);
    }
    return duration;
}

static void hudrc_reset()
{
    if (g_hudrc_on) ++g_hudrc_clears;
    g_hudrc_on = 0;
    for (int i = 0; i < 3; ++i) { g_hudrc_pos[i] = 0.f; g_hudrc_rot[i] = 0.f; }
    g_hudrc_owner = 0;
}

static int hudrc_set6(float px, float py, float pz,
                      float pitch, float yaw, float roll)
{
    float in[6] = {px, py, pz, pitch, yaw, roll};
    for (int i = 0; i < 6; ++i)
        if (!hudrc_finite(in[i])) {
            ++g_hudrc_rej_bad;
            hudrc_reset();
            return 0;
        }
    void* it = hudrc_item();
    if (!it) {
        ++g_hudrc_rej_nohud;
        hudrc_reset();
        return 0;
    }
    for (int i = 0; i < 3; ++i) {
        g_hudrc_pos[i] = hudrc_clamp(in[i],     HUDRC_MAX_POS);
        g_hudrc_rot[i] = hudrc_clamp(in[3 + i], HUDRC_MAX_ROT);
    }
    g_hudrc_owner = *(void**)((char*)it + OFF_AHI_PARENT_ITEM);
    g_hudrc_stamp = hudrc_time_ms();
    g_hudrc_on    = 1;
    return 1;
}

static int hudrc_set(float pos_y, float pitch, float yaw)
{
    return hudrc_set6(0.f, pos_y, 0.f, pitch, yaw, 0.f);
}

static void hudrc_mul(float* o, const float* a, const float* b)
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            float s = 0.f;
            for (int k = 0; k < 4; ++k) s += a[i * 4 + k] * b[k * 4 + j];
            o[i * 4 + j] = s;
        }
}

static void hudrc_build(float* r)
{
    float cp = cosf(g_hudrc_rot[0]), sp = sinf(g_hudrc_rot[0]);
    float cy = cosf(g_hudrc_rot[1]), sy = sinf(g_hudrc_rot[1]);
    float cr = cosf(g_hudrc_rot[2]), sr = sinf(g_hudrc_rot[2]);
    float rx[16] = { 1.f, 0.f, 0.f, 0.f,   0.f,  cp,  sp, 0.f,
                     0.f, -sp,  cp, 0.f,   0.f, 0.f, 0.f, 1.f };
    float ry[16] = {  cy, 0.f, -sy, 0.f,   0.f, 1.f, 0.f, 0.f,
                      sy, 0.f,  cy, 0.f,   0.f, 0.f, 0.f, 1.f };
    float rz[16] = {  cr,  sr, 0.f, 0.f,   -sr,  cr, 0.f, 0.f,
                     0.f, 0.f, 1.f, 0.f,   0.f, 0.f, 0.f, 1.f };
    float t[16];
    hudrc_mul(t, rx, ry);
    hudrc_mul(r, t, rz);
    r[12] = g_hudrc_pos[0];
    r[13] = g_hudrc_pos[1];
    r[14] = g_hudrc_pos[2];
    r[15] = 1.f;
}

static int hudrc_apply()
{
    g_hudrc_saved_at = 0;
    if (!g_hudrc_on) return 0;
    if (!g_hudrc_lock && (unsigned)(hudrc_time_ms() - g_hudrc_stamp) > HUDRC_WATCHDOG) {
        ++g_hudrc_expired;
        hudrc_reset();
        return 0;
    }
    void* it = hudrc_item();
    if (!it) {
        ++g_hudrc_rej_nohud;
        hudrc_reset();
        return 0;
    }
    if (*(void**)((char*)it + OFF_AHI_PARENT_ITEM) != g_hudrc_owner) {
        ++g_hudrc_rej_item;
        hudrc_reset();
        return 0;
    }
    float* m = (float*)((char*)it + OFF_AHI_ITEM_XFORM);
    for (int i = 0; i < 16; ++i) g_hudrc_saved[i] = m[i];

    float r[16], o[16];
    hudrc_build(r);
    hudrc_mul(o, r, g_hudrc_saved);
    for (int i = 0; i < 16; ++i) m[i] = o[i];

    g_hudrc_saved_at = m;
    ++g_hudrc_frames;
    return 1;
}

static void hudrc_restore()
{
    if (!g_hudrc_saved_at) return;
    for (int i = 0; i < 16; ++i) g_hudrc_saved_at[i] = g_hudrc_saved[i];
    g_hudrc_saved_at = 0;
}

static int lua_fl_hud_recoil_available(void* L)
{
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, hudrc_item() ? 1 : 0);
    return 1;
}

static int lua_fl_hud_recoil_set(void* L)
{
    if (g_hudrc_lock) {
        ++g_hudrc_rej_lock;
        ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 0);
        return 1;
    }
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    float y = (float)tonum(L, 1);
    float p = (float)tonum(L, 2);
    float w = (float)tonum(L, 3);
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, hudrc_set(y, p, w));
    return 1;
}

static int lua_fl_hud_recoil_set6(void* L)
{
    if (g_hudrc_lock) {
        ++g_hudrc_rej_lock;
        ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 0);
        return 1;
    }
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    float v[6];
    for (int i = 0; i < 6; ++i) v[i] = (float)tonum(L, i + 1);
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(
        L, hudrc_set6(v[0], v[1], v[2], v[3], v[4], v[5]));
    return 1;
}

static int lua_fl_hud_recoil_clear(void* L)
{
    if (!g_hudrc_lock) hudrc_reset();
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 1);
    return 1;
}

static int texture_exists(const char* name)
{
    void* fs = locator_fs();
    if (!fs || !name || !name[0]) return 0;

    char rel[520];
    int i = 0;
    for (; name[i] && i < (int)sizeof(rel) - 6; ++i)
        rel[i] = (name[i] == '/') ? '\\' : name[i];
    rel[i] = 0;
    if (i < 4 || rel[i - 4] != '.') {
        rel[i++] = '.'; rel[i++] = 'd'; rel[i++] = 'd'; rel[i++] = 's'; rel[i] = 0;
    }

    r_open_t  ropen  = (r_open_t)(g_base + RVA_R_OPEN);
    r_close_t rclose = (r_close_t)(g_base + RVA_R_CLOSE);
    IReader* r = ropen(fs, "$game_textures$", rel);
    if (!r) return 0;
    rclose(fs, &r);
    return 1;
}

static int tex_swap(const char* reg, const char* file)
{
    if (!reg || !reg[0] || !file || !file[0]) return 0;
    if (!texture_exists(reg) || !texture_exists(file)) return 0;

    void* rdev = ((rdev_instance_t)(g_base + RVA_RDEV_INSTANCE))();
    if (!rdev) return 0;
    void* res = *(void**)((uintptr_t)rdev + OFS_RDEV_RESOURCES);
    if (!res) return 0;

    void* tex = ((create_texture_t)(g_base + RVA_CREATE_TEXTURE))(res, reg);
    if (!tex) return 0;
    ((tex_unload_t)(g_base + RVA_TEX_UNLOAD))(tex);
    ((tex_load_t)(g_base + RVA_TEX_LOAD))(tex, file);
    return 1;
}

static int lua_fl_tex_swap(void* L)
{
    lua_tolstring_t tostr = (lua_tolstring_t)(g_base + RVA_LUA_TOLSTRING);
    const char* reg  = tostr(L, 1, 0);
    const char* file = tostr(L, 2, 0);
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, tex_swap(reg, file) ? 1 : 0);
    return 1;
}

static void cmd_tex(const char* args)
{
    char reg[260], file[260];
    int n = 0, i = 0;
    while (*args == ' ' || *args == '\t') ++args;
    while (args[i] && args[i] != ' ' && args[i] != '\t' && n < (int)sizeof(reg) - 1)
        reg[n++] = args[i++];
    reg[n] = 0;
    while (args[i] == ' ' || args[i] == '\t') ++i;
    n = 0;
    while (args[i] && args[i] != ' ' && args[i] != '\t' && n < (int)sizeof(file) - 1)
        file[n++] = args[i++];
    file[n] = 0;
    if (!reg[0]) {
        log_msg("~ [tex] usage: tex <texture> [file]   (file omitted = restore)");
        return;
    }
    const char* target = file[0] ? file : reg;
    if (tex_swap(reg, target))
        ((msg_t)(g_base + RVA_MSG))("~ [tex] [%s] <- [%s]", reg, target);
    else
        ((msg_t)(g_base + RVA_MSG))("!! [tex] failed: [%s] <- [%s]", reg, target);
}

static int lua_fl_step_material(void* L)
{
    const char* ground = 0;
    const char* self   = 0;
    int gid = 0;
    if (!step_material_read(&ground, &self, &gid)) return 0;
    pushstr_t pushstr = (pushstr_t)(g_base + RVA_PUSHSTR);
    pushstr(L, ground);
    ((lua_pushinteger_t)(g_base + RVA_LUA_PUSHINTEGER))(L, gid);
    pushstr(L, self ? self : "");
    return 3;
}

static void* g_pda3d_rt_holder = 0;

typedef void (*rt_create_t)(void* holder, const char* name,
    unsigned w, unsigned h, unsigned fmt, unsigned sample, unsigned flags);

static int vtbl_in_module(void* obj, const wchar_t* mod)
{
    if (!obj) return 0;
    void* vt = *(void**)obj;
    if (!vt) return 0;
    char* base = (char*)GetModuleHandleW(mod);
    if (!base) return 0;
    unsigned off = *(unsigned*)(base + 0x3C);
    unsigned size = *(unsigned*)(base + off + 4 + 20 + 56);
    return (char*)vt >= base && (char*)vt < base + size;
}

typedef void* (*getfont_t)(void);
typedef void (*font_out_t)(void* self, float x, float y, const char* fmt, ...);

typedef long (__stdcall *d3dcompile_t)(const void*, unsigned long long,
    const char*, const void*, void*, const char*, const char*,
    unsigned, unsigned, void**, void**);

struct M2Vert { float x, y, c[4]; };

static void* g_m2_vs = 0;
static void* g_m2_ps = 0;
static void* g_m2_layout = 0;
static void* g_m2_vb = 0;
static void* g_m2_cb = 0;

static const char M2_VS[] =
    "cbuffer CB : register(b0) { float4 so; };"
    "struct VI { float2 p : POSITION; float4 c : COLOR; };"
    "struct VO { float4 p : SV_POSITION; float4 c : COLOR; };"
    "VO vs(VI i) { VO o; o.p = float4(i.p * so.xy + so.zw, 0, 1); o.c = i.c; return o; }";
static const char M2_PS[] =
    "struct VO { float4 p : SV_POSITION; float4 c : COLOR; };"
    "float4 ps(VO i) : SV_TARGET { return i.c; }";

static int m2b_setup(void* dev)
{
    if (g_m2_vs && g_m2_ps && g_m2_layout && g_m2_vb && g_m2_cb) return 1;
    char sysdir[260];
    unsigned n = GetSystemDirectoryA(sysdir, 260);
    (void)n;
    char dllp[300];
    dllp[0] = 0;
    const char* dlls[] = { "d3dcompiler_47.dll", "d3dcompiler_43.dll", 0 };
    void* hdc = 0;
    for (int i = 0; dlls[i] && !hdc; ++i) hdc = LoadLibraryA(dlls[i]);
    if (!hdc) return 0;
    d3dcompile_t comp = (d3dcompile_t)GetProcAddress((HMODULE)hdc, "D3DCompile");
    msg_t Dm = (msg_t)(g_base + RVA_MSG);
    if (!comp) { Dm("!! [pda3d] m2b: no D3DCompile export"); return 0; }
    ID3D11Device* d = (ID3D11Device*)dev;
    void* vsb = 0; void* psb = 0; void* err = 0;
    long hr;
    hr = comp(M2_VS, sizeof(M2_VS), 0, 0, 0, "vs", "vs_4_0", 0, 0, &vsb, &err);
    if (hr) {
        const char* et = err ? (const char*)((ID3D10Blob*)err)->GetBufferPointer() : 0;
        Dm("!! [pda3d] m2b: VS compile hr=0x%X %s", (unsigned)hr, et ? et : "(no blob)");
        return 0;
    }
    err = 0;
    hr = comp(M2_PS, sizeof(M2_PS), 0, 0, 0, "ps", "ps_4_0", 0, 0, &psb, &err);
    if (hr) {
        const char* et = err ? (const char*)((ID3D10Blob*)err)->GetBufferPointer() : 0;
        Dm("!! [pda3d] m2b: PS compile hr=0x%X %s", (unsigned)hr, et ? et : "(no blob)");
        return 0;
    }
    int hrvs = d->CreateVertexShader(((ID3D10Blob*)vsb)->GetBufferPointer(),
        ((ID3D10Blob*)vsb)->GetBufferSize(), 0, (ID3D11VertexShader**)&g_m2_vs);
    if (hrvs) { Dm("!! [pda3d] m2b: CreateVS hr=0x%X", (unsigned)hrvs); return 0; }
    int hrps = d->CreatePixelShader(((ID3D10Blob*)psb)->GetBufferPointer(),
        ((ID3D10Blob*)psb)->GetBufferSize(), 0, (ID3D11PixelShader**)&g_m2_ps);
    if (hrps) { Dm("!! [pda3d] m2b: CreatePS hr=0x%X", (unsigned)hrps); return 0; }
    struct IE { const char* n; unsigned idx; unsigned fmt; unsigned slot; unsigned off; unsigned cls; unsigned step; };
    static const IE elems[2] = {
        { "POSITION", 0, 16, 0, 0, 0, 0 },
        { "COLOR", 0, 2, 0, 8, 0, 0 },
    };
    int hrl = d->CreateInputLayout((const D3D11_INPUT_ELEMENT_DESC*)elems, 2,
        ((ID3D10Blob*)vsb)->GetBufferPointer(), ((ID3D10Blob*)vsb)->GetBufferSize(),
        (ID3D11InputLayout**)&g_m2_layout);
    if (hrl) { Dm("!! [pda3d] m2b: CreateLayout hr=0x%X", (unsigned)hrl); return 0; }
    ((ID3D10Blob*)vsb)->Release();
    ((ID3D10Blob*)psb)->Release();
    D3D11_BUFFER_DESC bd;
    bd.ByteWidth = sizeof(M2Vert) * 256; bd.Usage = (D3D11_USAGE)1; bd.BindFlags = 1;
    bd.CPUAccessFlags = 0x10000; bd.MiscFlags = 0; bd.StructureByteStride = 0;
    int hrv = d->CreateBuffer(&bd, 0, (ID3D11Buffer**)&g_m2_vb);
    if (hrv) { Dm("!! [pda3d] m2b: CreateVB hr=0x%X", (unsigned)hrv); return 0; }
    D3D11_BUFFER_DESC cd;
    cd.ByteWidth = 16; cd.Usage = (D3D11_USAGE)1; cd.BindFlags = 4;
    cd.CPUAccessFlags = 0x10000; cd.MiscFlags = 0; cd.StructureByteStride = 0;
    int hrc = d->CreateBuffer(&cd, 0, (ID3D11Buffer**)&g_m2_cb);
    if (hrc) { Dm("!! [pda3d] m2b: CreateCB hr=0x%X", (unsigned)hrc); return 0; }
    return 1;
}

static int g_m2_nv = 0;
static M2Vert g_m2_v[256];

static void m2b_rect(float x0, float y0, float x1, float y1,
    float r, float g, float b, float a)
{
    if (g_m2_nv + 6 > 256) return;
    float q[6][2] = { {x0,y0}, {x1,y0}, {x1,y1}, {x0,y0}, {x1,y1}, {x0,y1} };
    for (int i = 0; i < 6; ++i) {
        M2Vert* v = &g_m2_v[g_m2_nv++];
        v->x = q[i][0]; v->y = q[i][1];
        v->c[0] = r; v->c[1] = g; v->c[2] = b; v->c[3] = a;
    }
}

static void cmd_pda3d_m2b(msg_t Msg)
{
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    if (!rtv || !vtbl_in_module(rtv, L"d3d11.dll")) { Msg("!! [pda3d] m2b: no RT (run m1a first)"); return; }
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv)->GetDevice(&dev);
    ID3D11DeviceContext* ctx = 0;
    if (dev) dev->GetImmediateContext(&ctx);
    if (!ctx) { Msg("!! [pda3d] m2b: no context"); return; }
    if (!m2b_setup(dev)) { Msg("!! [pda3d] m2b: shader setup failed"); return; }
    ID3D11RenderTargetView* oldRTV = 0;
    ID3D11DepthStencilView* oldDSV = 0;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
    unsigned oldNV = 0;
    ctx->RSGetViewports(&oldNV, 0);
    D3D11_VIEWPORT oldVP[8];
    if (oldNV > 8) oldNV = 8;
    if (oldNV) ctx->RSGetViewports(&oldNV, oldVP);
    ID3D11RenderTargetView* mine = (ID3D11RenderTargetView*)rtv;
    ctx->OMSetRenderTargets(1, &mine, 0);
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0; vp.TopLeftY = 0; vp.Width = 512; vp.Height = 512;
    vp.MinDepth = 0; vp.MaxDepth = 1;
    ctx->RSSetViewports(1, &vp);
    float c[4] = { 0.03f, 0.03f, 0.05f, 1.0f };
    ctx->ClearRenderTargetView(mine, c);
    g_m2_nv = 0;
    m2b_rect(20, 20, 492, 90, 0.1f, 0.5f, 0.9f, 1.0f);
    m2b_rect(20, 110, 492, 200, 0.9f, 0.8f, 0.1f, 1.0f);
    for (int i = 0; i < 8; ++i)
        m2b_rect(20 + i * 45, 230, 55 + i * 45, 265, 0.2f, 0.9f, 0.2f, 1.0f);
    m2b_rect(20, 290, 492, 480, 0.7f, 0.1f, 0.1f, 1.0f);
    D3D11_MAPPED_SUBRESOURCE mp;
    if (!ctx->Map((ID3D11Resource*)g_m2_vb, 0, (D3D11_MAP)4, 0, &mp)) {
        memcpy(mp.pData, g_m2_v, sizeof(M2Vert) * g_m2_nv);
        ctx->Unmap((ID3D11Resource*)g_m2_vb, 0);
        float cb[4] = { 2.0f / 512.0f, -2.0f / 512.0f, -1.0f, 1.0f };
        D3D11_MAPPED_SUBRESOURCE mc;
        if (!ctx->Map((ID3D11Resource*)g_m2_cb, 0, (D3D11_MAP)4, 0, &mc)) {
            memcpy(mc.pData, cb, 16);
            ctx->Unmap((ID3D11Resource*)g_m2_cb, 0);
            ctx->IASetInputLayout((ID3D11InputLayout*)g_m2_layout);
            unsigned stride = sizeof(M2Vert), off = 0;
            ID3D11Buffer* vb = (ID3D11Buffer*)g_m2_vb;
            ctx->IASetVertexBuffers(0, 1, &vb, &stride, &off);
            ctx->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)4);
            ctx->VSSetShader((ID3D11VertexShader*)g_m2_vs, 0, 0);
            ID3D11Buffer* cb1 = (ID3D11Buffer*)g_m2_cb;
            ctx->VSSetConstantBuffers(0, 1, &cb1);
            ctx->PSSetShader((ID3D11PixelShader*)g_m2_ps, 0, 0);
            ctx->Draw(g_m2_nv, 0);
        }
    }
    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    if (oldNV) ctx->RSSetViewports(oldNV, oldVP);
    if (oldRTV) oldRTV->Release();
    if (oldDSV) oldDSV->Release();
    Msg("~ [pda3d] m2b: %d verts drawn, state restored", g_m2_nv);
}

static void cmd_pda3d_m2(msg_t Msg)
{
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    if (!rtv || !vtbl_in_module(rtv, L"d3d11.dll")) { Msg("!! [pda3d] m2: no RT (run m1a first)"); return; }
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv)->GetDevice(&dev);
    ID3D11DeviceContext* ctx = 0;
    if (dev) dev->GetImmediateContext(&ctx);
    if (!ctx) { Msg("!! [pda3d] m2: no context"); return; }
    ID3D11RenderTargetView* oldRTV = 0;
    ID3D11DepthStencilView* oldDSV = 0;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
    unsigned oldNV = 0;
    ctx->RSGetViewports(&oldNV, 0);
    D3D11_VIEWPORT oldVP[8];
    if (oldNV > 8) oldNV = 8;
    if (oldNV) ctx->RSGetViewports(&oldNV, oldVP);
    unsigned oldNS = 0;
    ctx->RSGetScissorRects(&oldNS, 0);
    D3D11_RECT oldSR[8];
    if (oldNS > 8) oldNS = 8;
    if (oldNS) ctx->RSGetScissorRects(&oldNS, oldSR);
    ID3D11RenderTargetView* mine = (ID3D11RenderTargetView*)rtv;
    ctx->OMSetRenderTargets(1, &mine, 0);
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0; vp.TopLeftY = 0; vp.Width = 512; vp.Height = 512;
    vp.MinDepth = 0; vp.MaxDepth = 1;
    ctx->RSSetViewports(1, &vp);
    D3D11_RECT sr = { 0, 0, 512, 512 };
    ctx->RSSetScissorRects(1, &sr);
    float c[4] = { 0.05f, 0.05f, 0.08f, 1.0f };
    ctx->ClearRenderTargetView(mine, c);
    getfont_t gf = (getfont_t)(g_base + RVA_GETFONT_GRAFFITI);
    void* font = gf();
    Msg("~ [pda3d] m2: font=%p", font);
    if (font) {
        font_out_t out = (font_out_t)(g_base + RVA_FONT_OUT);
        out(font, 40.0f, 60.0f, "NPC PDA 3D-RT M2");
        out(font, 40.0f, 110.0f, "PAROL: ****");
        out(font, 40.0f, 160.0f, "TAYNIK: KORDON");
        out(font, 40.0f, 210.0f, "0123456789");
    }
    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    if (oldNV) ctx->RSSetViewports(oldNV, oldVP);
    if (oldNS) ctx->RSSetScissorRects(oldNS, oldSR);
    if (oldRTV) oldRTV->Release();
    if (oldDSV) oldDSV->Release();
    Msg("~ [pda3d] m2: text drawn, state restored");
}

static void cmd_pda3d_m1a(msg_t Msg)
{
    rt_create_t f = (rt_create_t)(g_base + RVA_RT_CREATE);
    f(&g_pda3d_rt_holder, "$user$pda3d", 512, 512, 28, 1, 0);
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    Msg("~ [pda3d] m1a: holder=%p crt=%p pRT=%p", g_pda3d_rt_holder, crt, rtv);
}

static void cmd_pda3d_m1b(msg_t Msg)
{
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    Msg("~ [pda3d] m1b: crt=%p pRT=%p d3d11vt=%d", crt, rtv,
        vtbl_in_module(rtv, L"d3d11.dll"));
    if (!rtv || !vtbl_in_module(rtv, L"d3d11.dll")) return;
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv)->GetDevice(&dev);
    Msg("~ [pda3d] m1b: dev=%p", dev);
}

static void cmd_pda3d_m1b2(msg_t Msg)
{
    unsigned char* base = (unsigned char*)(uintptr_t)g_base;
    Msg("~ [pda3d] m1b2: g_base=%p", base);
    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQuery(base, &mbi, sizeof(mbi))) { Msg("!! [pda3d] m1b2: VQ base failed"); return; }
    Msg("~ [pda3d] m1b2: base region base=%p size=0x%x state=0x%x prot=0x%x",
        mbi.AllocationBase, (unsigned)mbi.RegionSize, (unsigned)mbi.State, (unsigned)mbi.Protect);
    char mz[2] = { 0, 0 };
    SIZE_T done = 0;
    if (ReadProcessMemory(GetCurrentProcess(), base, mz, 2, &done) && done == 2)
        Msg("~ [pda3d] m1b2: MZ bytes: %02X %02X", (unsigned char)mz[0], (unsigned char)mz[1]);
    else
        Msg("!! [pda3d] m1b2: cannot read [g_base]");
    void* hw = (void*)(g_base + RVA_HW);
    if (!VirtualQuery(hw, &mbi, sizeof(mbi))) { Msg("!! [pda3d] m1b2: VQ hw failed"); return; }
    Msg("~ [pda3d] m1b2: hw=%p region base=%p size=0x%x state=0x%x prot=0x%x",
        hw, mbi.AllocationBase, (unsigned)mbi.RegionSize, (unsigned)mbi.State, (unsigned)mbi.Protect);
}

static void cmd_pda3d_m1c(msg_t Msg)
{
    void* crt0 = g_pda3d_rt_holder;
    void* rtv0 = crt0 ? *(void**)((char*)crt0 + OFF_CRT_PRT) : 0;
    if (!rtv0 || !vtbl_in_module(rtv0, L"d3d11.dll")) { Msg("!! [pda3d] m1c: bad RTV"); return; }
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv0)->GetDevice(&dev);
    if (!dev) { Msg("!! [pda3d] m1c: no D3D device"); return; }
    ID3D11DeviceContext* ctx = 0;
    dev->GetImmediateContext(&ctx);
    Msg("~ [pda3d] m1c: dev=%p ctx=%p", dev, ctx);
}

static void cmd_pda3d_m1(msg_t Msg)
{
    cmd_pda3d_m1a(Msg);
    cmd_pda3d_m1b(Msg);
    cmd_pda3d_m1c(Msg);
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    if (!crt || !rtv) { Msg("!! [pda3d] m1: RT not ready"); return; }
    if (!vtbl_in_module(rtv, L"d3d11.dll")) { Msg("!! [pda3d] m1: bad RTV"); return; }
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv)->GetDevice(&dev);
    ID3D11DeviceContext* ctx = 0;
    if (dev) dev->GetImmediateContext(&ctx);
    if (!ctx) { Msg("!! [pda3d] m1: no immediate context"); return; }
    ID3D11RenderTargetView* oldRTV = 0;
    ID3D11DepthStencilView* oldDSV = 0;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
    ID3D11RenderTargetView* mine = (ID3D11RenderTargetView*)rtv;
    ctx->OMSetRenderTargets(1, &mine, 0);
    float c[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
    ctx->ClearRenderTargetView(mine, c);
    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    if (oldRTV) oldRTV->Release();
    if (oldDSV) oldDSV->Release();
    Msg("~ [pda3d] m1: cleared magenta, state restored");
}

static void cmd_pda3d_m1_OLD(msg_t Msg)
{
    rt_create_t f = (rt_create_t)(g_base + RVA_RT_CREATE);
    f(&g_pda3d_rt_holder, "$user$pda3d", 512, 512, 28, 1, 0);
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    Msg("~ [pda3d] m1: holder=%p crt=%p pRT=%p", g_pda3d_rt_holder, crt, rtv);
    if (!crt || !rtv) { Msg("!! [pda3d] m1: RT not ready"); return; }
    void* hw = (void*)(g_base + RVA_HW);
    ID3D11Device* dev = *(ID3D11Device**)((char*)hw + OFF_HW_PDEVICE);
    if (!dev) { Msg("!! [pda3d] m1: no D3D device"); return; }
    ID3D11DeviceContext* ctx = 0;
    dev->GetImmediateContext(&ctx);
    if (!ctx) { Msg("!! [pda3d] m1: no immediate context"); return; }
    ID3D11RenderTargetView* oldRTV = 0;
    ID3D11DepthStencilView* oldDSV = 0;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
    ID3D11RenderTargetView* mine = (ID3D11RenderTargetView*)rtv;
    ctx->OMSetRenderTargets(1, &mine, 0);
    float c[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
    ctx->ClearRenderTargetView(mine, c);
    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    if (oldRTV) oldRTV->Release();
    if (oldDSV) oldDSV->Release();
    Msg("~ [pda3d] m1: cleared magenta, state restored");
}

static int lua_fl_pda3d_available(void* L);
static int lua_fl_pda3d_set_active(void* L);
static int lua_fl_pda3d_clear(void* L);
static int lua_fl_pda3d_text(void* L);
static int lua_fl_pda3d_hack(void* L);
static int lua_fl_pda3d_nosign(void* L);
static int lua_fl_pda3d_freeze(void* L);
static const unsigned char kNotesLoadPrologue[16] = {
    0x40, 0x55, 0x53, 0x48, 0x8D, 0xAC, 0x24, 0x58,
    0xF9, 0xFF, 0xFF, 0x48, 0x81, 0xEC, 0xA8, 0x07
};
static int      g_notes_verified = 0;
static int      g_notes_failed   = 0;
static int      g_notes_pushes   = 0;
static uint64_t g_notes_dummy   = 0;
typedef void (*strtable_load_t)(void* self, const char* xml_file);

static int notes_verify()
{
    if (g_notes_verified) return 1;
    if (!g_base) return 0;
    uint8_t* f = (uint8_t*)(g_base + RVA_STR_TABLE_LOAD);
    for (int i = 0; i < 16; ++i)
        if (f[i] != kNotesLoadPrologue[i]) { g_notes_failed = 1; return 0; }
    for (int i = 0; i < 0x400 - 7; ++i) {
        if ((f[i] == 0x48 || f[i] == 0x4C) && (f[i+1] == 0x8B || f[i+1] == 0x8D) &&
            ((f[i+2] & 0xC7) == 0x05)) {
            int32_t d = *(int32_t*)(f + i + 3);
            if (f + i + 7 + d == (uint8_t*)(g_base + RVA_STR_TABLE_ANCHOR)) {
                g_notes_verified = 1;
                return 1;
            }
        }
    }
    g_notes_failed = 1;
    return 0;
}

static int      g_notes_seh = 0;
static int      g_notes_test_ok = 0;

static void*       g_notes_rec = 0;
static void*       g_notes_rsp = 0;
static volatile int g_notes_call_ok = 0;

static LONG WINAPI notes_veh(EXCEPTION_POINTERS* ep)
{
    if (g_notes_rec &&
        ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        ++g_notes_seh;
        ep->ContextRecord->Rsp = (DWORD64)g_notes_rsp;
        ep->ContextRecord->Rip = (DWORD64)g_notes_rec;
        ep->ContextRecord->Rax = 0;
        g_notes_rec = 0;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static int      g_notes_depth = 0;

static int notes_load_file(const char* name)
{
    (void)name;
    if (!notes_verify()) return 0;
    return 0;
}

static int notes_push()
{
    return notes_load_file("he_notes_runtime");
}

static int lua_fl_notes_available(void* L)
{
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 1);
    return 1;
}

static int lua_fl_notes_push(void* L)
{
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, notes_push() ? 1 : 0);
    return 1;
}

static void* g_lua_api_state = 0;
static void ensure_lua_api()
{
    void* L = lua_state();
    if (!L || L == g_lua_api_state) return;
    lua_pushcclosure_t pushc = (lua_pushcclosure_t)(g_base + RVA_LUA_PUSHCCLOSURE);
    lua_setfield_t     setf  = (lua_setfield_t)(g_base + RVA_LUA_SETFIELD);
    pushc(L, &lua_fl_step_material, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_step_material");
    pushc(L, &lua_fl_hud_recoil_available, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_hud_recoil_available");
    pushc(L, &lua_fl_hud_recoil_set, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_hud_recoil_set");
    pushc(L, &lua_fl_hud_recoil_set6, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_hud_recoil_set6");
    pushc(L, &lua_fl_hud_recoil_clear, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_hud_recoil_clear");
    pushc(L, &lua_fl_tex_swap, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_tex_swap");
    pushc(L, &lua_fl_pda3d_available, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_available");
    pushc(L, &lua_fl_pda3d_set_active, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_set_active");
    pushc(L, &lua_fl_pda3d_clear, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_clear");
    pushc(L, &lua_fl_pda3d_text, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_text");
    pushc(L, &lua_fl_pda3d_hack, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_hack");
    pushc(L, &lua_fl_pda3d_nosign, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_nosign");
    pushc(L, &lua_fl_pda3d_freeze, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_pda3d_freeze");
    pushc(L, &lua_fl_notes_available, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_notes_available");
    pushc(L, &lua_fl_notes_push, 0);
    setf(L, LUA_GLOBALSINDEX, "fl_notes_push");
    hudrc_reset();
    g_lua_api_state = L;
}

static int      g_pda3d_rt_arm = 0;
static unsigned g_pda3d_pda_id = 0;
static int      g_pda3d_opens  = 0;
static int      g_pda3d_clears = 0;

static int lua_fl_pda3d_available(void* L)
{
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 1);
    return 1;
}

static int lua_fl_pda3d_set_active(void* L)
{
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    unsigned id = (unsigned)tonum(L, 1);
    if (!id) {
        ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 0);
        return 1;
    }
    g_pda3d_pda_id = id;
    g_pda3d_rt_arm = 1;
    ++g_pda3d_opens;
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 1);
    return 1;
}

static int lua_fl_pda3d_clear(void* L)
{
    g_pda3d_rt_arm = 0;
    g_pda3d_pda_id = 0;
    ++g_pda3d_clears;
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, 1);
    return 1;
}

static char g_pda3d_lines[14][80];
static int  g_pda3d_nlines   = 0;
static int  g_pda3d_rt_ready = 0;
static int  g_pda3d_draws    = 0;
static int  g_pda3d_draw_err = 0;

static const unsigned PDA3D_W = 512;
static const unsigned PDA3D_H = 512;

static int pda3d_ensure_rt(void)
{
    if (g_pda3d_rt_ready && g_pda3d_rt_holder) return 1;
    rt_create_t f = (rt_create_t)(g_base + RVA_RT_CREATE);
    f(&g_pda3d_rt_holder, "$user$pda3d", PDA3D_W, PDA3D_H, 28, 1, 0);
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    if (!rtv || !vtbl_in_module(rtv, L"d3d11.dll")) return 0;
    g_pda3d_rt_ready = 1;
    return 1;
}

static void m2b_draw_verts(ID3D11DeviceContext* ctx)
{
    D3D11_MAPPED_SUBRESOURCE mp;
    if (ctx->Map((ID3D11Resource*)g_m2_vb, 0, (D3D11_MAP)4, 0, &mp)) return;
    memcpy(mp.pData, g_m2_v, sizeof(M2Vert) * g_m2_nv);
    ctx->Unmap((ID3D11Resource*)g_m2_vb, 0);
    float cb[4] = { 2.0f / (float)PDA3D_W, -2.0f / (float)PDA3D_H, -1.0f, 1.0f };
    D3D11_MAPPED_SUBRESOURCE mc;
    if (ctx->Map((ID3D11Resource*)g_m2_cb, 0, (D3D11_MAP)4, 0, &mc)) return;
    memcpy(mc.pData, cb, 16);
    ctx->Unmap((ID3D11Resource*)g_m2_cb, 0);
    ctx->IASetInputLayout((ID3D11InputLayout*)g_m2_layout);
    unsigned stride = sizeof(M2Vert), off = 0;
    ID3D11Buffer* vb = (ID3D11Buffer*)g_m2_vb;
    ctx->IASetVertexBuffers(0, 1, &vb, &stride, &off);
    ctx->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)4);
    ctx->VSSetShader((ID3D11VertexShader*)g_m2_vs, 0, 0);
    ID3D11Buffer* cb1 = (ID3D11Buffer*)g_m2_cb;
    ctx->VSSetConstantBuffers(0, 1, &cb1);
    ctx->PSSetShader((ID3D11PixelShader*)g_m2_ps, 0, 0);
    ctx->Draw(g_m2_nv, 0);
}

static void pda3d_render(void)
{
    if (!g_pda3d_rt_arm) return;
    if (!pda3d_ensure_rt()) { ++g_pda3d_draw_err; return; }
    void* crt = g_pda3d_rt_holder;
    void* rtv = crt ? *(void**)((char*)crt + OFF_CRT_PRT) : 0;
    if (!rtv || !vtbl_in_module(rtv, L"d3d11.dll")) { ++g_pda3d_draw_err; return; }
    ID3D11Device* dev = 0;
    ((ID3D11RenderTargetView*)rtv)->GetDevice(&dev);
    ID3D11DeviceContext* ctx = 0;
    if (dev) dev->GetImmediateContext(&ctx);
    if (!ctx) { ++g_pda3d_draw_err; return; }
    if (!m2b_setup(dev)) { ctx->Release(); ++g_pda3d_draw_err; return; }

    ID3D11RenderTargetView* oldRTV = 0;
    ID3D11DepthStencilView* oldDSV = 0;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);
    unsigned oldNV = 0;
    ctx->RSGetViewports(&oldNV, 0);
    D3D11_VIEWPORT oldVP[8];
    if (oldNV > 8) oldNV = 8;
    if (oldNV) ctx->RSGetViewports(&oldNV, oldVP);
    unsigned oldNS = 0;
    ctx->RSGetScissorRects(&oldNS, 0);
    D3D11_RECT oldSR[8];
    if (oldNS > 8) oldNS = 8;
    if (oldNS) ctx->RSGetScissorRects(&oldNS, oldSR);

    ID3D11RenderTargetView* mine = (ID3D11RenderTargetView*)rtv;
    ctx->OMSetRenderTargets(1, &mine, 0);
    D3D11_VIEWPORT vp;
    vp.TopLeftX = 0; vp.TopLeftY = 0;
    vp.Width = (float)PDA3D_W; vp.Height = (float)PDA3D_H;
    vp.MinDepth = 0; vp.MaxDepth = 1;
    ctx->RSSetViewports(1, &vp);
    D3D11_RECT sr = { 0, 0, (LONG)PDA3D_W, (LONG)PDA3D_H };
    ctx->RSSetScissorRects(1, &sr);
    float c[4] = { 0.05f, 0.08f, 0.05f, 1.0f };
    ctx->ClearRenderTargetView(mine, c);

    g_m2_nv = 0;
    m2b_rect(6, 6, 506, 506, 0.09f, 0.30f, 0.11f, 1.0f);
    m2b_rect(6, 6, 506, 54, 0.14f, 0.52f, 0.18f, 1.0f);
    m2b_rect(14, 62, 498, 500, 0.02f, 0.05f, 0.03f, 1.0f);
    for (int i = 0; i < g_pda3d_nlines; ++i)
        m2b_rect(20, 74.0f + i * 30.0f, 492, 98.0f + i * 30.0f, 0.03f, 0.07f, 0.04f, 1.0f);
    m2b_draw_verts(ctx);

    getfont_t gf = (getfont_t)(g_base + RVA_GETFONT_GRAFFITI);
    void* font = gf();
    if (font) {
        font_out_t out = (font_out_t)(g_base + RVA_FONT_OUT);
        out(font, 22.0f, 16.0f, "PDA NPC [3D]");
        for (int i = 0; i < g_pda3d_nlines; ++i)
            out(font, 30.0f, 84.0f + i * 30.0f, "%s", g_pda3d_lines[i]);
    }
    ++g_pda3d_draws;

    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    if (oldNV) ctx->RSSetViewports(oldNV, oldVP);
    if (oldNS) ctx->RSSetScissorRects(oldNS, oldSR);
    if (oldRTV) oldRTV->Release();
    if (oldDSV) oldDSV->Release();
    ctx->Release();
}

static int lua_fl_pda3d_text(void* L)
{
    lua_tolstring_t tostr = (lua_tolstring_t)(g_base + RVA_LUA_TOLSTRING);
    lua_pushboolean_t pb = (lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN);
    size_t n = 0;
    const char* s = tostr(L, 1, &n);
    g_pda3d_nlines = 0;
    if (s && n) {
        const char* p = s;
        while (*p && g_pda3d_nlines < 14) {
            int k = 0;
            while (*p && *p != '\n' && k < 79) g_pda3d_lines[g_pda3d_nlines][k++] = *p++;
            g_pda3d_lines[g_pda3d_nlines][k] = 0;
            ++g_pda3d_nlines;
            if (*p == '\n') ++p;
        }
    }
    pb(L, 1);
    return 1;
}

#define RVA_CUIPDAWND_VFT    0x00DFE8F8ULL
#define RVA_CUIPDAWND_DRAW   0x0041B2B0ULL
#define RVA_CUIWINDOW_DRAW   0x004167A0ULL
#define OFF_VFT_STOPANYMOVE  0x170
#define OFF_VFT_DRAW         0x18
#define RVA_PDA3D_VP2_SITE   0x0032D519ULL
#define RVA_LAST_FRAME_2     0x01265804ULL
#define RVA_DEVICE_FRAME     0x01223774ULL
#define RVA_G_HUD            0x011FACB8ULL
#define OFF_HUD_VFT_GETUI    0x30
#define OFF_HOLDER_RECV      0x18
#define OFF_RECV_ITEM        0x10
#define OFF_WND_SHOWME       0x08
#define RVA_HUD_RENDERUI     0x00380A10ULL
#define RVA_HUD_RENDERUI_END 0x00380A25ULL

typedef void  (*uiwnd_draw_t)(void*);
typedef void  (*hud_renderui_t)(void*);
typedef void* (*hud_getui_t)(void*);

static int            g_pda3d_hack        = 0;
static void*          g_pda3d_hack_wnd    = 0;
static unsigned       g_pda3d_hack_frame  = 0;
static int            g_pda3d_hack_draws  = 0;
static int            g_pda3d_hack_hides  = 0;
static int            g_pda3d_hack_miss   = 0;
static int            g_pda3d_hack_bound  = 0;
static int            g_pda3d_hide_2d     = 1;
static int            g_pda3d_nosign      = 0;
static int            g_pda3d_nosign_draws = 0;
static uiwnd_draw_t   g_orig_pdawnd_draw  = 0;
static hud_renderui_t g_orig_hud_renderui = 0;

static void* pda3d_top_receiver(void)
{
    void* hud = *(void**)(g_base + RVA_G_HUD);
    if (!hud) return 0;
    void** hvft = *(void***)hud;
    if (!hvft) return 0;
    hud_getui_t getui = (hud_getui_t)hvft[OFF_HUD_VFT_GETUI / 8];
    if (!getui) return 0;
    void* ui = getui(hud);
    if (!ui) return 0;
    unsigned char* first = *(unsigned char**)((unsigned char*)ui + OFF_HOLDER_RECV);
    unsigned char* last  = *(unsigned char**)((unsigned char*)ui + OFF_HOLDER_RECV + 8);
    if (!first || !last || last <= first) return 0;
    return *(void**)(last - OFF_RECV_ITEM);
}

static FL_NOINLINE void hkCUIPdaWndDraw(void* self)
{
    if (g_pda3d_nosign
        && (uintptr_t)FL_RETURN_ADDRESS() == g_base + RVA_PDA3D_VP2_SITE) {
        *(unsigned*)(g_base + RVA_LAST_FRAME_2) = *(unsigned*)(g_base + RVA_DEVICE_FRAME);
        ++g_pda3d_nosign_draws;
        ((uiwnd_draw_t)(g_base + RVA_CUIWINDOW_DRAW))(self);
        return;
    }
    if (g_pda3d_hack
        && (uintptr_t)FL_RETURN_ADDRESS() == g_base + RVA_PDA3D_VP2_SITE) {
        void* wnd = pda3d_top_receiver();
        if (wnd && wnd != self) {
            unsigned frame = *(unsigned*)(g_base + RVA_DEVICE_FRAME);
            void** wvft = *(void***)wnd;
            *(unsigned*)(g_base + RVA_LAST_FRAME_2) = frame;
            g_pda3d_hack_wnd   = wnd;
            g_pda3d_hack_frame = frame;
            ++g_pda3d_hack_draws;
            ((uiwnd_draw_t)wvft[OFF_VFT_DRAW / 8])(wnd);
            return;
        }
        ++g_pda3d_hack_miss;
    }
    if (g_orig_pdawnd_draw) g_orig_pdawnd_draw(self);
}

static void hkHudRenderUI(void* self)
{
    unsigned char* wnd = 0;
    unsigned char saved = 0;
    if (g_pda3d_hack && g_pda3d_hide_2d && g_pda3d_hack_wnd
        && g_pda3d_hack_frame == *(unsigned*)(g_base + RVA_DEVICE_FRAME)
        && pda3d_top_receiver() == g_pda3d_hack_wnd) {
        wnd = (unsigned char*)g_pda3d_hack_wnd;
        saved = wnd[OFF_WND_SHOWME];
        wnd[OFF_WND_SHOWME] = 0;
        ++g_pda3d_hack_hides;
    }
    if (g_orig_hud_renderui) g_orig_hud_renderui(self);
    if (wnd) wnd[OFF_WND_SHOWME] = saved;
}

static int pda3d_hack_install(void)
{
    uint64_t* slot = (uint64_t*)(g_base + RVA_CUIPDAWND_VFT + OFF_VFT_DRAW);
    DWORD oldp;
    if (!VirtualProtect(slot, 8, PAGE_READWRITE, &oldp)) return 0;
    if (*slot != (uint64_t)(g_base + RVA_CUIPDAWND_DRAW)) {
        VirtualProtect(slot, 8, oldp, &oldp);
        return 0;
    }
    g_orig_pdawnd_draw = (uiwnd_draw_t)*slot;
    *slot = (uint64_t)&hkCUIPdaWndDraw;
    VirtualProtect(slot, 8, oldp, &oldp);
    g_pda3d_hack_bound = 1;
    return 1;
}

static void pda3d_hack_set(int on)
{
    g_pda3d_hack = on ? 1 : 0;
    if (!g_pda3d_hack) { g_pda3d_hack_wnd = 0; g_pda3d_hack_frame = 0; }
}

static uint64_t g_pda3d_stopmove_orig = 0;
static int      g_pda3d_freeze        = 0;

static char pda3d_stop_any_move(void* self) { (void)self; return 1; }

static void pda3d_freeze_set(int on)
{
    uint64_t* slot = (uint64_t*)(g_base + RVA_CUIPDAWND_VFT + OFF_VFT_STOPANYMOVE);
    DWORD oldp;
    if (!VirtualProtect(slot, 8, PAGE_READWRITE, &oldp)) return;
    if (on) {
        if (!g_pda3d_stopmove_orig) g_pda3d_stopmove_orig = *slot;
        *slot = (uint64_t)&pda3d_stop_any_move;
        g_pda3d_freeze = 1;
    } else if (g_pda3d_stopmove_orig) {
        *slot = g_pda3d_stopmove_orig;
        g_pda3d_freeze = 0;
    }
    VirtualProtect(slot, 8, oldp, &oldp);
}

static int lua_fl_pda3d_freeze(void* L)
{
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    pda3d_freeze_set((int)tonum(L, 1) ? 1 : 0);
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, g_pda3d_freeze);
    return 1;
}

static int lua_fl_pda3d_nosign(void* L)
{
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    g_pda3d_nosign = (int)tonum(L, 1) ? 1 : 0;
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, g_pda3d_hack_bound);
    return 1;
}

static int lua_fl_pda3d_hack(void* L)
{
    lua_tonumber_t tonum = (lua_tonumber_t)(g_base + RVA_LUA_TONUMBER);
    pda3d_hack_set((int)tonum(L, 1));
    ((lua_pushboolean_t)(g_base + RVA_LUA_PUSHBOOLEAN))(L, g_pda3d_hack_bound);
    return 1;
}

#define RVA_PSACTORFLAGS     0x010AC028ULL
#define RVA_ACTOR_ALLOW_PDA  0x010ABB44ULL
#define OFF_ACTOR_INVENTORY  0x418
#define OFF_INV_SLOTS        0x60
#define OFF_INV_ACTIVESLOT   0x88
#define SZ_INV_SLOT          0x20
#define OFF_SLOT_ITEM        0x08
#define OFF_PDA_IS3D         0x838
#define OFF_CUI_PUIGAME      0x48
#define OFF_SP_INVMENU       0x70
#define OFF_SP_PDAMENU       0x78

static void cmd_pda3d_why(msg_t Msg)
{
    unsigned flags = *(unsigned*)(g_base + RVA_PSACTORFLAGS);
    unsigned char allow = *(unsigned char*)(g_base + RVA_ACTOR_ALLOW_PDA);
    void* actor = *(void**)(g_base + RVA_G_ACTOR);
    Msg("~ [pda3d why] psActorFlags=0x%X g_3d_pda=%d g_actor_allow_pda=%d actor=%p",
        flags, (flags & 0x10000) ? 1 : 0, (int)allow, actor);
    if (!actor) return;

    void* inv = *(void**)((unsigned char*)actor + OFF_ACTOR_INVENTORY);
    if (!inv) { Msg("!! [pda3d why] no inventory"); return; }
    unsigned char* slots = *(unsigned char**)((unsigned char*)inv + OFF_INV_SLOTS);
    unsigned active = *(unsigned*)((unsigned char*)inv + OFF_INV_ACTIVESLOT);
    if (!slots) { Msg("!! [pda3d why] no slots, active=%u", active); return; }

    void* item = *(void**)(slots + 7 * SZ_INV_SLOT + OFF_SLOT_ITEM);
    int is3d = item ? (int)*((unsigned char*)item + OFF_PDA_IS3D) : -1;
    Msg("~ [pda3d why] active_slot=%u slot7_item=%p this_is_3d_pda=%d", active, item, is3d);

    void* hud = *(void**)(g_base + RVA_G_HUD);
    if (!hud) { Msg("!! [pda3d why] no g_hud"); return; }
    void** hvft = *(void***)hud;
    hud_getui_t getui = (hud_getui_t)hvft[OFF_HUD_VFT_GETUI / 8];
    void* ui = getui(hud);
    if (!ui) { Msg("!! [pda3d why] no CUI"); return; }
    void* sp = *(void**)((unsigned char*)ui + OFF_CUI_PUIGAME);
    if (!sp) { Msg("!! [pda3d why] no pUIGame"); return; }
    void* pda_wnd = *(void**)((unsigned char*)sp + OFF_SP_PDAMENU);
    void* inv_wnd = *(void**)((unsigned char*)sp + OFF_SP_INVMENU);
    Msg("~ [pda3d why] PdaMenu=%p show=%d  InventoryMenu=%p show=%d  top_recv=%p",
        pda_wnd, pda_wnd ? (int)*((unsigned char*)pda_wnd + OFF_WND_SHOWME) : -1,
        inv_wnd, inv_wnd ? (int)*((unsigned char*)inv_wnd + OFF_WND_SHOWME) : -1,
        pda3d_top_receiver());

    int gate = (item && is3d > 0 && (flags & 0x10000)) ? 1 : 0;
    Msg("~ [pda3d why] 3d_gate=%d -> %s", gate,
        gate ? "CInventory::Action should Activate(7)"
             : "engine takes the flat 2D path (CUIGameSP opens PdaMenu)");
}

#define RVA_INV_ACTION       0x00385A60ULL
#define RVA_INV_ACTION_END   0x00385A6FULL
#define RVA_INV_ACTIVATE     0x00385820ULL
#define PDA_SLOT_IDX         7
#define EACT_KEYACTION       1

typedef char (*inv_action_t)(void*, int, unsigned);
typedef char (*inv_activate_t)(void*, unsigned, int, char, char);

static inv_action_t g_orig_inv_action = 0;
static int          g_pda3d_trace     = 0;

static void* pda3d_actor_inventory(void)
{
    void* actor = *(void**)(g_base + RVA_G_ACTOR);
    if (!actor) return 0;
    return *(void**)((unsigned char*)actor + OFF_ACTOR_INVENTORY);
}

static char hkInvAction(void* self, int cmd, unsigned flags)
{
    if (!g_pda3d_trace) return g_orig_inv_action ? g_orig_inv_action(self, cmd, flags) : 0;
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    unsigned before = *(unsigned*)((unsigned char*)self + OFF_INV_ACTIVESLOT);
    char r = g_orig_inv_action ? g_orig_inv_action(self, cmd, flags) : 0;
    unsigned after = *(unsigned*)((unsigned char*)self + OFF_INV_ACTIVESLOT);
    Msg("~ [pda3d trace] Action cmd=%d flags=0x%X slot %u -> %u ret=%d%s",
        cmd, flags, before, after, (int)r,
        (self == pda3d_actor_inventory()) ? " (actor)" : "");
    return r;
}

static void cmd_pda3d_pull(msg_t Msg)
{
    void* inv = pda3d_actor_inventory();
    if (!inv) { Msg("!! [pda3d pull] no inventory"); return; }
    unsigned before = *(unsigned*)((unsigned char*)inv + OFF_INV_ACTIVESLOT);
    inv_activate_t act = (inv_activate_t)(g_base + RVA_INV_ACTIVATE);
    char r = act(inv, PDA_SLOT_IDX, EACT_KEYACTION, 0, 0);
    unsigned after = *(unsigned*)((unsigned char*)inv + OFF_INV_ACTIVESLOT);
    Msg("~ [pda3d pull] Activate(7, eKeyAction) ret=%d slot %u -> %u",
        (int)r, before, after);
}

#define RVA_SET_MAIN_IR      0x003CACD0ULL
#define RVA_SET_MAIN_IR_END  0x003CACE1ULL

typedef void (*set_main_ir_t)(void*, void*, char, unsigned char);

static set_main_ir_t g_orig_set_main_ir = 0;
static int           g_pda3d_ir_blocks  = 0;

static void* pda3d_pda_menu(void)
{
    void* hud = *(void**)(g_base + RVA_G_HUD);
    if (!hud) return 0;
    void** hvft = *(void***)hud;
    if (!hvft) return 0;
    hud_getui_t getui = (hud_getui_t)hvft[OFF_HUD_VFT_GETUI / 8];
    if (!getui) return 0;
    void* ui = getui(hud);
    if (!ui) return 0;
    void* sp = *(void**)((unsigned char*)ui + OFF_CUI_PUIGAME);
    if (!sp) return 0;
    return *(void**)((unsigned char*)sp + OFF_SP_PDAMENU);
}

static void hkSetMainInputReceiver(void* self, void* wnd, char no_focus, unsigned char flags)
{
    if (g_pda3d_hack && wnd && wnd == pda3d_pda_menu()) {
        ++g_pda3d_ir_blocks;
        return;
    }
    if (g_orig_set_main_ir) g_orig_set_main_ir(self, wnd, no_focus, flags);
}

static void cmd_pda3d_text(msg_t Msg, const char* s)
{
    while (*s == ' ' || *s == '\t') ++s;
    g_pda3d_nlines = 0;
    const char* p = s;
    while (*p && g_pda3d_nlines < 14) {
        int k = 0;
        while (*p && *p != '\n' && k < 79) g_pda3d_lines[g_pda3d_nlines][k++] = *p++;
        g_pda3d_lines[g_pda3d_nlines][k] = 0;
        ++g_pda3d_nlines;
        if (*p == '\n') ++p;
    }
    Msg("~ [pda3d] text: %d line(s)", g_pda3d_nlines);
}
static void cmd_pda3d(const char* a)
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    while (*a == ' ' || *a == '\t') ++a;
    if (a[0] == 'r' && a[1] == 't') {
        const char* s = a + 2;
        while (*s == ' ' || *s == '\t') ++s;
        if (*s == '1') g_pda3d_rt_arm = 1;
        else if (*s == '0') { g_pda3d_rt_arm = 0; g_pda3d_pda_id = 0; }
        Msg("~ [pda3d] rt_arm=%d pda_id=%u", g_pda3d_rt_arm, g_pda3d_pda_id);
        return;
    }
    if (a[0] == 'm' && a[1] == '1' && a[2] == 'a') { cmd_pda3d_m1a(Msg); return; }
    if (a[0] == 'm' && a[1] == '2' && a[2] == 'b') { cmd_pda3d_m2b(Msg); return; }
    if (a[0] == 'm' && a[1] == '2') { cmd_pda3d_m2(Msg); return; }
    if (a[0] == 'm' && a[1] == '1' && a[2] == 'b' && a[3] == '2') { cmd_pda3d_m1b2(Msg); return; }
    if (a[0] == 'm' && a[1] == '1' && a[2] == 'b') { cmd_pda3d_m1b(Msg); return; }
    if (a[0] == 'm' && a[1] == '1' && a[2] == 'c') { cmd_pda3d_m1c(Msg); return; }
    if (a[0] == 'm' && a[1] == '1') { cmd_pda3d_m1(Msg); return; }
    if (a[0] == 't' && a[1] == 'e' && a[2] == 'x' && a[3] == 't') { cmd_pda3d_text(Msg, a + 4); return; }
    if (a[0] == 'f' && a[1] == 'i' && a[2] == 'l' && a[3] == 'l') { pda3d_render(); Msg("~ [pda3d] fill: draws=%d err=%d", g_pda3d_draws, g_pda3d_draw_err); return; }
    if (a[0] == 't' && a[1] == 'r' && a[2] == 'a' && a[3] == 'c') {
        const char* s = a + 5;
        while (*s == ' ') ++s;
        if (*s == '1') g_pda3d_trace = 1;
        else if (*s == '0') g_pda3d_trace = 0;
        Msg("~ [pda3d] trace=%d hooked=%d", g_pda3d_trace, g_orig_inv_action ? 1 : 0);
        return;
    }
    if (a[0] == 'p' && a[1] == 'u' && a[2] == 'l' && a[3] == 'l') { cmd_pda3d_pull(Msg); return; }
    if (a[0] == 'h' && a[1] == 'i' && a[2] == 'd' && a[3] == 'e') {
        const char* s = a + 4;
        while (*s == ' ') ++s;
        if (*s == '1') g_pda3d_hide_2d = 1;
        else if (*s == '0') g_pda3d_hide_2d = 0;
        Msg("~ [pda3d] hide_2d=%d", g_pda3d_hide_2d);
        return;
    }
    if (a[0] == 'f' && a[1] == 'r' && a[2] == 'e' && a[3] == 'e') {
        const char* s = a + 6;
        while (*s == ' ') ++s;
        if (*s == '1') pda3d_freeze_set(1);
        else if (*s == '0') pda3d_freeze_set(0);
        Msg("~ [pda3d] freeze=%d orig=%p", g_pda3d_freeze, (void*)g_pda3d_stopmove_orig);
        return;
    }
    if (a[0] == 's' && a[1] == 'i' && a[2] == 'g' && a[3] == 'n') {
        const char* s = a + 4;
        while (*s == ' ') ++s;
        if (*s == '1') g_pda3d_nosign = 1;
        else if (*s == '0') g_pda3d_nosign = 0;
        Msg("~ [pda3d] nosign=%d draws=%d", g_pda3d_nosign, g_pda3d_nosign_draws);
        return;
    }
    if (a[0] == 'w' && a[1] == 'h' && a[2] == 'y') { cmd_pda3d_why(Msg); return; }
    if (a[0] == 'h' && a[1] == 'a' && a[2] == 'c' && a[3] == 'k') {
        const char* s = a + 4;
        while (*s == ' ' || *s == '\t') ++s;
        if (*s == '1') pda3d_hack_set(1);
        else if (*s == '0') pda3d_hack_set(0);
        Msg("~ [pda3d] hack=%d bound=%d draws=%d hides=%d miss=%d",
            g_pda3d_hack, g_pda3d_hack_bound, g_pda3d_hack_draws,
            g_pda3d_hack_hides, g_pda3d_hack_miss);
        return;
    }
    if (a[0] == 'o' && a[1] == 'n') { g_pda3d_rt_arm = 1; Msg("~ [pda3d] on"); return; }
    if (a[0] == 'o' && a[1] == 'f' && a[2] == 'f') { g_pda3d_rt_arm = 0; Msg("~ [pda3d] off"); return; }
    Msg("~ [pda3d] rt_arm=%d pda_id=%u opens=%d clears=%d bound=%d",
        g_pda3d_rt_arm, g_pda3d_pda_id, g_pda3d_opens, g_pda3d_clears,
        g_lua_api_state ? 1 : 0);
    Msg("~ [pda3d] phase=2 rt=%d lines=%d draws=%d",
        g_pda3d_rt_ready, g_pda3d_nlines, g_pda3d_draws);
    Msg("~ [pda3d] hack=%d bound=%d wnd=%p hides=%d",
        g_pda3d_hack, g_pda3d_hack_bound, g_pda3d_hack_wnd, g_pda3d_hack_hides);
    Msg("~ [pda3d] ir_guard=%d ir_blocks=%d",
        g_orig_set_main_ir ? 1 : 0, g_pda3d_ir_blocks);
}

#define WBF_NAME "fl_on_actor_weapon_before_fire"

static int g_wbf_calls   = 0;
static int g_wbf_no_vm   = 0;
static int g_wbf_no_cb   = 0;
static int g_wbf_err     = 0;
static int g_wbf_skip_np = 0;
static int g_wbf_skip_mf = 0;
static int g_wbf_busy    = 0;
static int g_wbf_last_gl = -1;

static void wbf_call(int is_gl)
{
    if (g_wbf_busy) return;
    void* L = lua_state();
    if (!L) { ++g_wbf_no_vm; return; }

    g_wbf_busy = 1;
    int top = ((lua_gettop_t)(g_base + RVA_LUA_GETTOP))(L);
    ((lua_getfield_t)(g_base + RVA_LUA_GETFIELD))(L, LUA_GLOBALSINDEX, WBF_NAME);
    if (((lua_type_t)(g_base + RVA_LUA_TYPE))(L, -1) != LUA_TFUNCTION) {
        ++g_wbf_no_cb;
    } else {
        ((lua_pushinteger_t)(g_base + RVA_LUA_PUSHINTEGER))(L, is_gl);
        if (((pcall_t)(g_base + RVA_PCALL))(L, 1, 0, 0)) {
            if (++g_wbf_err <= 3) {
                const char* e = ((lua_tolstring_t)(g_base + RVA_LUA_TOLSTRING))(L, -1, 0);
                ((msg_t)(g_base + RVA_MSG))("!! [wbf] %s: %s", WBF_NAME, e ? e : "?");
            }
        } else {
            ++g_wbf_calls;
        }
    }
    ((lua_settop2_t)(g_base + RVA_LUA_SETTOP2))(L, top);
    g_wbf_last_gl = is_gl;
    g_wbf_busy = 0;
}

static int wbf_is_actor_weapon(void* wpn)
{
    if (!wpn || !g_base) return 0;
    void* actor = *(void**)(g_base + RVA_G_ACTOR);
    if (!actor) return 0;
    return *(void**)((char*)wpn + OFF_WPN_PARENT) == actor;
}

static void hkFireTrace(void* self, const float* P, const float* D)
{
    if (wbf_is_actor_weapon(self)) {
        if (*(unsigned char*)((char*)self + OFF_WPN_ADDON_FLAGS) & 0x20)
            ++g_wbf_skip_mf;
        else
            wbf_call(0);
    } else {
        ++g_wbf_skip_np;
    }
    if (g_orig_fire_trace)
        g_orig_fire_trace(self, P, D);
}

static void hkGrenadeOnShot(void* self)
{
    if (self && *(unsigned char*)((char*)self + OFF_WPN_GRENADE_MODE) &&
        wbf_is_actor_weapon(self))
        wbf_call(1);
    if (g_orig_gl_onshot)
        g_orig_gl_onshot(self);
}

static void ensure_slash(char* path, int cap)
{
    int n = 0;
    while (path[n]) ++n;
    if (n == 0 || n + 1 >= cap) return;
    char last = path[n - 1];
    if (last == '\\' || last == '/') return;
    path[n] = '\\';
    path[n + 1] = 0;
}

static int basename_mod(const char* path_or_name, char* out, int outcap)
{
    if (!path_or_name || !*path_or_name || outcap < 2) return 0;
    const char* last = path_or_name;
    for (const char* p = path_or_name; *p; ++p)
        if (*p == '/' || *p == '\\') last = p + 1;
    int n = 0;
    for (const char* p = last; *p && *p != '.' && n + 1 < outcap; ++p) {
        char c = *p;
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'))
            break;
        out[n++] = c;
    }
    out[n] = 0;
    return n > 0;
}

static int has_script_ext(const char* name)
{
    if (!name) return 0;
    size_t n = 0;
    while (name[n]) ++n;
    if (n < 7) return 0;
    const char* e = name + n - 7;
    return (e[0]=='.') &&
           (e[1]=='s'||e[1]=='S') && (e[2]=='c'||e[2]=='C') &&
           (e[3]=='r'||e[3]=='R') && (e[4]=='i'||e[4]=='I') &&
           (e[5]=='p'||e[5]=='P') && (e[6]=='t'||e[6]=='T');
}

static int is_dot_dir(const char* name)
{
    return name && (name[0]=='.') && (name[1]==0 || (name[1]=='.' && name[2]==0));
}

static void skip_bom(const char** pdata, size_t* psize)
{
    const unsigned char* d = (const unsigned char*)*pdata;
    size_t n = *psize;
    if (n >= 3 && d[0] == 0xEF && d[1] == 0xBB && d[2] == 0xBF) {
        *pdata = (const char*)(d + 3);
        *psize = n - 3;
    }
}

static int reload_script_file_safe(void* L, void* fs, const char* full_path, const char* mod)
{
    r_open_t  ropen  = (r_open_t)(g_base + RVA_R_OPEN);
    r_close_t rclose = (r_close_t)(g_base + RVA_R_CLOSE);
    loadbuf_t loadbuf = (loadbuf_t)(g_base + RVA_LOADBUF);
    pcall_t   pcall   = (pcall_t)(g_base + RVA_PCALL);
    printout_t printout = (printout_t)(g_base + RVA_PRINTOUT);

    IReader* r = ropen(fs, 0, full_path);
    if (!r || !r->data) {
        log_fmt1("!! [rl] cannot open [%s]", full_path);
        return 0;
    }

    const char* body = r->data + r->Pos;
    size_t body_sz = r->Size - r->Pos;
    skip_bom(&body, &body_sz);

    size_t pre  = sizeof(WRAP_PRE) - 1;
    size_t mid  = sizeof(WRAP_MID) - 1;
    size_t post = sizeof(WRAP_POST) - 1;
    size_t mlen = 0;
    while (mod[mlen]) ++mlen;
    size_t total = pre + mlen + mid + mlen + post + body_sz;

    char* buf = (char*)HeapAlloc(GetProcessHeap(), 0, total + 1);
    if (!buf) {
        rclose(fs, &r);
        log_msg("!! [rl] OOM");
        return 0;
    }

    size_t o = 0;
    for (size_t i = 0; i < pre; ++i) buf[o++] = WRAP_PRE[i];
    for (size_t i = 0; i < mlen; ++i) buf[o++] = mod[i];
    for (size_t i = 0; i < mid; ++i) buf[o++] = WRAP_MID[i];
    for (size_t i = 0; i < mlen; ++i) buf[o++] = mod[i];
    for (size_t i = 0; i < post; ++i) buf[o++] = WRAP_POST[i];
    for (size_t i = 0; i < body_sz; ++i) buf[o++] = body[i];
    buf[o] = 0;

    char cname[300];
    cname[0] = '@';
    int ci = 1;
    for (const char* p = full_path; *p && ci + 1 < (int)sizeof(cname); ++p)
        cname[ci++] = *p;
    cname[ci] = 0;

    int rc = loadbuf(L, buf, o, cname, 0);
    rclose(fs, &r);

    if (rc) {
        printout(L, cname, rc);
        HeapFree(GetProcessHeap(), 0, buf);
        return 0;
    }
    rc = pcall(L, 0, 0, 0);
    HeapFree(GetProcessHeap(), 0, buf);
    if (rc) {
        printout(L, cname, rc);
        return 0;
    }
    return 1;
}

#define RL_MAX_MODS 48

struct WantMods {
    char name[RL_MAX_MODS][64];
    char found[RL_MAX_MODS];
    int  n;
};

static int streq_ci(const char* a, const char* b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb - 'A' + 'a');
        if (ca != cb) return 0;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

static int want_index(WantMods* w, const char* mod)
{
    for (int i = 0; i < w->n; ++i)
        if (streq_ci(w->name[i], mod)) return i;
    return -1;
}

static void walk_reload_wanted(void* L, void* fs, const char* path, WantMods* want, int* ok_n, int* fail_n)
{
    file_list_open_t  open  = (file_list_open_t)(g_base + RVA_FILE_LIST_OPEN);
    file_list_close_t close = (file_list_close_t)(g_base + RVA_FILE_LIST_CLOSE);

    void* vfiles = open(fs, path, FS_ListFiles);
    if (vfiles) {
        VecP* v = (VecP*)vfiles;
        if (v->first && v->last) {
            for (char** it = v->first; it != v->last; ++it) {
                const char* name = *it;
                if (!name || !has_script_ext(name)) continue;
                char mod[260];
                if (!basename_mod(name, mod, (int)sizeof(mod))) continue;
                int wi = want_index(want, mod);
                if (wi < 0) continue;

                char full[560];
                int i = 0;
                for (const char* p = path; *p && i + 1 < (int)sizeof(full); ++p)
                    full[i++] = *p;
                for (const char* p = name; *p && i + 1 < (int)sizeof(full); ++p)
                    full[i++] = *p;
                full[i] = 0;

                log_fmt1("~ [rl] reload [%s]", full);
                if (reload_script_file_safe(L, fs, full, mod)) {
                    want->found[wi] = 1;
                    ++(*ok_n);
                } else {
                    ++(*fail_n);
                }
            }
        }
        close(fs, &vfiles);
    }

    void* vdirs = open(fs, path, FS_ListFolders);
    if (vdirs) {
        VecP* v = (VecP*)vdirs;
        if (v->first && v->last) {
            for (char** it = v->first; it != v->last; ++it) {
                const char* name = *it;
                if (!name || is_dot_dir(name)) continue;
                char sub[520];
                int i = 0;
                for (const char* p = path; *p && i + 1 < (int)sizeof(sub); ++p)
                    sub[i++] = *p;
                for (const char* p = name; *p && i + 1 < (int)sizeof(sub); ++p)
                    sub[i++] = *p;
                sub[i] = 0;
                ensure_slash(sub, (int)sizeof(sub));
                walk_reload_wanted(L, fs, sub, want, ok_n, fail_n);
            }
        }
        close(fs, &vdirs);
    }
}

static void cmd_rl_scripts(const char* args)
{
    while (*args == ' ' || *args == '\t') ++args;
    if (!*args) {
        log_msg("~ [rl] usage: rl scripts <mod> [mod2 ...]");
        log_msg("~ [rl] example: rl scripts thirst");
        log_msg("~ [rl] example: rl scripts thirst nlc_corpse_decay kotovod");
        log_msg("~ [rl] NOTE: full dump of all scripts is unsafe mid-game (breaks db/schemes)");
        return;
    }

    WantMods want;
    want.n = 0;
    while (*args && want.n < RL_MAX_MODS) {
        while (*args == ' ' || *args == '\t') ++args;
        if (!*args) break;
        char raw[128];
        int i = 0;
        while (*args && *args != ' ' && *args != '\t' && i + 1 < (int)sizeof(raw))
            raw[i++] = *args++;
        raw[i] = 0;
        if (!basename_mod(raw, want.name[want.n], (int)sizeof(want.name[0])))
            continue;
        want.found[want.n] = 0;
        ++want.n;
    }
    if (want.n == 0) {
        log_msg("!! [rl] no valid module names");
        return;
    }

    void* L  = lua_state();
    void* fs = locator_fs();
    if (!L || !fs) {
        log_msg("!! [rl] no lua / FS (need loaded game)");
        return;
    }
    char root[520];
    update_path_t up = (update_path_t)(g_base + RVA_UPDATE_PATH);
    up(fs, root, "$game_scripts$", "");
    ensure_slash(root, (int)sizeof(root));

    log_fmt1i("~ [rl] reloading %d module(s)", want.n);
    int ok_n = 0, fail_n = 0;
    walk_reload_wanted(L, fs, root, &want, &ok_n, &fail_n);

    for (int i = 0; i < want.n; ++i) {
        if (!want.found[i]) {
            log_fmt1("!! [rl] not found: [%s].script", want.name[i]);
            ++fail_n;
        }
    }
    log_fmt2i("~ [rl] scripts done: ok=%d fail=%d", ok_n, fail_n);
}

static void cmd_rl_ui()
{
    if (g_orig && g_console) {
        g_orig(g_console, "ui_reload", 0, 1);
        log_msg("~ [rl] ui_reload");
    } else {
        log_msg("!! [rl] ui_reload skipped (no console)");
    }
}

static void cmd_rl_configs()
{
    reload_ini_t fn = (reload_ini_t)(g_base + RVA_RELOADINI);
    void* p = fn();
    if (p)
        log_msg("~ [rl] reload_system_ini OK (pSettings rebuilt)");
    else
        log_msg("!! [rl] reload_system_ini failed");
    log_msg("~ [rl] tip: UI XML -> rl ui ; close trade before reload if open");
}

static void cmd_rl_all()
{
    log_msg("~ [rl] all = configs only (no scripts bulk, no ui)");
    cmd_rl_configs();
}

static void cmd_rl_help()
{
    log_msg("~ [rl] usage:");
    log_msg("~ [rl]   rl scripts <mod> [mod2 ...]  - re-exec named .script modules");
    log_msg("~ [rl]   rl configs                   - reload_system_ini only");
    log_msg("~ [rl]   rl ui                        - ui_reload (HUD/XML)");
    log_msg("~ [rl]   rl all                       - same as rl configs");
    log_msg("~ [rl] example: rl scripts thirst");
}

static void run_rl(const char* args)
{
    while (*args == ' ' || *args == '\t') ++args;
    if (!*args) {
        cmd_rl_help();
        return;
    }
    char tok[64];
    int i = 0;
    while (*args && *args != ' ' && *args != '\t' && i + 1 < (int)sizeof(tok))
        tok[i++] = *args++;
    tok[i] = 0;

    if (streq_ci(tok, "all"))
        cmd_rl_all();
    else if (streq_ci(tok, "scripts") || streq_ci(tok, "script") || streq_ci(tok, "s"))
        cmd_rl_scripts(args);
    else if (streq_ci(tok, "configs") || streq_ci(tok, "config") || streq_ci(tok, "cfg") || streq_ci(tok, "c"))
        cmd_rl_configs();
    else if (streq_ci(tok, "ui") || streq_ci(tok, "hud"))
        cmd_rl_ui();
    else if (streq_ci(tok, "help") || streq_ci(tok, "?"))
        cmd_rl_help();
    else {
        char buf[512];
        int o = 0;
        for (const char* p = tok; *p && o + 1 < (int)sizeof(buf); ++p)
            buf[o++] = *p;
        while (*args && o + 1 < (int)sizeof(buf))
            buf[o++] = *args++;
        buf[o] = 0;
        cmd_rl_scripts(buf);
    }
}

static const char* const BRZ_NPC[] = {
    "esc_bridge_soldier5",
    "esc_soldier_commander_sub",
};
#define BRZ_NPC_N   (int)(sizeof(BRZ_NPC) / sizeof(BRZ_NPC[0]))
#define BRZ_IDS_MAX 8

static const char BRZ_ZONE[] = "escape_bridge_zone";

static unsigned short g_brz_ids[BRZ_IDS_MAX];
static int  g_brz_n = 0;
static char g_brz_logged[BRZ_IDS_MAX];

static int  g_brz_pending = 0;
static int  g_brz_probe = 0;

static void* g_brz_src = (void*)~(uintptr_t)0;
static void* g_brz_dst = 0;

static const char* se_name(void* se)
{
    if (!se) return 0;
    const char* repl = *(const char**)((char*)se + OFF_SE_NAME_REPL);
    if (repl && *repl) return repl;
    void* nm = *(void**)((char*)se + OFF_SE_NAME);
    if (!nm) return 0;
    return (const char*)nm + STRV_VALUE;
}

static const char* obj_name(void* obj)
{
    if (!obj) return 0;
    void* nm = *(void**)((char*)obj + OFF_OBJ_NAME);
    if (!nm) return 0;
    return (const char*)nm + STRV_VALUE;
}

static void* level_object(unsigned short id)
{
    if (id == 0xFFFF) return 0;
    uintptr_t lvl = *(uintptr_t*)(g_base + RVA_GPGAMELEVEL);
    if (!lvl) return 0;
    return *(void**)(lvl + OFF_LVL_OBJECTS + 8ull * (uintptr_t)id);
}

static int brz_name_match(const char* name)
{
    if (!name) return 0;
    for (int i = 0; i < BRZ_NPC_N; ++i)
        if (streq_ci(name, BRZ_NPC[i])) return 1;
    return 0;
}

static int brz_slot(unsigned short id)
{
    for (int i = 0; i < g_brz_n; ++i)
        if (g_brz_ids[i] == id) return i;
    return -1;
}

static void brz_remember(unsigned short id)
{
    if (brz_slot(id) >= 0) return;
    if (g_brz_n >= BRZ_IDS_MAX) return;
    g_brz_logged[g_brz_n] = 0;
    g_brz_ids[g_brz_n++] = id;
}

static int brz_is_exempt(unsigned short id)
{
    if (g_brz_pending) return 1;
    void* obj = level_object(id);
    if (obj) return brz_name_match(obj_name(obj));
    return brz_slot(id) >= 0;
}

static int brz_token_is_zone(const char* b, int n)
{
    if (n != (int)sizeof(BRZ_ZONE) - 1) return 0;
    for (int i = 0; i < n; ++i) {
        char c = b[i];
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (c != BRZ_ZONE[i]) return 0;
    }
    return 1;
}

static void* brz_filter(void* src)
{
    if (src == g_brz_src) return g_brz_dst;

    void* res = src;
    if (src) {
        const char* s = (const char*)src + STRV_VALUE;
        char buf[4096];
        int o = 0, removed = 0;
        for (const char* p = s; *p; ) {
            while (*p == ',' || *p == ' ' || *p == '\t') ++p;
            if (!*p) break;
            const char* b = p;
            while (*p && *p != ',') ++p;
            const char* e = p;
            while (e > b && (e[-1] == ' ' || e[-1] == '\t')) --e;
            int n = (int)(e - b);
            if (n <= 0) continue;
            if (brz_token_is_zone(b, n)) { removed = 1; continue; }
            if (o && o + 1 < (int)sizeof(buf)) buf[o++] = ',';
            for (int i = 0; i < n && o + 1 < (int)sizeof(buf); ++i) buf[o++] = b[i];
        }
        buf[o] = 0;
        if (removed) {
            if (!o) {
                res = 0;
            } else {
                void* tmp = 0;
                ((shared_set_t)(g_base + RVA_SHARED_SET))(&tmp, buf);
                res = tmp;
            }
        }
    }
    if (src) InterlockedIncrement((LONG volatile*)src);
    g_brz_src = src;
    g_brz_dst = res;
    return res;
}

static void hkRestrict(void* mgr, unsigned short id, void* out_r, void* in_r)
{
    if (mgr && brz_is_exempt(id)) {
        void** dflt = (void**)((char*)mgr + OFF_MGR_DEF_IN);
        void*  save = *dflt;
        void*  filt = brz_filter(save);
        if (filt != save) {
            *dflt = filt;
            g_orig_restrict(mgr, id, out_r, in_r);
            *dflt = save;

            if (g_brz_pending) brz_remember(id);
            int i = brz_slot(id);
            if (i >= 0 && !g_brz_logged[i]) {
                g_brz_logged[i] = 1;
                const char* nm = obj_name(level_object(id));
                log_fmt1("~ [brz] escape_bridge_zone dropped for [%s]", nm ? nm : "?");
            }
            return;
        }
    }
    g_orig_restrict(mgr, id, out_r, in_r);
}

static int hkRestrictedNetSpawn(void* self, void* data)
{
    ensure_lua_api();
    const char* nm = se_name(data);
    if (data && g_brz_probe < 6) {
        ++g_brz_probe;
        const char* repl = *(const char**)((char*)data + OFF_SE_NAME_REPL);
        void* sn = *(void**)((char*)data + OFF_SE_NAME);
        log_fmt1("~ [brz] probe repl [%s]", (repl && *repl) ? repl : "-");
        log_fmt1("~ [brz] probe name [%s]", sn ? (const char*)sn + STRV_VALUE : "-");
    }
    int match = (nm && brz_name_match(nm));
    if (match) {
        ++g_brz_pending;
        log_fmt1("~ [brz] tracking [%s]", nm);
    }
    int r = g_orig_ro_spawn(self, data);
    if (match && g_brz_pending > 0) --g_brz_pending;
    return r;
}

static unsigned char __fastcall hkRunAttackCheck(void* self)
{
    if (!self) {
        g_boar_last_reason = 2;
        ++g_boar_fail;
        return 0;
    }
    unsigned char* ra = (unsigned char*)self;
    if (ra[OFF_RA_ACTIVE]) {
        g_boar_last_reason = 1;
        ++g_boar_fail;
        return 0;
    }

    void* obj = *(void**)(ra + OFF_RA_OBJECT);
    if (!obj) {
        g_boar_last_reason = 2;
        ++g_boar_fail;
        return 0;
    }
    void* enemy = *(void**)((char*)obj + OFF_BM_ENEMY);
    if (!enemy) {
        g_boar_last_reason = 3;
        ++g_boar_fail;
        return 0;
    }

    unsigned tnow = *(unsigned*)(g_base + RVA_DEVICE_TIME);
    unsigned tnext = *(unsigned*)(ra + OFF_RA_TIME_NEXT);
    if (tnext > tnow) {
        g_boar_last_reason = 6;
        ++g_boar_fail;
        return 0;
    }

    float* op = (float*)((char*)obj + OFF_OBJ_POS);
    float* ep = (float*)((char*)enemy + OFF_OBJ_POS);
    float dx = ep[0] - op[0];
    float dy = ep[1] - op[1];
    float dz = ep[2] - op[2];
    float d2 = dx * dx + dy * dy + dz * dz;
    float dist = 0.f;
    if (d2 > 1e-8f) {
        union { float f; unsigned u; } u;
        u.f = d2;
        u.u = (u.u >> 1) + 0x1fc00000u;
        float x = u.f;
        x = 0.5f * (x + d2 / x);
        x = 0.5f * (x + d2 / x);
        dist = x;
    }
    g_boar_last_dist = dist;

    float dmin = *(float*)(ra + OFF_RA_MIN_DIST);
    float dmax = *(float*)(ra + OFF_RA_MAX_DIST);
    if (dmin < 2.5f) dmin = 2.5f;
    if (dmax < dmin + 0.5f) dmax = dmin + 3.0f;
    if (dmax > 10.0f) dmax = 10.0f;
    if (dist < dmin || dist > dmax) {
        g_boar_last_reason = 4;
        ++g_boar_fail;
        return 0;
    }

    float fx = *(float*)((char*)obj + OFF_OBJ_XFORM_K_X);
    float fz = *(float*)((char*)obj + OFF_OBJ_XFORM_K_Z);
    float fl2 = fx * fx + fz * fz;
    float tl2 = dx * dx + dz * dz;
    if (fl2 < 1e-8f || tl2 < 1e-8f) {
        g_boar_last_reason = 7;
        ++g_boar_fail;
        return 0;
    }
    float dot = fx * dx + fz * dz;
    if (dot <= 0.f || (dot * dot) < (BOAR_FACE_COS2 * fl2 * tl2)) {
        g_boar_last_reason = 7;
        ++g_boar_fail;
        return 0;
    }

    float vcur = 0.f;
    void* man = *(void**)(ra + OFF_RA_MAN);
    if (man) {
        void* mov = *(void**)((char*)man + OFF_MAN_MOVEMENT);
        if (mov) vcur = *(float*)((char*)mov + OFF_MOV_VEL_CUR);
    }
    g_boar_last_vel = vcur;
    if (vcur < BOAR_MIN_VEL) {
        g_boar_last_reason = 5;
        ++g_boar_fail;
        return 0;
    }

    *(unsigned*)(ra + OFF_RA_TIME_NEXT) = tnow + BOAR_RETRIG_MS;

    g_boar_last_reason = 0;
    ++g_boar_ok;
    return 1;
}

static void cmd_boar()
{
    if (!g_orig_run_atk) {
        log_msg("!! [boar] run-attack hook not installed");
        return;
    }
    log_fmt2i("~ [boar] ok=%d fail=%d", g_boar_ok, g_boar_fail);
    const char* why = "ok";
    switch (g_boar_last_reason) {
    case 1: why = "active"; break;
    case 2: why = "no_object"; break;
    case 3: why = "no_enemy"; break;
    case 4: why = "distance"; break;
    case 5: why = "standing"; break;
    case 6: why = "cooldown"; break;
    case 7: why = "not_facing"; break;
    }
    log_fmt1("~ [boar] last_reason [%s]", why);
    log_fmt2i("~ [boar] last_dist_x100=%d last_vel_x100=%d",
              (int)(g_boar_last_dist * 100.f), (int)(g_boar_last_vel * 100.f));
}

static void cmd_brz()
{
    if (!g_orig_restrict) {
        log_msg("!! [brz] restrict() hook not installed");
        return;
    }
    log_fmt1("~ [brz] zone [%s]", BRZ_ZONE);
    for (int i = 0; i < BRZ_NPC_N; ++i)
        log_fmt1("~ [brz] npc  [%s]", BRZ_NPC[i]);
    if (!g_brz_n) {
        log_msg("~ [brz] no tracked object online yet");
    } else {
        for (int i = 0; i < g_brz_n; ++i) {
            const char* nm = obj_name(level_object(g_brz_ids[i]));
            log_fmt2i("~ [brz] id=%d applied=%d", (int)g_brz_ids[i], (int)g_brz_logged[i]);
            log_fmt1("~ [brz]   name [%s]", nm ? nm : "(offline)");
        }
    }
    if (g_brz_src != (void*)~(uintptr_t)0) {
        log_fmt1("~ [brz] default_in      [%s]",
                 g_brz_src ? (const char*)g_brz_src + STRV_VALUE : "");
        log_fmt1("~ [brz] default_in(brz) [%s]",
                 g_brz_dst ? (const char*)g_brz_dst + STRV_VALUE : "");
    } else {
        log_msg("~ [brz] restrict() not reached for a tracked object yet");
    }
}

static const char* const BP_TEX[] = {
    "wm\\wm_blood_pool_1",  "wm\\wm_blood_pool_2",  "wm\\wm_blood_pool_3",
    "wm\\wm_blood_pool_4",  "wm\\wm_blood_pool_5",  "wm\\wm_blood_pool_6",
    "wm\\wm_blood_pool_7",  "wm\\wm_blood_pool_8",  "wm\\wm_blood_pool_9",
    "wm\\wm_blood_pool_10", "wm\\wm_blood_pool_11", "wm\\wm_blood_pool_12",
    "wm\\wm_blood_pool_13", "wm\\wm_blood_pool_14", "wm\\wm_blood_pool_15",
    "wm\\wm_blood_pool_16", "wm\\wm_blood_pool_17", "wm\\wm_blood_pool_18",
    "wm\\wm_blood_pool_19", "wm\\wm_blood_pool_20", "wm\\wm_blood_pool_21",
    "wm\\wm_blood_pool_22", "wm\\wm_blood_pool_23", "wm\\wm_blood_pool_24",
};
#define BP_TEX_N (int)(sizeof(BP_TEX) / sizeof(BP_TEX[0]))

static void* g_bp_array = 0;
static void* g_bp_one[BP_TEX_N] = { 0 };
static int   g_bp_rotfix = 0;
static int   g_bp_ok = 0;
static int   g_bp_fail = 0;
static int   g_bp_reason = 0;

static void bp_build_array(void)
{
    if (g_bp_array) return;
    create_wma_t create = (create_wma_t)(g_base + RVA_CREATE_WMA);
    appendmark_t append = (appendmark_t)(g_base + RVA_APPENDMARK);
    void* arr = create((void*)(g_base + RVA_RENDERFACTORY));
    if (!arr) return;
    for (int i = 0; i < BP_TEX_N; ++i)
        append(arr, BP_TEX[i]);
    for (int i = 0; i < BP_TEX_N; ++i) {
        void* one = create((void*)(g_base + RVA_RENDERFACTORY));
        if (!one) break;
        append(one, BP_TEX[i]);
        g_bp_one[i] = one;
    }
    g_bp_array = arr;
}

static int bp_place(float x, float y, float z, float dist, float size, int tex)
{
    uintptr_t lvl = *(uintptr_t*)(g_base + RVA_GPGAMELEVEL);
    if (!lvl) { g_bp_reason = 1; ++g_bp_fail; return 0; }

    bp_build_array();
    if (!g_bp_array) { g_bp_reason = 2; ++g_bp_fail; return 0; }
    void* arr = g_bp_array;
    if (tex >= 1 && tex <= BP_TEX_N && g_bp_one[tex - 1]) arr = g_bp_one[tex - 1];

    float start[3] = { x, y, z };
    float dir[3]   = { 0.f, -1.f, 0.f };
    unsigned char R[16];

    raypick_t raypick = (raypick_t)(g_base + RVA_RAYPICK);
    void* objspace = (void*)(lvl + OFF_LVL_OBJSPACE);
    if (!raypick(objspace, start, dir, dist, BP_RQ_STATIC, R, 0)) {
        g_bp_reason = 3; ++g_bp_fail; return 0;
    }
    if (*(void**)(R + 0)) { g_bp_reason = 4; ++g_bp_fail; return 0; }
    float range = *(float*)(R + 8);
    int   element = *(int*)(R + 0xC);
    if (element < 0) { g_bp_reason = 3; ++g_bp_fail; return 0; }

    float end[3];
    end[0] = start[0] + range * dir[0];
    end[1] = start[1] + range * dir[1];
    end[2] = start[2] + range * dir[2];

    void* tris  = *(void**)(lvl + OFF_LVL_TRIS);
    void* verts = *(void**)(lvl + OFF_LVL_VERTS);
    if (!tris || !verts) { g_bp_reason = 5; ++g_bp_fail; return 0; }

    unsigned char* tri = (unsigned char*)tris + (size_t)element * BP_TRI_SIZE;
    unsigned matid = *(unsigned short*)(tri + BP_TRI_DUMMY) & BP_TRI_MATMASK;

    void** mat_first = *(void***)(g_base + RVA_GMLIB_FIRST);
    void** mat_last  = *(void***)(g_base + RVA_GMLIB_LAST);
    if (!mat_first || matid >= (unsigned)(mat_last - mat_first)) {
        g_bp_reason = 6; ++g_bp_fail; return 0;
    }
    void* mtl = mat_first[matid];
    if (!mtl || !(*(unsigned*)((char*)mtl + OFF_MTL_FLAGS) & MTL_FL_BLOODMARK)) {
        g_bp_reason = 7; ++g_bp_fail; return 0;
    }

    void* render = *(void**)(g_base + RVA_RENDER);
    if (!render) { g_bp_reason = 8; ++g_bp_fail; return 0; }

    ((add_static_wm_t)(g_base + RVA_ADD_STATIC_WM))(render, arr, end, size, tri, verts);
    g_bp_reason = 0; ++g_bp_ok;
    return 1;
}

static const char* bp_parse_float(const char* s, float* out)
{
    while (*s == ' ' || *s == '\t') ++s;
    int neg = 0;
    if (*s == '-') { neg = 1; ++s; }
    else if (*s == '+') ++s;
    if ((*s < '0' || *s > '9') && *s != '.') return 0;
    double v = 0.0;
    int any = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10.0 + (*s - '0'); ++s; any = 1; }
    if (*s == '.') {
        ++s;
        double f = 0.1;
        while (*s >= '0' && *s <= '9') { v += (*s - '0') * f; f *= 0.1; ++s; any = 1; }
    }
    if (!any) return 0;
    *out = (float)(neg ? -v : v);
    return s;
}

static int bp_set_rotfix(int on)
{
    uint8_t* p = (uint8_t*)(g_base + RVA_WM_ROT_ANGLE);
    const uint8_t orig[3]  = { 0x0F, 0x28, 0xC1 };
    const uint8_t fixed[3] = { 0x0F, 0x57, 0xC0 };
    const uint8_t* want = on ? fixed : orig;
    const uint8_t* have = on ? orig  : fixed;
    if (p[0] == want[0] && p[1] == want[1] && p[2] == want[2]) {
        g_bp_rotfix = on;
        return 1;
    }
    if (p[0] != have[0] || p[1] != have[1] || p[2] != have[2]) return 0;
    DWORD oldp;
    if (!VirtualProtect(p, 3, PAGE_EXECUTE_READWRITE, &oldp)) return 0;
    p[0] = want[0]; p[1] = want[1]; p[2] = want[2];
    VirtualProtect(p, 3, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), p, 3);
    g_bp_rotfix = on;
    return 1;
}

static void cmd_bpm(const char* a)
{
    while (*a == ' ' || *a == '\t') ++a;
    if (!*a) {
        log_fmt2i("~ [bpm] ok=%d fail=%d", g_bp_ok, g_bp_fail);
        log_fmt1i("~ [bpm] last_reason=%d (0ok 3no_hit 7not_bloodmark)", g_bp_reason);
        log_fmt1i("~ [bpm] rotfix=%d (decal rotation jitter off)", g_bp_rotfix);
        return;
    }
    if (a[0] == 'r' && a[1] == 'o' && a[2] == 't') {
        const char* b = a + 3;
        while (*b == ' ' || *b == '\t') ++b;
        if (*b == '0' || *b == '1') {
            if (!bp_set_rotfix(*b - '0'))
                log_msg("!! [bpm] rot: patch site mismatch");
        }
        log_fmt1i("~ [bpm] rotfix=%d", g_bp_rotfix);
        return;
    }
    float x, y, z, dist, size, tex = 0.f;
    const char* p = bp_parse_float(a, &x);
    if (p) p = bp_parse_float(p, &y);
    if (p) p = bp_parse_float(p, &z);
    if (p) p = bp_parse_float(p, &dist);
    if (p) p = bp_parse_float(p, &size);
    if (!p) { log_msg("!! [bpm] usage: bpm x y z dist size [tex]"); return; }
    bp_parse_float(p, &tex);
    bp_place(x, y, z, dist, size, (int)tex);
}

static void* wm_hands_model(int second)
{
    if (!g_base) return 0;
    void** pph = (void**)(g_base + RVA_G_PLAYER_HUD);
    void* ph = pph ? *pph : 0;
    if (!ph) return 0;
    return *(void**)((char*)ph + (second ? OFF_PH_MODEL_2 : OFF_PH_MODEL));
}

static void* wm_hands_kin(int second)
{
    void* model = wm_hands_model(second);
    if (!model) return 0;
    void** vt = *(void***)model;
    if (!vt) return 0;
    dcast_pkin_t dcast = (dcast_pkin_t)vt[OFF_VT_DCAST_PKIN / sizeof(void*)];
    if (!dcast) return 0;
    return dcast(model);
}

static const char* wm_visual_name(int second)
{
    void* model = wm_hands_model(second);
    if (!model) return "<none>";
    void** vt = *(void***)model;
    if (!vt) return "<no vtbl>";
    dcast_rvis_t dcast = (dcast_rvis_t)vt[OFF_VT_DCAST_RVIS / sizeof(void*)];
    if (!dcast) return "<no dcast>";
    void* vis = dcast(model);
    if (!vis) return "<no visual>";
    void** vvt = *(void***)vis;
    if (!vvt) return "<no vis vtbl>";
    getdebugname_t f = (getdebugname_t)vvt[OFF_VT_GETDEBUGNAME / sizeof(void*)];
    if (!f) return "<no getDebugName>";
    char* sv = 0;
    f(vis, &sv);
    return sv ? sv + STRV_VALUE : "<empty>";
}

static int wm_name_eq(const char* path, const char* want)
{
    if (!path || !want || !*want) return 0;
    const char* base = path;
    for (const char* p = path; *p; ++p)
        if (*p == '\\' || *p == '/') base = p + 1;
    unsigned i = 0;
    for (;; ++i) {
        char a = base[i], b = want[i];
        if (a >= 'A' && a <= 'Z') a = (char)(a + 32);
        if (b >= 'A' && b <= 'Z') b = (char)(b + 32);
        if (!b) break;
        if (a != b) return 0;
    }
    const char* rest = base + i;
    if (!*rest) return 1;
    if (rest[0] != '.') return 0;
    return (rest[1] == 'o' || rest[1] == 'O') &&
           (rest[2] == 'g' || rest[2] == 'G') &&
           (rest[3] == 'f' || rest[3] == 'F') && !rest[4];
}

static unsigned wm_child_count(void* kin)
{
    if (!kin) return 0;
    void** vt = *(void***)kin;
    if (!vt) return 0;
    rchildcount_t f = (rchildcount_t)vt[OFF_VT_RCHILDCOUNT / sizeof(void*)];
    return f ? f(kin) : 0;
}

static int wm_set(void* kin, unsigned idx, bool state)
{
    if (!kin) return 0;
    if (idx >= wm_child_count(kin)) { ++g_wm_skipped; return 0; }
    void** vt = *(void***)kin;
    if (!vt) return 0;
    set_rflag_t f = (set_rflag_t)vt[OFF_VT_SET_RFLAG / sizeof(void*)];
    if (!f) return 0;
    f(kin, idx, state);
    ++g_wm_applied;
    return 1;
}

static void cmd_wm(const char* a)
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    while (*a == ' ' || *a == '\t') ++a;

    void* k1 = wm_hands_kin(0);
    void* k2 = wm_hands_kin(1);

    if (!*a) {
        Msg("~ [wm] m_model=%p children=%u visual=%s",
            k1, wm_child_count(k1), wm_visual_name(0));
        Msg("~ [wm] m_model_2=%p children=%u visual=%s",
            k2, wm_child_count(k2), wm_visual_name(1));
        Msg("~ [wm] calls=%d applied=%d skipped_idx=%d skipped_model=%d "
            "skipped_name=%d nomatch=%d last_state=%d nomodel=%d",
            g_wm_calls, g_wm_applied, g_wm_skipped, g_wm_skipped_model,
            g_wm_skipped_name, g_wm_mismatch, g_wm_last_state, g_wm_nomodel);
        return;
    }

    if (*a != '0' && *a != '1') {
        Msg("!! [wm] usage: wm <0|1> <nchildren> [visual] <idx> [idx ...]  |  wm");
        return;
    }
    bool state = (*a == '1');
    ++a;

    while (*a == ' ' || *a == '\t') ++a;
    if (*a < '0' || *a > '9') {
        Msg("!! [wm] usage: wm <0|1> <nchildren> [visual] <idx> [idx ...]  |  wm");
        return;
    }
    unsigned want = 0;
    while (*a >= '0' && *a <= '9') { want = want * 10u + (unsigned)(*a - '0'); ++a; }

    char vname[64];
    vname[0] = 0;
    while (*a == ' ' || *a == '\t') ++a;
    if (*a && (*a < '0' || *a > '9')) {
        unsigned j = 0;
        while (*a && *a != ' ' && *a != '\t' && j + 1 < sizeof(vname)) vname[j++] = *a++;
        vname[j] = 0;
    }

    if (!k1 && !k2) {
        if (++g_wm_nomodel <= 3)
            Msg("!! [wm] no hands model (g_player_hud/m_model empty), n=%d", g_wm_nomodel);
        return;
    }

    unsigned n1 = wm_child_count(k1), n2 = wm_child_count(k2);
    void* t1 = (want && n1 != want) ? 0 : k1;
    void* t2 = (want && n2 != want) ? 0 : k2;
    if ((k1 && !t1) || (k2 && !t2)) ++g_wm_skipped_model;
    if (vname[0]) {
        if (t1 && !wm_name_eq(wm_visual_name(0), vname)) { t1 = 0; ++g_wm_skipped_name; }
        if (t2 && !wm_name_eq(wm_visual_name(1), vname)) { t2 = 0; ++g_wm_skipped_name; }
    }
    if (!t1 && !t2) {
        if (++g_wm_mismatch <= 3)
            Msg("!! [wm] no model %s with %u children: m_model=%u (%s) m_model_2=%u (%s)",
                vname[0] ? vname : "<any>", want,
                n1, wm_visual_name(0), n2, wm_visual_name(1));
        return;
    }

    int n = 0;
    while (*a) {
        while (*a == ' ' || *a == '\t' || *a == ',') ++a;
        if (*a < '0' || *a > '9') break;
        unsigned idx = 0;
        while (*a >= '0' && *a <= '9') { idx = idx * 10u + (unsigned)(*a - '0'); ++a; }
        if (idx > WM_MAX_IDX) continue;
        wm_set(t1, idx, state);
        wm_set(t2, idx, state);
        ++n;
    }

    ++g_wm_calls;
    g_wm_last_state = state ? 1 : 0;
    if (!n) Msg("!! [wm] no valid indices in command");
}

static int g_blr_on = 0;
static int g_blr_added = 0;

typedef void* (__fastcall *additem_t)(void* lb, const char* text);
static additem_t g_orig_additem = 0;

static int str_eq(const char* a, const char* b)
{
    while (*a && *a == *b) { ++a; ++b; }
    return *a == *b;
}

static void* __fastcall hkListBoxAddItem(void* lb, const char* text)
{
    void* item = g_orig_additem(lb, text);
    if (g_blr_on && item && text && str_eq(text, "st_undress_outfit")) {
        void* extra = g_orig_additem(lb, "st_ballon_remove");
        if (extra) {
            *(uint64_t*)((uint8_t*)extra + UI_LBI_PDATA) = 0;
            *(uint32_t*)((uint8_t*)extra + UI_LBI_TAG) = UI_TAG_CHARGE_TORCH;
            ++g_blr_added;
        }
    }
    return item;
}

static void* install_additem_detour(void* hook)
{
    uint8_t* func = (uint8_t*)(g_base + RVA_UI_ADDITEM);
    const SIZE_T steal = (SIZE_T)(RVA_UI_ADDITEM_CONT - RVA_UI_ADDITEM);
    if (steal != 18) return 0;

    if (!(func[0] == 0x40 && func[1] == 0x56 && func[2] == 0x41 && func[3] == 0x57 &&
          func[4] == 0x48 && func[5] == 0x81 && func[6] == 0xEC && func[7] == 0xB8 &&
          func[8] == 0x00 && func[9] == 0x00 && func[10] == 0x00 &&
          func[11] == 0x48 && func[12] == 0x8B && func[13] == 0x05))
        return 0;

    uint8_t* tramp = (uint8_t*)VirtualAlloc(0, 64,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return 0;

    SIZE_T o = 0;
    for (; o < 11; ++o) tramp[o] = func[o];
    tramp[o++] = 0x48; tramp[o++] = 0xB8;
    *(uint64_t*)(tramp + o) = g_base + RVA_SECURITY_COOKIE;
    o += 8;
    tramp[o++] = 0x48; tramp[o++] = 0x8B; tramp[o++] = 0x00;
    tramp[o++] = 0xFF; tramp[o++] = 0x25;
    *(uint32_t*)(tramp + o) = 0; o += 4;
    *(uint64_t*)(tramp + o) = (uint64_t)(func + steal);

    DWORD oldp;
    if (!VirtualProtect(func, steal, PAGE_EXECUTE_READWRITE, &oldp)) return 0;
    func[0] = 0xFF; func[1] = 0x25;
    *(uint32_t*)(func + 2) = 0;
    *(uint64_t*)(func + 6) = (uint64_t)hook;
    for (SIZE_T i = 14; i < steal; ++i) func[i] = 0x90;
    VirtualProtect(func, steal, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), func, steal);
    return tramp;
}

static void cmd_hudrc(const char* a)
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    while (*a == ' ' || *a == '\t') ++a;

    if (a[0] == 'c' && a[1] == 'l') { hudrc_reset(); Msg("~ [hudrc] cleared"); return; }

    if (a[0] == 'l' && a[1] == 'o') {
        const char* s = a + 4;
        while (*s == ' ' || *s == '\t') ++s;
        if (*s == '1') g_hudrc_lock = 1;
        else if (*s == '0') { g_hudrc_lock = 0; hudrc_reset(); }
        Msg("~ [hudrc] lock=%d (Lua writes ignored, watchdog frozen)", g_hudrc_lock);
        return;
    }

    if (a[0] == 't' && a[1] == 'e') {
        float v[6] = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
        const char* s = a + 4;
        int n = 0;
        while (n < 6 && s) {
            const char* nxt = bp_parse_float(s, &v[n]);
            if (!nxt) break;
            s = nxt;
            ++n;
        }
        int ok;
        if (n == 3)      ok = hudrc_set(v[0], v[1], v[2]);
        else if (n == 6) ok = hudrc_set6(v[0], v[1], v[2], v[3], v[4], v[5]);
        else {
            Msg("!! [hudrc] usage: hudrc test <py> <pitch> <yaw>"
                "  |  hudrc test <px> <py> <pz> <pitch> <yaw> <roll>");
            return;
        }
        Msg("~ [hudrc] test set=%d pos=%f %f %f rot=%f %f %f", ok,
            g_hudrc_pos[0], g_hudrc_pos[1], g_hudrc_pos[2],
            g_hudrc_rot[0], g_hudrc_rot[1], g_hudrc_rot[2]);
        return;
    }

    void* it = hudrc_item();
    Msg("~ [hudrc] bound=%d player_hud=%p item=%p hud_item=%p owner=%p",
        g_lua_api_state ? 1 : 0,
        g_base ? *(void**)(g_base + RVA_G_PLAYER_HUD) : 0, it,
        it ? *(void**)((char*)it + OFF_AHI_PARENT_ITEM) : 0, g_hudrc_owner);
    Msg("~ [hudrc] on=%d pos=%f %f %f rot=%f %f %f age_ms=%u frames=%d",
        g_hudrc_on,
        g_hudrc_pos[0], g_hudrc_pos[1], g_hudrc_pos[2],
        g_hudrc_rot[0], g_hudrc_rot[1], g_hudrc_rot[2],
        g_hudrc_on ? (unsigned)(hudrc_time_ms() - g_hudrc_stamp) : 0u, g_hudrc_frames);
    Msg("~ [hudrc] lock=%d clears=%d expired=%d rej_nohud=%d rej_item=%d "
        "rej_invalid=%d rej_locked=%d",
        g_hudrc_lock, g_hudrc_clears, g_hudrc_expired, g_hudrc_rej_nohud,
        g_hudrc_rej_item, g_hudrc_rej_bad, g_hudrc_rej_lock);
}

static void cmd_wbf(void)
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    Msg("~ [wbf] name=%s hooks: fire_trace=%p gl_onshot=%p",
        WBF_NAME, (void*)g_orig_fire_trace, (void*)g_orig_gl_onshot);
    Msg("~ [wbf] calls=%d no_vm=%d no_callback=%d lua_errors=%d",
        g_wbf_calls, g_wbf_no_vm, g_wbf_no_cb, g_wbf_err);
    Msg("~ [wbf] skipped_not_actor=%d skipped_misfire=%d last_gl=%d",
        g_wbf_skip_np, g_wbf_skip_mf, g_wbf_last_gl);
}

static void cmd_notes_diag()
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    char exe[MAX_PATH];
    exe[0] = 0;
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    Msg("~ [notes] exe=%s base=%p", exe, (void*)g_base);
    uint8_t* f = (uint8_t*)(g_base + RVA_STR_TABLE_LOAD);
    Msg("~ [notes] want=40 55 53 48 8D AC 24 58 F9 FF FF 48 81 EC A8 07");
    Msg("~ [notes] got =%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
        f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7],
        f[8], f[9], f[10], f[11], f[12], f[13], f[14], f[15]);
    int bad = -1;
    for (int i = 0; i < 16; ++i)
        if (f[i] != kNotesLoadPrologue[i]) { bad = i; break; }
    Msg("~ [notes] prologue_bad_idx=%d", bad);
    char abuf[48];
    const char* asrc = (const char*)(g_base + RVA_STR_TABLE_ANCHOR);
    int ai = 0;
    for (; ai < 47; ++ai) {
        char c = asrc[ai];
        abuf[ai] = c;
        if (!c) break;
    }
    abuf[ai] = 0;
    Msg("~ [notes] anchor_str=[%s]", abuf);
    int found = -1;
    for (int i = 0; i < 0x400 - 7; ++i) {
        if ((f[i] == 0x48 || f[i] == 0x4C) && (f[i+1] == 0x8B || f[i+1] == 0x8D) &&
            ((f[i+2] & 0xC7) == 0x05)) {
            int32_t d = *(int32_t*)(f + i + 3);
            if (f + i + 7 + d == (uint8_t*)(g_base + RVA_STR_TABLE_ANCHOR)) { found = i; break; }
        }
    }
    Msg("~ [notes] anchor_ref_at=0x%x", (unsigned)found);
}

static void cmd_notes(const char* a)
{
    msg_t Msg = (msg_t)(g_base + RVA_MSG);
    while (*a == ' ' || *a == '	') ++a;
    if (*a == 0) { run_fl("he_pda_notes.open()"); return; }
    if (a[0] == 's' && a[1] == 't') {
        Msg("~ [notes] load=%p verified=%d failed=%d pushes=%d seh=%d test=%d lua=%d",
            (void*)(g_base + RVA_STR_TABLE_LOAD),
            g_notes_verified, g_notes_failed, g_notes_pushes,
            g_notes_seh, g_notes_test_ok,
            g_lua_api_state ? 1 : 0);
        return;
    }
    if (a[0] == 'p' && a[1] == 'u') {
        if (a[2] == 's' && a[3] == 'h' && a[4] == '_' && a[5] == 't') {
            g_notes_test_ok = notes_load_file("string_table_enc_zone");
            Msg("~ [notes] push_test(known-good)=%d seh=%d", g_notes_test_ok, g_notes_seh);
            return;
        }
        Msg("~ [notes] push=%d", notes_push());
        return;
    }
    if (a[0] == 'd' && a[1] == 'i') {
        cmd_notes_diag();
        return;
    }
    Msg("~ [notes] usage: notes | notes status | notes push[_test] | notes diag");
}

static sr_go1_t g_orig_sr_go1 = 0;
static int      g_scost_bound = 0;
static unsigned g_scost_calls = 0;
static unsigned g_scost_misses = 0;
static unsigned g_scost_last = 0;

static void heSetCost(void* script_obj, unsigned cost)
{
    void* item = ((sgo_cast_invitem_t)(g_base + RVA_SGO_CAST_INVITEM))(script_obj);
    if (!item) {
        ++g_scost_misses;
        return;
    }
    *(unsigned*)((char*)item + OFF_INVITEM_COST) = cost;
    g_scost_last = cost;
    ++g_scost_calls;
}

static void* hkScriptRegisterGameObject1(void* ret, void* src)
{
    void* cls = g_orig_sr_go1 ? g_orig_sr_go1(ret, src) : ret;
    if (cls) {
        ((lb_def_u32_t)(g_base + RVA_LB_DEF_U32))(cls, "set_cost", (void*)&heSetCost);
        ++g_scost_bound;
    }
    return cls;
}

static void cmd_scost()
{
    if (!g_orig_sr_go1) {
        log_msg("!! [scost] class registration hook not installed");
        return;
    }
    log_fmt2i("~ [scost] bound=%d calls=%d", g_scost_bound, (int)g_scost_calls);
    log_fmt2i("~ [scost] not_an_item=%d last_cost=%d", (int)g_scost_misses, (int)g_scost_last);
}

static void hkExecuteCommand(void* self, const char* cmd, char record, char allow)
{
    g_console = self;
    const char* p = cmd;
    if (p) {
        while (*p == ' ' || *p == '\t') ++p;

        if (p[0] == 'f' && p[1] == 'l' &&
            (p[2] == 0 || p[2] == ' ' || p[2] == '\t')) {
            const char* a = p + 2;
            while (*a == ' ' || *a == '\t') ++a;
            run_fl(a);
            return;
        }

        if (p[0] == 'r' && p[1] == 'l' &&
            (p[2] == 0 || p[2] == ' ' || p[2] == '\t')) {
            const char* a = p + 2;
            while (*a == ' ' || *a == '\t') ++a;
            run_rl(a);
            return;
        }

        if (p[0] == 'b' && p[1] == 'r' && p[2] == 'z' &&
            (p[3] == 0 || p[3] == ' ' || p[3] == '\t')) {
            cmd_brz();
            return;
        }

        if (p[0] == 'b' && p[1] == 'o' && p[2] == 'a' && p[3] == 'r' &&
            (p[4] == 0 || p[4] == ' ' || p[4] == '\t')) {
            cmd_boar();
            return;
        }

        if (p[0] == 'b' && p[1] == 'p' && p[2] == 'm' &&
            (p[3] == 0 || p[3] == ' ' || p[3] == '\t')) {
            cmd_bpm(p + 3);
            return;
        }

        if (p[0] == 't' && p[1] == 'e' && p[2] == 'x' &&
            (p[3] == 0 || p[3] == ' ' || p[3] == '\t')) {
            cmd_tex(p + 3);
            return;
        }

        if (p[0] == 'w' && p[1] == 'm' &&
            (p[2] == 0 || p[2] == ' ' || p[2] == '\t')) {
            cmd_wm(p + 2);
            return;
        }

        if (p[0] == 'h' && p[1] == 'u' && p[2] == 'd' && p[3] == 'r' && p[4] == 'c' &&
            (p[5] == 0 || p[5] == ' ' || p[5] == '\t')) {
            cmd_hudrc(p + 5);
            return;
        }

        if (p[0] == 's' && p[1] == 'c' && p[2] == 'o' && p[3] == 's' && p[4] == 't' &&
            (p[5] == 0 || p[5] == ' ' || p[5] == '\t')) {
            cmd_scost();
            return;
        }

        if (p[0] == 'w' && p[1] == 'b' && p[2] == 'f' &&
            (p[3] == 0 || p[3] == ' ' || p[3] == '\t')) {
            cmd_wbf();
            return;
        }

        if (p[0] == 'b' && p[1] == 'p' && p[2] == 'u' && p[3] == 'i' &&
            (p[4] == 0 || p[4] == ' ' || p[4] == '\t')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            const char* a = p + 4;
            while (*a == ' ' || *a == '\t') ++a;
            if (*a == '1') g_bp_ui = 1;
            else if (*a == '0') g_bp_ui = 0;
            Msg("! [fl] bpui=%d body_skips=%d hands_skips=%d",
                g_bp_ui, g_skip_body_n, g_skip_hands_n);
            return;
        }

        if (p[0] == 'b' && p[1] == 'l' && p[2] == 'r' &&
            (p[3] == 0 || p[3] == ' ' || p[3] == '\t')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            const char* a = p + 3;
            while (*a == ' ' || *a == '\t') ++a;
            if (*a == '1') g_blr_on = 1;
            else if (*a == '0') g_blr_on = 0;
            else Msg("! [fl] blr=%d added=%d hook=%p",
                     g_blr_on, g_blr_added, (void*)g_orig_additem);
            return;
        }

        if (p[0] == 't' && p[1] == 'l' && p[2] == 'u' && p[3] == 'a' &&
            (p[4] == 0 || p[4] == ' ' || p[4] == '\t')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            const char* a = p + 4;
            while (*a == ' ' || *a == '\t') ++a;
            if (*a == '1') g_torch_lua = 1;
            else if (*a == '0') g_torch_lua = 0;
            Msg("! [fl] tlua=%d blocked=%d hook=%p active_item=%p",
                g_torch_lua, g_torch_blocked_n, (void*)g_orig_actor_kbpress,
                g_actor_ircv ? ircv_active_item(g_actor_ircv) : 0);
            return;
        }

        if (p[0] == 's' && p[1] == 't' && p[2] == 'e' && p[3] == 'p' &&
            p[4] == 'm' && p[5] == 't' && p[6] == 'l' &&
            (p[7] == 0 || p[7] == ' ' || p[7] == '\t')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            ensure_lua_api();
            const char* ground = 0;
            const char* self_m = 0;
            int gid = 0;
            if (step_material_read(&ground, &self_m, &gid))
                Msg("! [fl] stepmtl: ground=[%s] id=%d self=[%s] bound=%d",
                    ground, gid, self_m ? self_m : "", g_lua_api_state ? 1 : 0);
            else
                Msg("! [fl] stepmtl: unavailable (no actor / no material lib)");
            return;
        }

        if (p[0] == 'b' && p[1] == 'p' && p[2] == 'b' && p[3] == 'o' &&
            p[4] == 'd' && p[5] == 'y' &&
            (p[6] == 0 || p[6] == ' ' || p[6] == '\t')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            void** pph = (void**)(g_base + RVA_G_PLAYER_HUD);
            void* ph = pph ? *pph : 0;
            unsigned part = 0xFF;
            void* item = 0;
            if (ph) {
                part = *(unsigned char*)((char*)ph + OFF_PH_SCRIPT_PART);
                item = *(void**)((char*)ph + OFF_PH_SCRIPT_ITEM);
            }
            Msg("! [fl] bpbody: bpui=%d body_skips=%d hands_skips=%d "
                "g_player_hud=%p part=%u item=%p body_hook=%p hud_hook=%p",
                g_bp_ui, g_skip_body_n, g_skip_hands_n, ph, part, item,
                (void*)g_orig_actor_render, (void*)g_orig_actor_onhud);
            return;
        }

        if (p[0] == 'p' && p[1] == 'd' && p[2] == 'a' && p[3] == '3' && p[4] == 'd' &&
            (p[5] == 0 || p[5] == ' ' || p[5] == '\t')) {
            cmd_pda3d(p + 5);
            return;
        }

        if (p[0] == 'a' && p[1] == 'p' && p[2] == 'a' && p[3] == 't' &&
            p[4] == 'h' && (p[5] == 0 || p[5] == ' ' || p[5] == '	')) {
            msg_t Msg = (msg_t)(g_base + RVA_MSG);
            uint8_t* q = (uint8_t*)(g_base + RVA_ALIFE_NOWAY);
            Msg("! [fl] apath: patched=%d site=%02X %02X %02X %02X %02X (build alifepath-1)",
                g_alife_noway_patched, q[0], q[1], q[2], q[3], q[4]);
            return;
        }
        if (p[0] == 'n' && p[1] == 'o' && p[2] == 't' && p[3] == 'e' && p[4] == 's' &&
            (p[5] == 0 || p[5] == ' ' || p[5] == '	')) {
            cmd_notes(p + 5);
            return;
        }

    }
    g_orig(self, cmd, record, allow);
}

static void render_script_hud_item_only(void* ph, unsigned context_id, void* root)
{
    void* item = *(void**)((char*)ph + OFF_PH_SCRIPT_ITEM);
    if (!item) return;

    void** pr = (void**)(g_base + RVA_RENDER);
    void* render = pr ? *pr : 0;
    if (!render) return;

    void** item_vt = *(void***)item;
    if (!item_vt) return;
    dcast_rvis_t dcast = (dcast_rvis_t)item_vt[OFF_VT_DCAST_RVIS / sizeof(void*)];
    if (!dcast) return;
    void* visual = dcast(item);
    if (!visual) return;

    void** r_vt = *(void***)render;
    if (!r_vt) return;
    add_visual_t add_vis = (add_visual_t)r_vt[OFF_VT_ADD_VISUAL / sizeof(void*)];
    if (!add_vis) return;

    void* xform = (char*)ph + OFF_PH_ITEM_POS;
    add_vis(render, context_id, root, visual, xform);
}

static void hkActorRenderableRender(void* self, unsigned context_id, void* root)
{
    if (g_orig_actor_render && g_base && g_bp_ui) {
        void* actor = (char*)self - OFF_IR_TO_ACTOR;
        int cam = *(int*)((char*)actor + OFF_ACTOR_CAM_ACTIVE);
        if (cam == 0) {
            ++g_skip_body_n;
            return;
        }
    }
    if (g_orig_actor_render)
        g_orig_actor_render(self, context_id, root);
}

static void hkActorOnHUDDraw(void* self, void* hud, unsigned context_id, void* root)
{
    ensure_lua_api();
    hudrc_apply();
    pda3d_render();
    if (g_base && g_bp_ui) {
        void** pph = (void**)(g_base + RVA_G_PLAYER_HUD);
        void* ph = pph ? *pph : 0;
        if (ph) {
            ++g_skip_hands_n;
            render_script_hud_item_only(ph, context_id, root);
            hudrc_restore();
            return;
        }
    }
    if (g_orig_actor_onhud)
        g_orig_actor_onhud(self, hud, context_id, root);
    hudrc_restore();
}

static void* install_onhuddraw_detour(void* hook)
{
    uint8_t* func = (uint8_t*)(g_base + RVA_ACTOR_ONHUDDRAW);
    const SIZE_T steal = (SIZE_T)(RVA_ACTOR_ONHUDDRAW_END - RVA_ACTOR_ONHUDDRAW);
    if (steal != 16) return 0;

    if (!(func[0] == 0x40 && func[1] == 0x53 && func[2] == 0x55 &&
          func[3] == 0x56 && func[4] == 0x57 &&
          func[5] == 0x48 && func[6] == 0x83 && func[7] == 0xEC && func[8] == 0x38 &&
          func[9] == 0x48 && func[10] == 0x8B && func[11] == 0x3D))
        return 0;

    uint8_t* tramp = (uint8_t*)VirtualAlloc(0, 64,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return 0;

    SIZE_T o = 0;
    for (; o < 9; ++o) tramp[o] = func[o];
    tramp[o++] = 0x48; tramp[o++] = 0xB8;
    *(uint64_t*)(tramp + o) = g_base + RVA_G_PLAYER_HUD;
    o += 8;
    tramp[o++] = 0x48; tramp[o++] = 0x8B; tramp[o++] = 0x38;
    tramp[o++] = 0xFF; tramp[o++] = 0x25;
    *(uint32_t*)(tramp + o) = 0; o += 4;
    *(uint64_t*)(tramp + o) = (uint64_t)(func + steal);

    DWORD oldp;
    if (!VirtualProtect(func, steal, PAGE_EXECUTE_READWRITE, &oldp)) return 0;
    func[0] = 0xFF; func[1] = 0x25;
    *(uint32_t*)(func + 2) = 0;
    *(uint64_t*)(func + 6) = (uint64_t)hook;
    func[14] = 0x90;
    func[15] = 0x90;
    VirtualProtect(func, steal, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), func, steal);
    return tramp;
}

struct CoverVecMS { const void** first; const void** last; const void** end; };

static void bai_set_ret(long long v)
{
    void* L = lua_state();
    if (!L) return;
    ((lua_pushinteger_t)(g_base + RVA_LUA_PUSHINTEGER))(L, v);
    ((lua_setfield_t)(g_base + RVA_LUA_SETFIELD))(L, LUA_GLOBALSINDEX, "bai_ret");
}

static void far_evaluator_setup(void* ev, const float* enemy, float mn, float mx,
                                float dev, const void* fn)
{
    ((setup_ce_base_t)(g_base + RVA_SETUP_CE_BASE))(ev, fn);
    char* e = (char*)ev;
    *(float*)(e + OFF_EV_ENEMY_POS + 0) = enemy[0];
    *(float*)(e + OFF_EV_ENEMY_POS + 4) = enemy[1];
    *(float*)(e + OFF_EV_ENEMY_POS + 8) = enemy[2];

    const float EPS = 0.00001f;
    unsigned char act = *(unsigned char*)(e + OFF_EV_ACTUALITY);
    if (act) {
        float d0 = *(float*)(e + OFF_EV_DEVIATION)    - dev;
        float d1 = *(float*)(e + OFF_EV_MIN_DISTANCE) - mn;
        float d2 = *(float*)(e + OFF_EV_MAX_DISTANCE) - mx;
        if (d0 < 0.0f) d0 = -d0;
        if (d1 < 0.0f) d1 = -d1;
        if (d2 < 0.0f) d2 = -d2;
        act = (unsigned char)((d0 < EPS && d1 < EPS && d2 < EPS) ? 1 : 0);
    }
    *(float*)(e + OFF_EV_DEVIATION)    = dev;
    *(float*)(e + OFF_EV_MIN_DISTANCE) = mn;
    *(float*)(e + OFF_EV_MAX_DISTANCE) = mx;
    *(unsigned char*)(e + OFF_EV_ACTUALITY) = act;
}

static void*    g_pass_member[4]   = {0, 0, 0, 0};
static void*    g_pass_manager[5]  = {0, 0, 0, 0, 0};
static void*    g_pass_location[1] = {0};
static void*    g_pass_agent[4]    = {0, 0, 0, 0};
static bool     g_pass_ready       = false;

static void build_pass_restrictor(void)
{
    if (g_pass_ready) return;
    g_pass_manager[4]  = (void*)&g_pass_member[0];
    g_pass_location[0] = (void*)&g_pass_manager[0];
    g_pass_agent[3]    = (void*)&g_pass_location[0];
    g_pass_ready = true;
}

static const void* hkAmbushCover(void* self, const float* pos, const float* enemy,
                                 float radius, float min_distance, const void* cb)
{
    if (!g_orig_ambush) return 0;
    if (radius >= 0.0f)
        return g_orig_ambush(self, pos, enemy, radius, min_distance, cb);

    const float packed = -radius;
    const int   mode   = (int)(packed / 1000.0f);
    const float r      = packed - (float)mode * 1000.0f;
    if (r <= 0.0f) return 0;

    void* go = *(void**)((char*)self + OFF_SGO_GAME_OBJECT);
    if (!go) return 0;

    if (mode == COVER_MODE_KILLER_ID) {
        void* alive = ((rtdcast_t)(g_base + RVA_RTDCAST))(
            go, 0, (void*)(g_base + RVA_RTTI_CGAMEOBJECT),
            (void*)(g_base + RVA_RTTI_CENTITY_ALIVE), 0);
        bai_set_ret(alive ? (long long)*(unsigned short*)((char*)alive + OFF_ENTITY_KILLER_ID)
                          : -1);
        return 0;
    }

    if (mode != COVER_MODE_CLOSE && mode != COVER_MODE_FAR) return 0;
    void* stalker = ((rtdcast_t)(g_base + RVA_RTDCAST))(
        go, 0, (void*)(g_base + RVA_RTTI_CGAMEOBJECT),
        (void*)(g_base + RVA_RTTI_CAI_STALKER), 0);
    if (!stalker) return 0;
    void* ev = *(void**)((char*)stalker + (mode == COVER_MODE_CLOSE
                                          ? OFF_STALKER_CE_CLOSE
                                          : OFF_STALKER_CE_FAR));
    if (!ev) return 0;

    struct CoverVecMS covers;
    covers.first = 0; covers.last = 0; covers.end = 0;

    void* fn[8];
    for (int i = 0; i < 8; ++i) fn[i] = 0;
    fn[0] = (void*)(g_base + RVA_COVER_LAMBDA_VFT);
    fn[1] = (void*)&covers;
    fn[7] = (void*)&fn[0];

    if (mode == COVER_MODE_CLOSE)
        ((setup_ce_close_t)(g_base + RVA_SETUP_CE_CLOSE))(
            ev, enemy, min_distance, 300.0f, 10.0f, (const void*)fn);
    else
        far_evaluator_setup(ev, enemy, min_distance, 300.0f, 10.0f, (const void*)fn);
    if (fn[7]) {
        void** vt = (void**)fn[0];
        ((func_delete_this_t)vt[4])((void*)&fn[0],
                                    (unsigned char)(fn[7] != (void*)&fn[0]));
    }

    void* aispace = ((ai_space_fn_t)(g_base + RVA_AI_SPACE_FN))();
    if (aispace) {
        void* mgr = *(void**)((char*)aispace + OFF_AISPACE_COVER_MGR);
        if (mgr && mode == COVER_MODE_CLOSE) {
            ((best_cover_close_t)(g_base + RVA_BEST_COVER_CLOSE))(mgr, pos, r, ev);
        } else if (mgr) {
            build_pass_restrictor();
            void* restr[3];
            restr[0] = stalker;
            restr[1] = (void*)&g_pass_agent[0];
            restr[2] = 0;
            ((best_cover_far_t)(g_base + RVA_BEST_COVER_FAR))(mgr, pos, r, ev,
                                                              (const void*)restr);
        }
    }

    const void* result = 0;
    if (covers.first && covers.last != covers.first) {
        const long n = (long)(covers.last - covers.first);
        if (cb) {
            void* L   = *(void**)((char*)cb + OFF_FUNCTOR_LUA_STATE);
            int   ref = *(int*)((char*)cb + OFF_FUNCTOR_REF);
            for (long i = n - 1; i >= 0; --i) {
                const void*  p     = covers.first[i];
                const void** tuple = &p;
                ((lua_rawgeti_t)(g_base + RVA_LUA_RAWGETI))(L, -10000, ref);
                ((push_cover_arg_t)(g_base + RVA_PUSH_COVER_ARG))(L, (void*)&tuple);
                if (((luabind_pcall_t)(g_base + RVA_LUABIND_PCALL))(L, 1, 1)) break;
                unsigned char* top = *(unsigned char**)((char*)L + OFF_LUA_STATE_TOP);
                unsigned long long tv = *(unsigned long long*)(top - 8);
                const int accepted = ((unsigned int)(tv >> 47) < 0xFFFFFFFEu);
                ((lua_settop2_t)(g_base + RVA_LUA_SETTOP2))(L, -2);
                if (accepted) { result = p; break; }
            }
        } else {
            result = covers.first[n - 1];
        }
    }

    if (covers.first) {
        void* blk = (void*)covers.first;
        if ((((char*)covers.end - (char*)covers.first) & ~(long long)7) >= 0x1000)
            blk = *((void**)covers.first - 1);
        ((mi_free_ex_t)(g_base + RVA_MI_FREE_EX))(blk, 0);
    }
    return result;
}

static void* ircv_active_item(void* self)
{
    if (!self) return 0;
    char* inv = *(char**)((char*)self + OFF_IRCV_INVENTORY);
    if (!inv) return 0;
    unsigned slot = *(unsigned*)(inv + OFF_INV_ACTIVE_SLOT);
    if (slot == 0xFF || slot >= INV_SLOT_MAX) return 0;
    char* first = *(char**)(inv + OFF_INV_SLOTS_FIRST);
    char* last = *(char**)(inv + OFF_INV_SLOTS_LAST);
    if (!first) return 0;
    char* cell = first + (size_t)slot * INV_SLOT_STRIDE;
    if (last && cell + INV_SLOT_STRIDE > last) return 0;
    return *(void**)(cell + OFF_INV_SLOT_ITEM);
}

static void hkActorKeyboardPress(void* self, unsigned cmd)
{
    g_actor_ircv = self;
    ensure_lua_api();
    if (g_torch_lua && (cmd == ACT_TORCH || cmd == ACT_TORCH_MODE) &&
        !ircv_active_item(self)) {
        ++g_torch_blocked_n;
        return;
    }
    if (g_orig_actor_kbpress)
        g_orig_actor_kbpress(self, cmd);
}

static void* install_detour(uintptr_t rva_begin, uintptr_t rva_end, void* hook)
{
    uint8_t* func = (uint8_t*)(g_base + rva_begin);
    const SIZE_T steal = (SIZE_T)(rva_end - rva_begin);
    if (steal < 14 || steal > 32) return 0;

    uint8_t* tramp = (uint8_t*)VirtualAlloc(0, steal + 14,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return 0;
    for (SIZE_T i = 0; i < steal; ++i) tramp[i] = func[i];
    tramp[steal + 0] = 0xFF; tramp[steal + 1] = 0x25;
    *(uint32_t*)(tramp + steal + 2) = 0;
    *(uint64_t*)(tramp + steal + 6) = (uint64_t)(func + steal);

    DWORD oldp;
    if (!VirtualProtect(func, steal, PAGE_EXECUTE_READWRITE, &oldp)) return 0;
    uint8_t patch[32];
    patch[0] = 0xFF; patch[1] = 0x25;
    *(uint32_t*)(patch + 2) = 0;
    *(uint64_t*)(patch + 6) = (uint64_t)hook;
    for (SIZE_T i = 14; i < steal; ++i) patch[i] = 0x90;
    for (SIZE_T i = 0; i < steal; ++i) func[i] = patch[i];
    VirtualProtect(func, steal, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), func, steal);
    return tramp;
}

static bool install_hook()
{
    g_base = (uintptr_t)GetModuleHandleW(NULL);
    if (!g_base) return false;

    g_orig = (execcmd_t)install_detour(RVA_EXECCMD, RVA_STEAL_END,
                                       (void*)&hkExecuteCommand);
    if (!g_orig) return false;

    g_orig_ro_spawn = (ro_spawn_t)install_detour(RVA_RO_NET_SPAWN, RVA_RO_NET_SPAWN_END,
                                                 (void*)&hkRestrictedNetSpawn);
    g_orig_restrict = (restrict_t)install_detour(RVA_RESTRICT, RVA_RESTRICT_END,
                                                 (void*)&hkRestrict);

    g_orig_run_atk = (run_atk_check_t)install_detour(RVA_RUN_ATK_CHECK, RVA_RUN_ATK_CHECK_END,
                                                     (void*)&hkRunAttackCheck);

    g_orig_ambush = (ambush_cover_t)install_detour(RVA_AMBUSH_COVER, RVA_AMBUSH_COVER_END,
                                                   (void*)&hkAmbushCover);

    g_orig_actor_render = (actor_render_t)install_detour(
        RVA_ACTOR_RENDER, RVA_ACTOR_RENDER_END, (void*)&hkActorRenderableRender);
    g_orig_actor_onhud = (actor_onhuddraw_t)install_onhuddraw_detour(
        (void*)&hkActorOnHUDDraw);
    g_orig_hud_renderui = (hud_renderui_t)install_detour(
        RVA_HUD_RENDERUI, RVA_HUD_RENDERUI_END, (void*)&hkHudRenderUI);
    pda3d_hack_install();
    g_orig_set_main_ir = (set_main_ir_t)install_detour(
        RVA_SET_MAIN_IR, RVA_SET_MAIN_IR_END, (void*)&hkSetMainInputReceiver);
    g_orig_inv_action = (inv_action_t)install_detour(
        RVA_INV_ACTION, RVA_INV_ACTION_END, (void*)&hkInvAction);

    {
        uint8_t* f = (uint8_t*)(g_base + RVA_FIRETRACE);
        if (f[0] == 0x48 && f[1] == 0x8B && f[2] == 0xC4 &&
            f[3] == 0x53 && f[4] == 0x55 && f[5] == 0x56 && f[6] == 0x57 &&
            f[7] == 0x41 && f[8] == 0x54 &&
            f[15] == 0x48 && f[16] == 0x81 && f[17] == 0xEC)
            g_orig_fire_trace = (fire_trace_t)install_detour(
                RVA_FIRETRACE, RVA_FIRETRACE_END, (void*)&hkFireTrace);

        uint8_t* g = (uint8_t*)(g_base + RVA_GL_ONSHOT);
        if (g[0] == 0x40 && g[1] == 0x56 &&
            g[2] == 0x48 && g[3] == 0x83 && g[4] == 0xEC && g[5] == 0x40 &&
            g[6] == 0x80 && g[7] == 0xB9)
            g_orig_gl_onshot = (gl_onshot_t)install_detour(
                RVA_GL_ONSHOT, RVA_GL_ONSHOT_END, (void*)&hkGrenadeOnShot);
    }

    {
        const IMAGE_DOS_HEADER* dos = (const IMAGE_DOS_HEADER*)g_base;
        const IMAGE_NT_HEADERS64* nt = dos->e_magic == IMAGE_DOS_SIGNATURE
            ? (const IMAGE_NT_HEADERS64*)(g_base + dos->e_lfanew) : 0;
        static const uint8_t expected[] = {
            0x40, 0x53, 0x55, 0x56, 0x57, 0x41, 0x55,
            0x48, 0x81, 0xEC, 0x10, 0x01, 0x00, 0x00
        };
        const uint8_t* entry = (const uint8_t*)(g_base + RVA_HUD_PLAYMOTION);
        if (nt && nt->Signature == IMAGE_NT_SIGNATURE &&
            nt->FileHeader.TimeDateStamp == KNIFE_ENGINE_STAMP &&
            nt->OptionalHeader.SizeOfImage == KNIFE_ENGINE_IMAGE &&
            sizeof(expected) == RVA_HUD_PLAYMOTION_END - RVA_HUD_PLAYMOTION &&
            memcmp(entry, expected, sizeof(expected)) == 0) {
            g_orig_hud_playmotion = (hud_playmotion_t)install_detour(
                RVA_HUD_PLAYMOTION, RVA_HUD_PLAYMOTION_END, (void*)&hkHudPlayMotion);
        }
        ((msg_t)(g_base + RVA_MSG))("~ [knife_combo] hook %s",
            g_orig_hud_playmotion ? "ready" : "unavailable for this engine build");
    }

    {
        uint8_t* f = (uint8_t*)(g_base + RVA_ACTOR_KEYPRESS);
        if (f[0] == 0x40 && f[1] == 0x55 && f[2] == 0x57 && f[3] == 0x41 &&
            f[4] == 0x56 && f[5] == 0x48 && f[6] == 0x8D && f[7] == 0x6C)
            g_orig_actor_kbpress = (actor_kbpress_t)install_detour(
                RVA_ACTOR_KEYPRESS, RVA_ACTOR_KEYPRESS_END,
                (void*)&hkActorKeyboardPress);
    }

    {
        uint8_t* p = (uint8_t*)(g_base + RVA_ROTJUMP_NULLWRITE);
        if (p[0] == 0x33 && p[1] == 0xC0) {
            DWORD oldp;
            if (VirtualProtect(p, 2, PAGE_EXECUTE_READWRITE, &oldp)) {
                p[0] = 0xEB;
                p[1] = 0x03;
                VirtualProtect(p, 2, oldp, &oldp);
                FlushInstructionCache(GetCurrentProcess(), p, 2);
            }
        }
    }

    bp_set_rotfix(1);

    {
        uint8_t* p = (uint8_t*)(g_base + RVA_ALIFE_NOWAY);
        if (p[0] == 0x45 && p[1] == 0x8B && p[2] == 0xC2 &&
            p[3] == 0x48 && p[4] == 0x8D && p[5] == 0x15 &&
            p[10] == 0x48 && p[11] == 0x8D && p[12] == 0x0D &&
            p[17] == 0xE8 &&
            p[22] == 0x41 && p[23] == 0x0F && p[24] == 0x28 && p[25] == 0xFB) {
            const size_t n = 26;
            DWORD oldp;
            if (VirtualProtect(p, n, PAGE_EXECUTE_READWRITE, &oldp)) {
                int32_t rel = (int32_t)(RVA_ALIFE_DROPPATH - (RVA_ALIFE_NOWAY + 5));
                p[0] = 0xE9;
                p[1] = (uint8_t)(rel & 0xFF);
                p[2] = (uint8_t)((rel >> 8) & 0xFF);
                p[3] = (uint8_t)((rel >> 16) & 0xFF);
                p[4] = (uint8_t)((rel >> 24) & 0xFF);
                for (size_t i = 5; i < n; ++i) p[i] = 0x90;
                VirtualProtect(p, n, oldp, &oldp);
                FlushInstructionCache(GetCurrentProcess(), p, n);
                g_alife_noway_patched = 1;
            }
        }
    }

    {
        uint8_t* f = (uint8_t*)(g_base + RVA_SR_GAME_OBJECT1);
        if (f[0] == 0x48 && f[1] == 0x89 && f[2] == 0x5C && f[3] == 0x24 && f[4] == 0x18 &&
            f[5] == 0x55 && f[6] == 0x56 && f[7] == 0x57 &&
            f[16] == 0x48 && f[17] == 0x8D && f[18] == 0x6C && f[19] == 0x24 && f[20] == 0xC0)
            g_orig_sr_go1 = (sr_go1_t)install_detour(
                RVA_SR_GAME_OBJECT1, RVA_SR_GAME_OBJECT1_END,
                (void*)&hkScriptRegisterGameObject1);
    }

    g_orig_additem = (additem_t)install_additem_detour((void*)&hkListBoxAddItem);
    return true;
}

typedef HRESULT (WINAPI *DI8C_t)(void* hinst, DWORD ver, void* riid, void** out, void* unk);
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectInput8Create(void* hinst, DWORD ver, void* riid, void** out, void* unk)
{
    static DI8C_t real = 0;
    static int hooks_done = 0;
    if (!hooks_done) {
        hooks_done = 1;
        install_hook();
    }
    if (!real) {
        char path[MAX_PATH];
        UINT n = GetSystemDirectoryA(path, MAX_PATH);
        lstrcpyA(path + n, "\\dinput8.dll");
        HMODULE m = LoadLibraryA(path);
        if (m) real = (DI8C_t)GetProcAddress(m, "DirectInput8Create");
    }
    if (!real) return E_FAIL;
    return real(hinst, ver, riid, out, unk);
}

BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(h);
    return TRUE;
}
