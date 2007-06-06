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

#define _CRT_SECURE_NO_DEPRECATE 1

#include <windows.h>
#include <winuser.h>
#include <stdlib.h>
#include <process.h>
#include <malloc.h>
#include <stdio.h>
#include <signal.h>
#define PATH_MAX 1024

void getMayaLocation(int major, int minor, char *loc)
{
  char key[1024]; HKEY hkey;
  DWORD size, dtype; LONG res; int retry;
  loc[0] = 0;

  for (retry=0; retry<3; retry++) {
    char *developer;
    switch(retry) {
    case 0: developer = "Alias|Wavefront"; break;
    case 1: developer = "Alias"; break;
    case 2: developer = "Autodesk"; break;
    }

    sprintf(key, "SOFTWARE\\%s\\Maya\\%d.%d\\Setup\\InstallPath", developer, major, minor);

    res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_READ, &hkey);
    if (res == ERROR_SUCCESS) {
      size=1024;
      res = RegQueryValueEx(hkey, "MAYA_INSTALL_LOCATION", NULL, &dtype, (LPBYTE)loc, &size);
      if ((res == ERROR_SUCCESS)&&(dtype == REG_SZ)) {
        loc[size] = 0;
        break;
      } else {
        loc[0] = 0;
      }
      RegCloseKey(hkey);
    }
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
  int major = MAYAVERSION / 10;
  int minor = MAYAVERSION - major * 10;
  char loc[1024], prog[1024];
  char *cmd, *path, *env1, *env2; int len;
  STARTUPINFO si; PROCESS_INFORMATION pi;

  if (major == 0) {
    major = minor;
    minor = 0;
  }
  
  getMayaLocation(major, minor, loc);
  if (loc[0]==0) {
    printf("Cannot locate Maya %d.%d (registry key missing)\n", major, minor);
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

