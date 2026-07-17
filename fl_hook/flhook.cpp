#include <windows.h>
#include <stdint.h>

#define RVA_EXECCMD   0x000837a0ULL
#define RVA_STEAL_END 0x000837b2ULL
#define RVA_LOADBUF   0x00c1fcb0ULL
#define RVA_PUSHSTR   0x00c18fb0ULL
#define RVA_PCALL     0x00c1a260ULL
#define RVA_PRINTOUT  0x0042a9a0ULL
#define RVA_GAISPACE  0x01203a18ULL
#define OFF_SCRENG    0x38
#define OFF_LUAVM     0x98

typedef int  (*loadbuf_t)(void* L, const char* buf, size_t sz, const char* name, const char* mode);
typedef const char* (*pushstr_t)(void* L, const char* s);
typedef int  (*pcall_t)(void* L, int nargs, int nresults, int errfunc);
typedef char (*printout_t)(void* L, const char* name, int status);
typedef void (*execcmd_t)(void* self, const char* cmd, char record, char allow_disabled);

static uintptr_t g_base = 0;
static execcmd_t g_orig = 0;

static const char BOOT[] =
    "local a=... local i=a:match(\"^%s*([%w_.]+)%s*$\") "
    "if i then local b=i:gsub(\"%.script$\",\"\") "
    "local m,f=b:match(\"^([%w_]+)%.([%w_]+)$\") "
    "if f then _G[m][f]() else return _G[b] end "
    "else assert(loadstring(a))() end";

static inline void* lua_state()
{
    uintptr_t* pAi = (uintptr_t*)(g_base + RVA_GAISPACE);
    if (!pAi) return 0;
    uintptr_t ai = *pAi;
    if (!ai) return 0;
    uintptr_t se = *(uintptr_t*)(ai + OFF_SCRENG);
    if (!se) return 0;
    return *(void**)(se + OFF_LUAVM);
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

static void hkExecuteCommand(void* self, const char* cmd, char record, char allow)
{
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
