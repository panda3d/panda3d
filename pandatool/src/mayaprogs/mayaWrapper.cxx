///////////////////////////////////////////////////////////////////////
//
// Under windows, when multiple versions of maya are installed, 
// maya2egg can accidentally use the wrong version of OpenMaya.dll.
// This small wrapper program alters your PATH and MAYA_LOCATION
// environment variables in order to ensure that maya2egg finds the
// right DLLs.
//
// To use this wrapper, maya2egg.exe must be renamed to 
// maya2egg-wrapped.exe.  Then, this wrapper program must be
// installed as maya2egg.exe
//
///////////////////////////////////////////////////////////////////////


#ifndef MAYAVERSION
#error You must define the symbol MAYAVERSION when compiling mayawrapper.
#endif

#define QUOTESTR(x) #x
#define TOSTRING(x) QUOTESTR(x)

#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <stdio.h>
#include <signal.h>
#define PATH_MAX 1024

struct { char *ver, *key; } reg_keys[] = {
  { "MAYA6",    "SOFTWARE\\Alias|Wavefront\\Maya\\6.0\\Setup\\InstallPath" },
  { "MAYA65",   "SOFTWARE\\Alias|Wavefront\\Maya\\6.5\\Setup\\InstallPath" },
  { "MAYA7",    "SOFTWARE\\Alias|Wavefront\\Maya\\7.0\\Setup\\InstallPath" },
  { "MAYA8",    "SOFTWARE\\Alias\\Maya\\8.0\\Setup\\InstallPath" },
  { "MAYA85",   "SOFTWARE\\Alias\\Maya\\8.5\\Setup\\InstallPath" },
  { "MAYA2008", "SOFTWARE\\Autodesk\\Maya\\2008\\Setup\\InstallPath" },
  { 0, 0 },
};

char *getRegistryKey(char *ver) {
  for (int i=0; reg_keys[i].ver != 0; i++) {
    if (strcmp(reg_keys[i].ver, ver)==0) {
      return reg_keys[i].key;
    }
  }
  return 0;
}

void getMayaLocation(char *key, char *loc)
{
  HKEY hkey; DWORD size, dtype; LONG res;

  loc[0] = 0;
  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hkey);
  if (res == ERROR_SUCCESS) {
    size=1024;
    res = RegQueryValueEx(hkey, "MAYA_INSTALL_LOCATION", NULL, &dtype, (LPBYTE)loc, &size);
    if ((res == ERROR_SUCCESS)&&(dtype == REG_SZ)) {
      loc[size] = 0;
      return;
    } else {
      loc[0] = 0;
    }
    RegCloseKey(hkey);
  }
}

void getWrapperName(char *prog)
{
  DWORD res;
  res = GetModuleFileName(NULL, prog, 1000);
  if (res == 0) {
    prog[0] = 0;
    return;
  }
  int len = strlen(prog);
  if (_stricmp(prog+len-4, ".exe")) {
    prog[0] = 0;
    return;
  }
  prog[len-4] = 0;
}

int main(int argc, char **argv)
{
  char loc[1024], prog[1024];
  char *key, *cmd, *path, *env1, *env2; int len;
  STARTUPINFO si; PROCESS_INFORMATION pi;
  
  key = getRegistryKey(TOSTRING(MAYAVERSION));
  if (key == 0) {
    printf("MayaWrapper: unknown maya version %s\n", TOSTRING(MAYAVERSION));
    exit(1);
  }
  getMayaLocation(key, loc);
  if (loc[0]==0) {
    printf("Cannot locate %s - it does not appear to be installed\n", TOSTRING(MAYAVERSION));
    exit(1);
  }

  getWrapperName(prog);
  if (prog[0]==0) {
    printf("mayaWrapper cannot determine its own filename (bug)\n");
    exit(1);
  }
  strcat(prog, "-wrapped.exe");

  path = getenv("PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "PATH=%s\\bin;%s", loc, path);
  env2 = (char*)malloc(100 + strlen(loc));
  sprintf(env2, "MAYA_LOCATION=%s", loc);
  _putenv(env1);
  _putenv(env2);

  cmd = GetCommandLine();
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(STARTUPINFO);
  if (CreateProcess(prog, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    WaitForSingleObject(pi.hProcess, INFINITE);
    exit(0);
  } else {
    printf("Could not launch %s\n", prog);
    exit(1);
  }
}

