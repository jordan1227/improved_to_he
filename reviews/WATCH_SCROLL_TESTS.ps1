Add-Type @"
using System; using System.Runtime.InteropServices;
public static class WL {
[DllImport(@"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll",CallingConvention=CallingConvention.Cdecl)] public static extern IntPtr luaL_newstate();
[DllImport(@"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll",CallingConvention=CallingConvention.Cdecl)] public static extern void luaL_openlibs(IntPtr L);
[DllImport(@"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll",CallingConvention=CallingConvention.Cdecl)] public static extern int luaL_loadstring(IntPtr L,string s);
[DllImport(@"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll",CallingConvention=CallingConvention.Cdecl)] public static extern int lua_pcall(IntPtr L,int a,int b,int c);
[DllImport(@"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll",CallingConvention=CallingConvention.Cdecl)] public static extern IntPtr lua_tolstring(IntPtr L,int i,IntPtr n);
}
"@
$l=[WL]::luaL_newstate();[WL]::luaL_openlibs($l)
$source="dofile([[C:/Games/NLC_OGSR_HE files/improved_to_he/reviews/WATCH_SCROLL_TESTS.lua]])"
$rc=[WL]::luaL_loadstring($l,$source)
if($rc -eq 0){$rc=[WL]::lua_pcall($l,0,0,0)}
if($rc -ne 0){throw [Runtime.InteropServices.Marshal]::PtrToStringAnsi([WL]::lua_tolstring($l,-1,[IntPtr]::Zero))}