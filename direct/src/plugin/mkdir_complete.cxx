// Filename: mkdir_complete.cxx
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

#include "mkdir_complete.h"
#include "is_pathsep.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>  // for mkdir()
#include <errno.h>
#include <string.h>     // strerror()
#endif



////////////////////////////////////////////////////////////////////
//     Function: get_dirname
//  Description: Returns the directory component of the indicated
//               pathname, or the empty string if there is no
//               directory prefix.
////////////////////////////////////////////////////////////////////
static string
get_dirname(const string &filename) {
  size_t p = filename.length();
  while (p > 0) {
    --p;
    if (is_pathsep(filename[p])) {
      return filename.substr(0, p);
    }
  }

  return string();
}



////////////////////////////////////////////////////////////////////
//     Function: mkdir_complete
//  Description: Creates a new directory, with normal access
//               privileges.  Returns true on success, false on
//               failure.  Will create intervening directories if
//               necessary.
////////////////////////////////////////////////////////////////////
bool
mkdir_complete(const string &dirname, ostream &logfile) {
#ifdef _WIN32
  if (CreateDirectory(dirname.c_str(), NULL) != 0) {
    // Success!
    return true;
  }

  // Failed.
  DWORD last_error = GetLastError();
  if (last_error == ERROR_ALREADY_EXISTS) {
    // Not really an error: the directory is already there.
    return true;
  }

  if (last_error == ERROR_PATH_NOT_FOUND) {
    // We need to make the parent directory first.
    string parent = get_dirname(dirname);
    if (!parent.empty() && mkdir_complete(parent)) {
      // Parent successfully created.  Try again to make the child.
      if (CreateDirectory(dirname.c_str(), NULL) != 0) {
        // Got it!
        return true;
      }
      logfile 
        << "Couldn't create " << dirname << "\n";
    }
  }
  return false;

#else  //_WIN32
  if (mkdir(dirname.c_str(), 0777) == 0) {
    // Success!
    return true;
  }

  // Failed.
  if (errno == EEXIST) {
    // Not really an error: the directory is already there.
    return true;
  }

  if (errno == ENOENT || errno == EACCES) {
    // We need to make the parent directory first.
    string parent = get_dirname(dirname);
    if (!parent.empty() && mkdir_complete(parent, logfile)) {
      // Parent successfully created.  Try again to make the child.
      if (mkdir(dirname.c_str(), 0777) == 0) {
        // Got it!
        return true;
      }
      // Couldn't create the directory. :(
      logfile 
        << "Couldn't create " << dirname << ": " << strerror(errno) << "\n";
    }
  }
  return false;

#endif  // _WIN32
}

////////////////////////////////////////////////////////////////////
//     Function: mkfile_complete
//  Description: Creates a new file with normal access
//               priviledges.  Returns true on success, false on
//               failure.  This will create intervening directories if
//               needed.
////////////////////////////////////////////////////////////////////
bool
mkfile_complete(const string &filename, ostream &logfile) {
#ifdef _WIN32
  HANDLE file = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    // Try to make the parent directory first.
    string parent = get_dirname(filename);
    if (!parent.empty() && mkdir_complete(parent, logfile)) {
      // Parent successfully created.  Try again to make the file.
      file = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (file == INVALID_HANDLE_VALUE) {
      logfile
        << "Couldn't create " << filename << "\n";
      return false;
    }
  }
  CloseHandle(file);
  return true;

#else  // _WIN32
  int fd = creat(filename.c_str(), 0777);
  if (fd == -1) {
    // Try to make the parent directory first.
    string parent = get_dirname(filename);
    if (!parent.empty() && mkdir_complete(parent, logfile)) {
      // Parent successfully created.  Try again to make the file.
      fd = creat(filename.c_str(), 0777);
    }
    if (fd == -1) {
      logfile
        << "Couldn't create " << filename << ": " << strerror(errno) << "\n";
      return false;
    }
  }
  close(fd);
  return true;

#endif  // _WIN32
}
