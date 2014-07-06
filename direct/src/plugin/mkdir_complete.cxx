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
#include "wstring_encode.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>    // chmod()
#else
#include <fcntl.h>
#include <sys/stat.h>  // for mkdir()
#include <errno.h>
#include <string.h>     // strerror()
#include <unistd.h>
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

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: get_dirname_w
//  Description: The wide-character implementation of get_dirname().
//               Only implemented (and needed) on Windows.
////////////////////////////////////////////////////////////////////
static wstring
get_dirname_w(const wstring &filename) {
  size_t p = filename.length();
  while (p > 0) {
    --p;
    if (is_pathsep(filename[p])) {
      return filename.substr(0, p);
    }
  }

  return wstring();
}
#endif  // _WIN32



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
  wstring dirname_w;
  if (!string_to_wstring(dirname_w, dirname)) {
    return false;
  }
  return mkdir_complete_w(dirname_w, logfile);

#else  //_WIN32
  if (mkdir(dirname.c_str(), 0755) == 0) {
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
      if (mkdir(dirname.c_str(), 0755) == 0) {
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
  wstring filename_w;
  if (!string_to_wstring(filename_w, filename)) {
    return false;
  }
  return mkfile_complete_w(filename_w, logfile);
#else  // _WIN32
  // Make sure we delete any previously-existing file first.
  unlink(filename.c_str());

  int fd = creat(filename.c_str(), 0755);
  if (fd == -1) {
    // Try to make the parent directory first.
    string parent = get_dirname(filename);
    if (!parent.empty() && mkdir_complete(parent, logfile)) {
      // Parent successfully created.  Try again to make the file.
      fd = creat(filename.c_str(), 0755);
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


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: mkdir_complete_w
//  Description: The wide-character implementation of
//               mkdir_complete().  Only implemented (and needed) on
//               Windows.
////////////////////////////////////////////////////////////////////
bool
mkdir_complete_w(const wstring &dirname, ostream &logfile) {
  if (CreateDirectoryW(dirname.c_str(), NULL) != 0) {
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
    wstring parent = get_dirname_w(dirname);
    if (!parent.empty() && mkdir_complete_w(parent, logfile)) {
      // Parent successfully created.  Try again to make the child.
      if (CreateDirectoryW(dirname.c_str(), NULL) != 0) {
        // Got it!
        return true;
      }
      logfile 
        << "Couldn't create " << dirname << "\n";
    }
  }
  return false;
}
#endif  // _WIN32

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: mkfile_complete_w
//  Description: The wide-character implementation of
//               mkfile_complete().  Only implemented (and needed) on
//               Windows.
////////////////////////////////////////////////////////////////////
bool
mkfile_complete_w(const wstring &filename, ostream &logfile) {
  // Make sure we delete any previously-existing file first.

  // Windows can't delete a file if it's read-only.  Weird.
  _wchmod(filename.c_str(), 0644);
  _wunlink(filename.c_str());

  HANDLE file = CreateFileW(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    // Try to make the parent directory first.
    wstring parent = get_dirname_w(filename);
    if (!parent.empty() && mkdir_complete_w(parent, logfile)) {
      // Parent successfully created.  Try again to make the file.
      file = CreateFileW(filename.c_str(), GENERIC_READ | GENERIC_WRITE,
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
}
#endif  // _WIN32
