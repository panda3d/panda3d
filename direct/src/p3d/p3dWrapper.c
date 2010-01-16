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
   on the PATH that has the same basename as the executable, and runs
   it with panda3d.exe that is also located on the PATH.  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
const char delims[] = ";";
#else
const char delims[] = ":";
#endif

int main (int argc, char* argv[]) {
  char* p3dname = strdup (argv [0]);
  strcat (p3dname, ".p3d");
  
  char* p3dfile = NULL;
  char* panda3d = NULL;
  char* path = getenv ("PATH");
  if (path) {
    // Locate the .p3d file and panda3d(.exe) on PATH.
    char* pathtok = strtok(path, delims);
    while (pathtok != NULL) {
      p3dfile = strdup (pathtok);
#ifdef _WIN32
      strcat (p3dfile, "\\");
#else
      strcat (p3dfile, "/");
#endif
      strcat (p3dfile, p3dname);
      if (access (p3dfile, R_OK) == 0) {
        break;
      } else {
        p3dfile = NULL;
      }
      panda3d = strdup (pathtok);
#ifdef _WIN32
      strcat (panda3d, "\\panda3d.exe");
#else
      strcat (panda3d, "/panda3d");
#endif
      if (access (panda3d, X_OK) == 0) {
        break;
      } else {
        panda3d = NULL;
      }
      pathtok = strtok(NULL, delims);
    }
  }
  if (p3dfile == NULL) {
    p3dfile = p3dname;
  }
  if (panda3d == NULL) {
#ifdef _WIN32
    panda3d = "panda3d.exe";
#else
    panda3d = "panda3d";
#endif
  }
  
  // Fill in a new argv object to pass to panda3d(.exe).
  char** newargv = malloc(sizeof(char*) * (argc + 2));
  newargv [0] = argv [0];
  newargv [1] = p3dfile;
  int i;
  for (i = 1; i < argc; ++i) {
    newargv [i + 1] = argv [i];
  }
  newargv [argc + 2] = 0;
  return execvp (panda3d, newargv);
}

