// Filename: test_glob.cxx
// Created by:  drose (30May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "globPattern.h"

#ifdef WIN32
#include <io.h>
#else
#include <sys/types.h>
#include <dirent.h>
#endif

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr
      << "test_glob \"pattern\"\n\n"
      << "Attempts to match the pattern against each of the files in the\n"
      << "current directory.  Reports all of the matching files.  This is,\n"
      << "of course, exactly the normal behavior of the shell; this test\n"
      << "program merely exercises the Panda GlobPattern object, which\n"
      << "duplicates the shell functionality.\n\n";
    exit(1);
  }

  GlobPattern pattern(argv[1]);

#ifdef WIN32
  struct _finddata_t fd;
  long dp = _findfirst("*.*", &fd);

  if (dp != 0) {
    do {
      if (pattern.matches(fd.name)) {
        cout << "Matches: " << fd.name << "\n";
      } else {
        cout << "Fails: " << fd.name << "\n";
      }
    } while (_findnext(dp, &fd) == 0);
  }

#else // WIN32
  DIR *dp = opendir(".");
  if (dp == (DIR *)NULL) {
    cerr << "Unable to scan directory '.'\n";
    exit(1);
  }

  struct dirent *ent;
  ent = readdir(dp);
  while (ent != (struct dirent *)NULL) {
    if (pattern.matches(ent->d_name)) {
      cout << "Matches: " << ent->d_name << "\n";
    } else {
      cout << "Fails: " << ent->d_name << "\n";
    }
    ent = readdir(dp);
  }

  closedir(dp);
#endif  // WIN32

  return (0);
}


