// Filename: find_root_dir.cxx
// Created by:  drose (29Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "find_root_dir.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

////////////////////////////////////////////////////////////////////
//     Function: find_root_dir
//  Description: Returns the path to the installable Panda3D directory
//               on the user's machine.
////////////////////////////////////////////////////////////////////
string
find_root_dir() {
#ifdef _WIN32
  // e.g., c:/Documents and Settings/<username>/Panda3D

  char buffer[MAX_PATH];
  if (SHGetSpecialFolderPath(NULL, buffer, CSIDL_APPDATA, true)) {
    bool isdir = false;
    DWORD results = GetFileAttributes(buffer);
    if (results != -1) {
      isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    if (isdir) {
      // The user prefix exists; do we have a Panda3D child?
      string root = buffer;
      root += string("/Panda3D");

      // Attempt to make it first, if possible.
      CreateDirectory(root.c_str(), NULL);

      isdir = false;
      results = GetFileAttributes(root.c_str());
      if (results != -1) {
        isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
      }

      if (isdir) {
        // The directory exists!
        return root;
      }
    }
  }

  // Couldn't find a directory.  Bail.
  return ".";

#else  // _WIN32
  // TODO.
  return "/Users/drose/p3ddir";

#endif
}
