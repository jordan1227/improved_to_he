$ErrorActionPreference = 'Stop'
if ([IntPtr]::Size -ne 4) {
    throw 'Run this script using C:\Windows\SysWOW64\WindowsPowerShell\v1.0\powershell.exe (32-bit Lua DLL).'
}
$previousDirectory = [Environment]::CurrentDirectory
$repositoryDirectory = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
[Environment]::CurrentDirectory = $repositoryDirectory
$code = @"
using System;
using System.Runtime.InteropServices;
public class PdaLuaTest {
 const string D = @"C:\Games\NLC_OGSR_HE files\Tools\AXRToolset-master\bin\lua51.dll";
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern IntPtr luaL_newstate();
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern void luaL_openlibs(IntPtr l);
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern int luaL_loadfile(IntPtr l,string f);
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern int lua_pcall(IntPtr l,int a,int r,int e);
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern IntPtr lua_tolstring(IntPtr l,int i,IntPtr n);
 [DllImport(D, CallingConvention=CallingConvention.Cdecl)] static extern void lua_close(IntPtr l);
 public static void Run(string file) {
  IntPtr l=luaL_newstate(); luaL_openlibs(l);
  try { int rc=luaL_loadfile(l,file); if(rc==0) rc=lua_pcall(l,0,0,0); if(rc!=0) throw new Exception(Marshal.PtrToStringAnsi(lua_tolstring(l,-1,IntPtr.Zero))); }
  finally {lua_close(l);}
 }
}
"@
try {
    Add-Type -TypeDefinition $code
    [PdaLuaTest]::Run('docs/tests/pda_state_tests.lua')
} finally {
    [Environment]::CurrentDirectory = $previousDirectory
}
