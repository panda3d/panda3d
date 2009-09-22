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
#else
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#endif

#include <iostream>
using namespace std;


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: get_csidl_dir
//  Description: A wrapper around SHGetSpecialFolderPath(), to return
//               the Panda3D directory under the indicated CSIDL
//               folder.
////////////////////////////////////////////////////////////////////
static string
get_csidl_dir(int csidl) {
  static const int buffer_size = MAX_PATH;
  char buffer[buffer_size];
  if (SHGetSpecialFolderPath(NULL, buffer, csidl, true)) {
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

  // Something went wrong.
  return string();
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: find_root_dir
//  Description: Returns the path to the installable Panda3D directory
//               on the user's machine.
////////////////////////////////////////////////////////////////////
string
find_root_dir() {
#ifdef _WIN32
  // TODO: use IEIsProtectedModeProcess() to determine if we are
  // running in IE's "protected mode" here.
  bool is_protected = false;

  int csidl = CSIDL_APPDATA;
  // e.g., c:/Documents and Settings/<username>/Application Data/Panda3D

  if (is_protected) {
    // In IE's "protected mode", we have to use the common directory,
    // which has already been created for us with low-level
    // permissions.

    csidl = CSIDL_COMMON_APPDATA;
    // e.g., c:/Documents and Settings/All Users/Application Data/Panda3D
  }
  string root = get_csidl_dir(csidl);
  if (!root.empty()) {
    return root;
  }

  // Hmm, if that failed, try the "Temporary Internet Files" folder.
  root = get_csidl_dir(CSIDL_INTERNET_CACHE);
  if (!root.empty()) {
    return root;
  }
  
  // If we couldn't get any of those folders, huh.  Punt and try for
  // Temp, for lack of a better place.
  static const int buffer_size = MAX_PATH;
  char buffer[buffer_size];
  if (GetTempPath(buffer_size, buffer) != 0) {
    string root = buffer;
    root += string("Panda3D");
    
    // Attempt to make it first, if possible.
    CreateDirectory(root.c_str(), NULL);
    
    bool isdir = false;
    DWORD results = GetFileAttributes(root.c_str());
    if (results != -1) {
      isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    
    if (isdir) {
      // The directory exists!
      return root;
    }
  }

#else  // _WIN32
  // e.g., /home/<username>/.panda3d

  string root;
  const char *uname = getlogin();
  if (uname == NULL) uname = getenv("USER");

  const passwd *pwdata = getpwnam(uname);
  if (pwdata == NULL) {
    root = getenv("HOME");
  } else {
    root = pwdata->pw_dir;
  }
  
  root += "/.panda3d";
  if (mkdir(root.c_str(), 0700) == 0 || errno == EEXIST) {
    return root;
  }

#endif

  // Couldn't find a directory.  Punt.
  return ".";
}

