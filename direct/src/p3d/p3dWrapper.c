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
   it with panda3d(.exe) that is also located on the PATH.  */

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
  char havep3dfile = 0;
  char havepanda3d = 0;
  char* p3dname = strdup (argv [0]);
  strcat (p3dname, ".p3d");
  if (access (p3dname, R_OK) == 0) {
    havep3dfile = 1;
  } else {
    // Make sure that p3dname contains a basename only.
    int c;
    for (c = strlen(p3dname) - 1; c >= 0; --c) {
      if (p3dname[c] == '/' || p3dname[c] == '\\') {
        p3dname += c + 1;
        break;
      }
    }
  }
  
  char* p3dfile = NULL;
  char* panda3d = NULL;
  char* path = getenv ("PATH");
  if (path) {
    // Locate the .p3d file and panda3d(.exe) on PATH.
    char* pathtok = NULL;
    pathtok = strtok (path, delims);
    while (pathtok != NULL) {
      // Check if the p3d file is in this directory.
      if (!havep3dfile) {
        p3dfile = strdup (pathtok);
#ifdef _WIN32
        strcat (p3dfile, "\\");
#else
        strcat (p3dfile, "/");
#endif
        strcat (p3dfile, p3dname);
        if (access (p3dfile, R_OK) == 0) {
          havep3dfile = 1;
        }
      }
      // Check if panda3d(.exe) is in this directory.
      if (!havepanda3d) {
        panda3d = strdup (pathtok);
#ifdef _WIN32
        strcat (panda3d, "\\panda3d.exe");
#else
        strcat (panda3d, "/panda3d");
#endif
        if (access (panda3d, X_OK) == 0) {
          havepanda3d = 1;
        }
      }
      if (havep3dfile && havepanda3d) {
        break;
      }
      pathtok = strtok(NULL, delims);
    }
  }
  if (havep3dfile == 0 || p3dfile == NULL) {
    p3dfile = p3dname;
  }
  if (havepanda3d == 0 || panda3d == NULL) {
#ifdef _WIN32
    panda3d = (char*) "panda3d.exe";
#else
    panda3d = (char*) "panda3d";
#endif
  }
  
  // Fill in a new argv object to pass to panda3d(.exe).
  char** newargv = (char**) malloc(sizeof(char*) * (argc + 2));
  newargv [0] = argv [0];
  newargv [1] = p3dfile;
  int i;
  for (i = 1; i < argc; ++i) {
    newargv [i + 1] = argv [i];
  }
  newargv [argc + 2] = 0;
  return execvp (panda3d, newargv);
}

