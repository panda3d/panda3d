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
#include <limits.h>
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
  { "MAYA2010", "2010" },
  { "MAYA2011", "2011"},
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
#if __WORDSIZE == 64
  sprintf(mpath, "/usr/autodesk/maya%s-x64", ver);
#else
  sprintf(mpath, "/usr/autodesk/maya%s", ver);
#endif
  struct stat st;
  if(stat(mpath, &st) == 0) {
    strcpy(loc, mpath);
  } else {
#if __WORDSIZE == 64
    sprintf(mpath, "/usr/aw/maya%s-x64", ver);
#else
    sprintf(mpath, "/usr/aw/maya%s", ver);
#endif
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
  char loc[PATH_MAX], prog[PATH_MAX];
  char *key, *path, *env1, *env2, *env3, *env4;
  int nLocLen;
  
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

  // "loc" == MAYA_LOCATION
  // Now set PYTHONHOME & PYTHONPATH.  Maya requires this to be
  // set and pointing within MAYA_LOCATION, or it might get itself
  // confused with another Python installation (e.g. Panda's).
  // Finally, prepend PATH with MAYA_LOCATION\bin; as well.

// As of Sept. 2009, at least some WIN32 platforms had 
// much difficulty with Maya 2009 egging, e.g., see forums:
// http://www.panda3d.org/phpbb2/viewtopic.php?p=42790 
// 
// Historically:
// http://www.panda3d.org/phpbb2/viewtopic.php?t=3842 
// http://www.panda3d.org/phpbb2/viewtopic.php?t=6468 
// http://www.panda3d.org/phpbb2/viewtopic.php?t=6533 
// http://www.panda3d.org/phpbb2/viewtopic.php?t=5070 
// 
// Hoped solution:  carry over code that was in mayapath.cxx 
// and use that here to set 4 important environment variables:
// MAYA_LOCATION
// PYTHONPATH
// PYTHONHOME
// PATH (add Maya bin to start of this)
// BUT... mayapath.cxx makes use of FILENAME and other code
// from the rest of the Panda build, so for now, to keep this
// wrapper thinner, just correct/verify that the latter 3 environment
// variables are set properly (as they are in mayapath.cxx)
// for use with Maya under WIN32.
// FIRST TRY, keeping PYTHONPATH simple, as just loc\Python, failed.
// SECOND TRY, as coded formerly here (in mayaWrapper.cxx), also fails:
//   PYTHONPATH=%s\\bin;%s\\Python;%s\\Python\\DLLs;%s\\Python\\lib;%s\\Python\\lib\\site-packages
// Eventually, solution was found that has AT MOST this (which does NOT match mayapath.cxx....):
//   PYTHONPATH=%s\\bin\\python25.zip;%s\\Python\\DLLs;%s\\Python\\lib;%s\\Python\\lib\\plat-win;%s\\Python\\lib\\lib-tk;%s\\bin;%s\\Python;%s\\Python\\lib\\site-packages", loc, loc, loc, loc, loc, loc, loc, loc);
// One attempt to thin down to just the .zip file and the site-packages file works!  This seems to be minimum needed 
// as removing the .zip file mentioned first will then break again with the dreaded:
// "Invalid Python Environment: Python is unable to find Maya's Python modules"
// Again, this minimal necessary set (for Maya 2009 32-bit at least) does NOT match mayapath.cxx....):
//   PYTHONPATH=%s\\bin\\python25.zip;%s\\Python\\lib\\site-packages", loc, loc);
// 

#ifdef _WIN32
  // Not sure of non-WIN32 environments, but for WIN32,
  // verify that terminating directory/folder separator
  // character \ is NOT found at end of "loc" string:
  nLocLen = strlen(loc);
  if (nLocLen > 0 && loc[nLocLen - 1] == '\\')
  {
    loc[nLocLen - 1] = '\0';
  }
  path = getenv("PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "PATH=%s\\bin;%s", loc, path);
  env2 = (char*)malloc(100 + strlen(loc));
  sprintf(env2, "MAYA_LOCATION=%s", loc);
  env3 = (char*)malloc(100 + strlen(loc));
  sprintf(env3, "PYTHONHOME=%s\\Python", loc);
  env4 = (char*)malloc(100 + 2*strlen(loc));
  // FYI,  background on what does Maya (e.g., Maya 2009) expect
  // in PYTHONPATH by doing a check of sys.paths in Python
  // as discussed in 
  // http://www.rtrowbridge.com/blog/2008/11/27/maya-python-import-scripts/
  // gives this:
  // C:\Program Files\Autodesk\Maya2009\bin\python25.zip
  // C:\Program Files\Autodesk\Maya2009\Python\DLLs
  // C:\Program Files\Autodesk\Maya2009\Python\lib
  // C:\Program Files\Autodesk\Maya2009\Python\lib\plat-win
  // C:\Program Files\Autodesk\Maya2009\Python\lib\lib-tk
  // C:\Program Files\Autodesk\Maya2009\bin
  // C:\Program Files\Autodesk\Maya2009\Python
  // C:\Program Files\Autodesk\Maya2009\Python\lib\site-packages
  // ...
  // Experimenting and a check of 
  // http://www.panda3d.org/phpbb2/viewtopic.php?t=3842
  // leads to these 2 items being necessary and hopefully sufficient:
  // bin\python25.zip (within loc)
  // Python\lib\site-packages (within loc)
  // ...so set PYTHONPATH accordingly:
  if (strcmp(key, "2011") == 0) {
    //Maya 2011 is built against Python 2.6 so look for that one instead
    sprintf(env4, "PYTHONPATH=%s\\bin\\python26.zip;%s\\Python\\lib\\site-packages", loc, loc);
  } else {
    sprintf(env4, "PYTHONPATH=%s\\bin\\python25.zip;%s\\Python\\lib\\site-packages", loc, loc);
  }
  // Set environment variables MAYA_LOCATION, PYTHONHOME, PYTHONPATH, PATH
  _putenv(env2);
  _putenv(env3);
  _putenv(env4);
  _putenv(env1);
#else
#ifdef __APPLE__
  path = getenv("DYLD_LIBRARY_PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "DYLD_LIBRARY_PATH=%s/MacOS:%s", loc, path);
  env3 = (char*)malloc(100 + strlen(loc));
  sprintf(env3, "PYTHONHOME=%s/Frameworks/Python.framework/Versions/Current", loc);
  _putenv(env3);
#else
  path = getenv("LD_LIBRARY_PATH");
  if (path == 0) path = "";
  env1 = (char*)malloc(100 + strlen(loc) + strlen(path));
  sprintf(env1, "LD_LIBRARY_PATH=%s/lib:%s", loc, path);
#endif // __APPLE__
  env2 = (char*)malloc(100 + strlen(loc));
  sprintf(env2, "MAYA_LOCATION=%s", loc);
  
  _putenv(env1);
  _putenv(env2);
#endif // _WIN32

  // When this is set, Panda3D will try not to use any functions from the
  // CPython API.  This is necessary because Maya links with its own copy
  // of Python, which may be incompatible with ours.
  _putenv("PANDA_INCOMPATIBLE_PYTHON=1");
  
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

