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

static uintptr_t g_base = 0;
static execcmd_t g_orig = 0;
static void* g_console = 0;

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
    }
    g_orig(self, cmd, record, allow);
}

static bool install_hook()
{
    g_base = (uintptr_t)GetModuleHandleW(NULL);
    if (!g_base) return false;

    uint8_t* func  = (uint8_t*)(g_base + RVA_EXECCMD);
    const SIZE_T steal = (SIZE_T)(RVA_STEAL_END - RVA_EXECCMD);

    uint8_t* tramp = (uint8_t*)VirtualAlloc(0, steal + 14,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!tramp) return false;
    for (SIZE_T i = 0; i < steal; ++i) tramp[i] = func[i];
    tramp[steal + 0] = 0xFF; tramp[steal + 1] = 0x25;
    *(uint32_t*)(tramp + steal + 2) = 0;
    *(uint64_t*)(tramp + steal + 6) = (uint64_t)(func + steal);
    g_orig = (execcmd_t)tramp;

    DWORD oldp;
    if (!VirtualProtect(func, steal, PAGE_EXECUTE_READWRITE, &oldp)) return false;
    uint8_t patch[18];
    patch[0] = 0xFF; patch[1] = 0x25;
    *(uint32_t*)(patch + 2) = 0;
    *(uint64_t*)(patch + 6) = (uint64_t)&hkExecuteCommand;
    for (SIZE_T i = 14; i < steal; ++i) patch[i] = 0x90;
    for (SIZE_T i = 0; i < steal; ++i) func[i] = patch[i];
    VirtualProtect(func, steal, oldp, &oldp);
    FlushInstructionCache(GetCurrentProcess(), func, steal);
    return true;
}

typedef HRESULT (WINAPI *DI8C_t)(void* hinst, DWORD ver, void* riid, void** out, void* unk);
extern "C" __declspec(dllexport)
HRESULT WINAPI DirectInput8Create(void* hinst, DWORD ver, void* riid, void** out, void* unk)
{
    static DI8C_t real = 0;
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

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) {
        install_hook();
    }
    return TRUE;
}
