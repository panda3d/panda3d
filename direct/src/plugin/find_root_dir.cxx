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

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: wstr_to_string
//  Description: Converts Windows' LPWSTR to a std::string.
////////////////////////////////////////////////////////////////////
static bool
wstr_to_string(string &result, const LPWSTR wstr) {
  bool success = false;
  int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1,
                                 NULL, 0, NULL, NULL);
  if (size > 0) {
    char *buffer = new char[size];
    int rc = WideCharToMultiByte(CP_UTF8, 0, wstr, -1,
                                 buffer, size, NULL, NULL);
    if (rc != 0) {
      buffer[size - 1] = 0;
      result = buffer;
      success = true;
    }
    delete[] buffer;
  }

  return success;
}
#endif  // _WIN32

////////////////////////////////////////////////////////////////////
//     Function: find_root_dir
//  Description: Returns the path to the installable Panda3D directory
//               on the user's machine.
////////////////////////////////////////////////////////////////////
string
find_root_dir(ostream &logfile) {
#ifdef _WIN32
  // First, use IEIsProtectedModeProcess() to determine if we are
  // running in IE's "protected mode".

  bool is_protected = false;
  HMODULE module = LoadLibrary("ieframe.dll");
  if (module != NULL) {
    typedef HRESULT STDAPICALLTYPE IEIsProtectedModeProcess(BOOL *pbResult);
    IEIsProtectedModeProcess *func = (IEIsProtectedModeProcess *)GetProcAddress(module, "IEIsProtectedModeProcess");
    if (func != NULL) {
      BOOL result = false;
      HRESULT hr = (*func)(&result);
      if (hr == S_OK) {
        is_protected = (result != 0);
        logfile << "IEIsProtectedModeProcess indicates: " << is_protected << "\n";
      }
      // Any other return value means some error, especially
      // E_NOTIMPL, which means we're not running under Vista.  In
      // this case we can assume we're not running in protected mode.
    }
  }    

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
    if (module != NULL) {
      FreeLibrary(module);
    }
    return root;
  }

  // All right, couldn't get a writable pointer to our APPDATA folder.
  // We're in fallback mode now.  Start by asking for a "writeable
  // path" to a temporary folder.
  if (module != NULL) {
    typedef HRESULT STDAPICALLTYPE IEGetWriteableFolderPath(REFGUID clsidFolderID, LPWSTR* lppwstrPath);
    IEGetWriteableFolderPath *func = (IEGetWriteableFolderPath *)GetProcAddress(module, "IEGetWriteableFolderPath");
    if (func != NULL) {
      // From KnownFolders.h (part of Vista SDK):
#define DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
   const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
      DEFINE_KNOWN_FOLDER(FOLDERID_InternetCache, 0x352481E8, 0x33BE, 0x4251, 0xBA, 0x85, 0x60, 0x07, 0xCA, 0xED, 0xCF, 0x9D); 
      
      LPWSTR cache_path = NULL;
      HRESULT hr = (*func)(FOLDERID_InternetCache, &cache_path);
      if (SUCCEEDED(hr)) {
        if (!wstr_to_string(root, cache_path)) {
          // Couldn't decode the LPWSTR.
          CoTaskMemFree(cache_path);
        } else {
          CoTaskMemFree(cache_path);
          root += string("/Panda3D");
          
          // Attempt to make it first, if possible.
          CreateDirectory(root.c_str(), NULL);
          
          bool isdir = false;
          DWORD results = GetFileAttributes(root.c_str());
          if (results != -1) {
            isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
          }
        
          if (isdir) {
            // The directory exists!
            FreeLibrary(module);
            return root;
          }
        }
      }
    }
  }    

  // We're done with the HMODULE now.
  if (module != NULL) {
    FreeLibrary(module);
  }

  // All right, GetWriteableFolderPath failed; fall back to the XP-era
  // way of asking for the "Temporary Internet Files".
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

