///////////////////////////////////////////////////////////////////////
//
// When multiple versions of maya are installed, maya2egg can
// accidentally use the wrong version of the OpenMaya libraries.
// This small wrapper program alters your PATH, MAYA_LOCATION, etc
// environment variables in order to ensure that maya2egg finds the
// right ligraries.
//
// To use this wrapper, maya2egg must be renamed to maya2egg-wrapped.
// Then, this wrapper program must be installed as maya2egg.
//
///////////////////////////////////////////////////////////////////////


#ifndef MAYAVERSION
#error You must define the symbol MAYAVERSION when compiling mayawrapper.
#endif

#define QUOTESTR(x) #x
#define TOSTRING(x) QUOTESTR(x)

#define _CRT_SECURE_NO_DEPRECATE 1

#ifdef _WIN32
  #include <windows.h>
  #include <winuser.h>
  #include <process.h>
#else
  #include <string.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <unistd.h>
  #define _putenv putenv
#endif
#ifdef __APPLE__
  #include <sys/malloc.h>
#else
  #include <malloc.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#define PATH_MAX 1024

#ifdef __APPLE__
  // This is for _NSGetExecutablePath().
  #include <mach-o/dyld.h>
#endif

struct { char *ver, *key; } maya_versions[] = {
  { "MAYA6",    "6.0" },
  { "MAYA65",   "6.5" },
  { "MAYA7",    "7.0" },
  { "MAYA8",    "8.0" },
  { "MAYA85",   "8.5" },
  { "MAYA2008", "2008" },
  { "MAYA2009", "2009" },
  { 0, 0 },
};

char *getVersionNumber(char *ver) {
  for (int i=0; maya_versions[i].ver != 0; i++) {
    if (strcmp(maya_versions[i].ver, ver)==0) {
      return maya_versions[i].key;
    }
  }
  return 0;
}

#if defined(_WIN32)
void getMayaLocation(char *ver, char *loc)
{
  char fullkey[1024], *developer;
  HKEY hkey; DWORD size, dtype; LONG res; int dev, hive;

  for (dev=0; dev<3; dev++) {
    switch (dev) {
    case 0: developer="Alias|Wavefront"; break;
    case 1: developer="Alias"; break;
    case 2: developer="Autodesk"; break;
    }
    sprintf(fullkey, "SOFTWARE\\%s\\Maya\\%s\\Setup\\InstallPath", developer, ver);
    for (hive=0; hive<2; hive++) {
      loc[0] = 0;
      res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, fullkey, 0, KEY_READ | (hive ? 256:0), &hkey);
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

#elif defined(__APPLE__)
void getMayaLocation(char *ver, char *loc)
{
  char mpath[64];
  sprintf(mpath, "/Applications/Autodesk/maya%s/Maya.app/Contents", ver);
  struct stat st;
  if(stat(mpath, &st) == 0) {
    strcpy(loc, mpath);
  } else {
    loc[0] = 0;
  }
}

void getWrapperName(char *prog)
{
  char *pathbuf = new char[PATH_MAX];
  uint32_t bufsize = PATH_MAX;
  if (_NSGetExecutablePath(pathbuf, &bufsize) == 0) {
    strcpy(prog, pathbuf);
  } else {
    prog[0] = 0;
  }
  delete[] pathbuf;
}

#else
void getMayaLocation(char *ver, char *loc)
{
  char mpath[64];
  sprintf(mpath, "/usr/autodesk/maya%s", ver);
  struct stat st;
  if(stat(mpath, &st) == 0) {
    strcpy(loc, mpath);
  } else {
    sprintf(mpath, "/usr/aw/maya%s", ver);
    if(stat(mpath, &st) == 0) {
      strcpy(loc, mpath);
    } else {
      loc[0] = 0;
    }
  }
}

void getWrapperName(char *prog)
{
  char readlinkbuf[PATH_MAX];
  int pathlen = readlink("/proc/self/exe", readlinkbuf, PATH_MAX-1);
  if (pathlen > 0) {
    readlinkbuf[pathlen] = 0;
    strcpy(prog, readlinkbuf);
  } else {
    prog[0] = 0;
  }
}
#endif

int main(int argc, char **argv)
{
  char loc[1024], prog[1024];
  char *key, *path, *env1, *env2, *env3, *env4; int len;
  
  key = getVersionNumber(TOSTRING(MAYAVERSION));
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
  
#ifdef _WIN32
  strcat(prog, "-wrapped.exe");
#else
  strcat(prog, "-wrapped");
#endif

#ifdef _WIN32
  path = getenv("PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "PATH=%s\\bin;%s", loc, path);
  env2 = (char*)malloc(100 + strlen(loc));
  sprintf(env2, "MAYA_LOCATION=%s", loc);
  env3 = (char*)malloc(300 + 5*strlen(loc));
  sprintf(env3, "PYTHONPATH=%s\\bin;%s\\Python;%s\\Python\\DLLs;%s\\Python\\lib;%s\\Python\\lib\\site-packages", loc, loc, loc, loc, loc);
#else
#ifdef __APPLE__
  path = getenv("DYLD_LIBRARY_PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "DYLD_LIBRARY_PATH=%s/MacOS:%s", loc, path);
#else
  path = getenv("LD_LIBRARY_PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "LD_LIBRARY_PATH=%s/lib:%s", loc, path);
#endif // __APPLE__
  env2 = (char*)malloc(100 + strlen(loc));
  sprintf(env2, "MAYA_LOCATION=%s", loc);
  env3 = (char*)malloc(100 + strlen(loc));
  sprintf(env3, "PYTHONHOME=%s/Frameworks/Python.framework/Versions/Current", loc);
  
  _putenv(env3);
#endif // _WIN32
  _putenv(env1);
  _putenv(env2);
  //  _putenv(env3);
  
#ifdef _WIN32
  STARTUPINFO si; PROCESS_INFORMATION pi;
  char *cmd;
  
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
#else
  if (execvp(prog, argv) == 0) {
    exit(0);
  } else {
    printf("Could not launch %s\n", prog);
    exit(1);
  }
#endif
}

