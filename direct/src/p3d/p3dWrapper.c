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

#define BUFFER_SIZE 1024

int main (int argc, char* argv[]) {
  int i;
  char buffer [BUFFER_SIZE];
  char* p3dfile;
  char* runtime = NULL;
  DWORD size;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  char *cmd;
  char *newcmd;
  HKEY hKey = 0;
  char buf [1024] = {0};
  DWORD dwType = 0;
  DWORD dwBufSize = sizeof(buf);
  size = GetModuleFileName (NULL, buffer, BUFFER_SIZE);
  assert (size > 0);

  /* Chop off the .exe and replace it by .p3d. */
  p3dfile = (char*) malloc (size + 1);
  memcpy (p3dfile, buffer, size);
  p3dfile [size] = 0;
  memcpy (p3dfile + size - 3, "p3d", 3);

  /* Find the Panda3D applet\DefaultIcon key and extract the path to the runtime from there. */
  if (RegOpenKey (HKEY_CLASSES_ROOT, "Panda3D applet\\DefaultIcon", &hKey) == ERROR_SUCCESS) {
    dwType = REG_SZ;
    if (RegQueryValueEx(hKey, 0, 0, &dwType, (BYTE*) buf, &dwBufSize) == ERROR_SUCCESS) {
      for (i = dwBufSize - 1; i >= 0; --i) {
        if (buf [i] == '/' || buf [i] == '\\') {
          runtime = (char*) malloc (i + 13);
          memcpy (runtime, buf, i);
          runtime [i] = 0;         
          strcat (runtime, "\\panda3d.exe");      
          break;
        }
      }
    } else {
      fprintf (stderr, "Failed to read registry key. Try reinstalling the Panda3D Runtime.\n");
      return 1;
    }
    RegCloseKey(hKey);
  } else {
    fprintf (stderr, "The Panda3D Runtime does not appear to be installed!\n");
    return 1;
  }

  if (runtime == NULL) {
    fprintf (stderr, "Failed to find panda3d.exe in registry. Try reinstalling the Panda3D Runtime.\n");
    return 1;
  }

  /* Build the command-line and run panda3d.exe. */
  cmd = GetCommandLine();
  newcmd = (char*) malloc (strlen(runtime) + strlen(p3dfile) + strlen (cmd) - strlen (argv[0]) + 7);
  sprintf (newcmd, "\"%s\" \"%s\" %s", runtime, p3dfile, cmd + strlen (argv[0]));
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(STARTUPINFO);
  if (CreateProcess(runtime, newcmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    WaitForSingleObject(pi.hProcess, INFINITE);
  }
  free (newcmd);
  return 0;
}

