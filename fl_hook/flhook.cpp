#include <windows.h>
#include <stdint.h>

#define RVA_EXECCMD         0x000837a0ULL
#define RVA_STEAL_END       0x000837b2ULL
#define RVA_LOADBUF         0x00c1fcb0ULL
#define RVA_PUSHSTR         0x00c18fb0ULL
#define RVA_PCALL           0x00c1a260ULL
#define RVA_PRINTOUT        0x0042a9a0ULL
#define RVA_GAISPACE        0x01203a18ULL
#define RVA_RELOADINI       0x0077cf30ULL
#define RVA_MSG             0x00090140ULL
#define RVA_XR_FS           0x01224098ULL
#define RVA_UPDATE_PATH     0x000A2F30ULL
#define RVA_FILE_LIST_OPEN  0x000A0290ULL
#define RVA_FILE_LIST_CLOSE 0x000A0730ULL
#define RVA_R_OPEN          0x000A1C20ULL
#define RVA_R_CLOSE         0x000A2580ULL
#define OFF_SCRENG          0x38
#define OFF_LUAVM           0x98

#define RVA_RESTRICT          0x00421D50ULL
#define RVA_RESTRICT_END      0x00421D63ULL
#define RVA_RO_NET_SPAWN      0x006E7620ULL
#define RVA_RO_NET_SPAWN_END  0x006E762EULL
#define RVA_GPGAMELEVEL       0x011FACA8ULL
#define RVA_SHARED_SET        0x0000B020ULL

#define RVA_RUN_ATK_CHECK     0x00AD1A10ULL
#define RVA_RUN_ATK_CHECK_END 0x00AD1A20ULL
#define RVA_DEVICE_TIME       0x0122378CULL

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
#define STRV_VALUE          20

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
typedef void* (*shared_set_t)(void* dst, const char* s);
typedef unsigned char (*run_atk_check_t)(void* self);
typedef void (*actor_render_t)(void* self, unsigned context_id, void* root);
typedef void (*actor_onhuddraw_t)(void* self, void* hud, unsigned context_id, void* root);
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

static uintptr_t g_base = 0;
static execcmd_t g_orig = 0;
static void* g_console = 0;
static restrict_t g_orig_restrict = 0;
static ro_spawn_t g_orig_ro_spawn = 0;
static run_atk_check_t g_orig_run_atk = 0;
static actor_render_t g_orig_actor_render = 0;
static actor_onhuddraw_t g_orig_actor_onhud = 0;
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

static int   g_boar_ok = 0;
static int   g_boar_fail = 0;
static float g_boar_last_dist = -1.f;
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

static void* g_brz_src = (void*)~(uintptr_t)0;
static void* g_brz_dst = 0;

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
    if (self) {
        void* obj = *(void**)((char*)self + OFF_RO_OBJECT);
        const char* nm = obj_name(obj);
        if (nm && brz_name_match(nm)) {
            unsigned short id = *(unsigned short*)((char*)obj + OFF_OBJ_ID);
            brz_remember(id);
            log_fmt1("~ [brz] tracking [%s]", nm);
        }
    }
    return g_orig_ro_spawn(self, data);
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
            "nomatch=%d last_state=%d nomodel=%d",
            g_wm_calls, g_wm_applied, g_wm_skipped, g_wm_skipped_model,
            g_wm_mismatch, g_wm_last_state, g_wm_nomodel);
        return;
    }

    if (*a != '0' && *a != '1') {
        Msg("!! [wm] usage: wm <0|1> <nchildren> <idx> [idx ...]  |  wm");
        return;
    }
    bool state = (*a == '1');
    ++a;

    while (*a == ' ' || *a == '\t') ++a;
    if (*a < '0' || *a > '9') {
        Msg("!! [wm] usage: wm <0|1> <nchildren> <idx> [idx ...]  |  wm");
        return;
    }
    unsigned want = 0;
    while (*a >= '0' && *a <= '9') { want = want * 10u + (unsigned)(*a - '0'); ++a; }

    if (!k1 && !k2) {
        if (++g_wm_nomodel <= 3)
            Msg("!! [wm] no hands model (g_player_hud/m_model empty), n=%d", g_wm_nomodel);
        return;
    }

    unsigned n1 = wm_child_count(k1), n2 = wm_child_count(k2);
    void* t1 = (want && n1 != want) ? 0 : k1;
    void* t2 = (want && n2 != want) ? 0 : k2;
    if ((k1 && !t1) || (k2 && !t2)) ++g_wm_skipped_model;
    if (!t1 && !t2) {
        if (++g_wm_mismatch <= 3)
            Msg("!! [wm] no model with %u children: m_model=%u (%s) m_model_2=%u (%s)",
                want, n1, wm_visual_name(0), n2, wm_visual_name(1));
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

        if (p[0] == 'w' && p[1] == 'm' &&
            (p[2] == 0 || p[2] == ' ' || p[2] == '\t')) {
            cmd_wm(p + 2);
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
    if (g_base && g_bp_ui) {
        void** pph = (void**)(g_base + RVA_G_PLAYER_HUD);
        void* ph = pph ? *pph : 0;
        if (ph) {
            ++g_skip_hands_n;
            render_script_hud_item_only(ph, context_id, root);
            return;
        }
    }
    if (g_orig_actor_onhud)
        g_orig_actor_onhud(self, hud, context_id, root);
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

    g_orig_actor_render = (actor_render_t)install_detour(
        RVA_ACTOR_RENDER, RVA_ACTOR_RENDER_END, (void*)&hkActorRenderableRender);
    g_orig_actor_onhud = (actor_onhuddraw_t)install_onhuddraw_detour(
        (void*)&hkActorOnHUDDraw);

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
