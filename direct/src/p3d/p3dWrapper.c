/* Filename: p3dWrapper.c
 * Created by:  rdb (16Jan10)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* p3dWrapper is a small wrapper executable that locates a .p3d file
   in the same directory as this executable file, with the same name
   (except .p3d instead of .exe of course). It is only meant to be
   used on Windows system, it is not needed on Unix-like systems. */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <assert.h>
#include <malloc.h>

#define BUFFER_SIZE 1024

/* It makes sense to use "App Paths\panda3d.exe".  However, Microsoft
   decided in their infinite wisdom to disable Redirection for that
   key from Windows 7 onward, so we can't rely on it producing a
   result appropriate to the right architecture when both the 32-bit
   and 64-bit versions of the runtime are installed.  Beh. */

#define UNINST_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Panda3D Game Engine"

int main (int argc, char* argv[]) {
  int i;
  char buffer[BUFFER_SIZE];
  char* p3dfile;
  char* runtime = NULL;
  DWORD size;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char *cmd;
  char *newcmd;
  HKEY hKey = 0;
  char buf[1024] = {0};
  DWORD dwType = REG_SZ;
  DWORD dwBufSize = sizeof(buf);
  size = GetModuleFileName(NULL, buffer, BUFFER_SIZE);
  assert (size > 0);

  /* Chop off the .exe and replace it by .p3d. */
  p3dfile = (char*) _alloca(size + 1);
  memcpy(p3dfile, buffer, size);
  p3dfile[size] = 0;
  memcpy(p3dfile + size - 3, "p3d", 3);

  /* Find the location of panda3d.exe using the registry path. */
#ifdef _WIN64
  /* If we're on 64-bit Windows, try the 64-bit registry first. */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, UNINST_KEY, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) == ERROR_SUCCESS) {
    if (RegQueryValueEx(hKey, "DisplayIcon", 0, &dwType, (BYTE*) buf, &dwBufSize) == ERROR_SUCCESS) {
      char *slash = strrchr(buf, '\\');
      if (slash != NULL) {
        strcpy(slash, "\\panda3d.exe");
        runtime = buf;
      }
    }
    RegCloseKey(hKey);
  }
#endif

  /* On 32-bit Windows, or no 64-bit Runtime installed.  Try 32-bit registry. */
  if (runtime == NULL) {
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, UNINST_KEY, 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &hKey) == ERROR_SUCCESS) {
      if (RegQueryValueEx(hKey, "DisplayIcon", 0, &dwType, (BYTE*) buf, &dwBufSize) == ERROR_SUCCESS) {
        char *slash = strrchr(buf, '\\');
        if (slash != NULL) {
          strcpy(slash, "\\panda3d.exe");
          runtime = buf;
        }
      }
      RegCloseKey(hKey);
    }
  }

  /* Backward compatibility: Runtime 1.0.4 and below looked for the below key, even though the
     above keys should work fine, but let's just be certain.
     Find the Panda3D applet\DefaultIcon key and extract the path to the runtime from there.  */
  if (runtime == NULL) {
    if (RegOpenKey(HKEY_CLASSES_ROOT, "Panda3D applet\\DefaultIcon", &hKey) == ERROR_SUCCESS) {
      if (RegQueryValueEx(hKey, 0, 0, &dwType, (BYTE*) buf, &dwBufSize) == ERROR_SUCCESS) {
        char *slash = strrchr(buf, '\\');
        if (slash != NULL) {
          strcpy(slash, "\\panda3d.exe");
          runtime = buf;
        }
      } else {
        fprintf(stderr, "Failed to read registry key. Try reinstalling the Panda3D Runtime.\n");
        return 1;
      }
      RegCloseKey(hKey);
    } else {
      fprintf(stderr, "The Panda3D Runtime does not appear to be installed!\n");
      return 1;
    }
  }

  if (runtime == NULL) {
    fprintf(stderr, "Failed to find panda3d.exe in registry. Try reinstalling the Panda3D Runtime.\n");
    return 1;
  }

  /* Build the command-line and run panda3d.exe. */
  cmd = GetCommandLine();
  newcmd = (char*) _alloca(strlen(runtime) + strlen(p3dfile) + strlen(cmd) - strlen (argv[0]) + 7);
  sprintf(newcmd, "\"%s\" \"%s\" %s", runtime, p3dfile, cmd + strlen(argv[0]));
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(STARTUPINFO);
  if (CreateProcess(runtime, newcmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    WaitForSingleObject(pi.hProcess, INFINITE);
  }
  return 0;
}
