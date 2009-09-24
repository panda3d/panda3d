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
// From KnownFolders.h (part of Vista SDK):
#define DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
   const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppData, 0xF1B32785, 0x6FBA, 0x4FCF, 0x9D, 0x55, 0x7B, 0x8E, 0x7F, 0x15, 0x70, 0x91);
DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppDataLow, 0xA520A1A4, 0x1780, 0x4FF6, 0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16);
DEFINE_KNOWN_FOLDER(FOLDERID_InternetCache, 0x352481E8, 0x33BE, 0x4251, 0xBA, 0x85, 0x60, 0x07, 0xCA, 0xED, 0xCF, 0x9D); 
#endif  // _WIN32


#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: check_root_dir
//  Description: Tests the proposed root dir string for validity.
//               Returns true if Panda3D can be successfully created
//               within the dir, false otherwise.
////////////////////////////////////////////////////////////////////
static bool
check_root_dir(const string &root) {
  // Attempt to make it first, if possible.
  CreateDirectory(root.c_str(), NULL);
  
  bool isdir = false;
  DWORD results = GetFileAttributes(root.c_str());
  if (results != -1) {
    isdir = (results & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }
  return isdir;
}
#endif // _WIN32

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
    string root = buffer;
    root += string("/Panda3D");
    
    if (check_root_dir(root)) {
      return root;
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
  // running in IE's "protected mode" under Vista.

  string root;
  bool is_protected = false;
  HMODULE ieframe = LoadLibrary("ieframe.dll");
  if (ieframe != NULL) {
    typedef HRESULT STDAPICALLTYPE IEIsProtectedModeProcess(BOOL *pbResult);
    IEIsProtectedModeProcess *func = (IEIsProtectedModeProcess *)GetProcAddress(ieframe, "IEIsProtectedModeProcess");
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

    if (is_protected) {
      // If we *are* running in protected mode, we need to use
      // FOLDERID_LocalAppDataLow.
      
      // We should be able to use IEGetWriteableFolderPath() to query
      // this folder, but for some reason, that function returns
      // E_ACCESSDENIED on FOLDERID_LocalAppDataLow, even though this is
      // certainly a folder we have write access to.
      
      // Well, SHGetKnownFolderPath() does work.  This function only
      // exists on Vista and above, though, so we still have to pull it
      // out of the DLL instead of hard-linking it.
      
      HMODULE shell32 = LoadLibrary("shell32.dll");
      if (shell32 != NULL) {
        typedef HRESULT STDAPICALLTYPE SHGetKnownFolderPath(REFGUID rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
        SHGetKnownFolderPath *func = (SHGetKnownFolderPath *)GetProcAddress(shell32, "SHGetKnownFolderPath");
        if (func != NULL) {
          LPWSTR cache_path = NULL;
          HRESULT hr = (*func)(FOLDERID_LocalAppDataLow, 0, NULL, &cache_path);
          
          if (SUCCEEDED(hr)) {
            if (!wstr_to_string(root, cache_path)) {
              // Couldn't decode the LPWSTR.
              CoTaskMemFree(cache_path);
            } else {
              CoTaskMemFree(cache_path);
              
              root += string("/Panda3D");
              if (check_root_dir(root)) {
                FreeLibrary(shell32);
                FreeLibrary(ieframe);
                return root;
              }
            }
          }
        }
        FreeLibrary(shell32);
      }
      
      // Couldn't get FOLDERID_LocalAppDataLow for some reason.  We're
      // in fallback mode now.  Use IEGetWriteableFolderPath to get
      // the standard cache folder.
      typedef HRESULT STDAPICALLTYPE IEGetWriteableFolderPath(REFGUID clsidFolderID, LPWSTR* lppwstrPath);
      IEGetWriteableFolderPath *func = (IEGetWriteableFolderPath *)GetProcAddress(ieframe, "IEGetWriteableFolderPath");
      if (func != NULL) {
        LPWSTR cache_path = NULL;

        // Since we're here, we'll start by asking for
        // LocalAppDataLow, even though I know it doesn't work.
        HRESULT hr = (*func)(FOLDERID_LocalAppDataLow, &cache_path);
        if (FAILED(hr)) {
          // This one should work.
          hr = (*func)(FOLDERID_InternetCache, &cache_path);
        }

        if (SUCCEEDED(hr)) {
          if (!wstr_to_string(root, cache_path)) {
            // Couldn't decode the LPWSTR.
            CoTaskMemFree(cache_path);
          } else {
            CoTaskMemFree(cache_path);
            root += string("/Panda3D");
            if (check_root_dir(root)) {
              FreeLibrary(ieframe);
              return root;
            }
          }            
        }
      }
    }

    FreeLibrary(ieframe);
  }

  // All right, here we are in the normal, unprotected mode.  This is
  // also the normal XP codepath.

  // e.g., c:/Documents and Settings/<username>/Local Settings/Application Data/Panda3D
  root = get_csidl_dir(CSIDL_LOCAL_APPDATA);
  if (!root.empty()) {
    return root;
  }

  // For some crazy reason, we can't get CSIDL_LOCAL_APPDATA.  Fall
  // back to the cache folder.

  // e.g. c:/Documents and Settings/<username>/Local Settings/Temporary Internet Files/Panda3D
  root = get_csidl_dir(CSIDL_INTERNET_CACHE);
  if (!root.empty()) {
    return root;
  }
  
  // If we couldn't get any of those folders, huh.  Punt and try for
  // the old standby GetTempPath, for lack of anything better.
  static const int buffer_size = MAX_PATH;
  char buffer[buffer_size];
  if (GetTempPath(buffer_size, buffer) != 0) {
    root = buffer;
    root += string("Panda3D");
    if (check_root_dir(root)) {
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

