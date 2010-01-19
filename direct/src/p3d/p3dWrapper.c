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
  char** newargv;
  DWORD size;
  size = GetModuleFileName (NULL, buffer, BUFFER_SIZE);
  assert (size > 0);

  /* Chop off the .exe and replace it by .p3d. */
  p3dfile = (char*) malloc (size + 1);
  memcpy (p3dfile, buffer, size);
  p3dfile [size] = 0;
  memcpy (p3dfile + size - 3, "p3d", 3);

  /* Fill in a new argv object to pass to panda3d(.exe). */
  newargv = (char**) malloc (sizeof (char*) * (argc + 2));
  newargv [0] = "panda3d.exe";
  newargv [1] = p3dfile;
  for (i = 1; i < argc; ++i) {
    newargv [i + 1] = _strdup (argv [i]);
  }
  newargv [argc + 1] = NULL;
  if (_execvp ("panda3d.exe", newargv) == -1) {
    fprintf (stderr, "panda3d.exe: %s", _strerror (NULL));
    return 1;
  }
  return 0;
}

