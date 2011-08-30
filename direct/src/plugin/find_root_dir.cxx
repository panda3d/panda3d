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
#include "mkdir_complete.h"
#include "get_tinyxml.h"
#include "wstring_encode.h"

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
//     Function: get_csidl_dir_w
//  Description: A wrapper around SHGetSpecialFolderPath(), to return
//               the Panda3D directory under the indicated CSIDL
//               folder.
////////////////////////////////////////////////////////////////////
static wstring
get_csidl_dir_w(int csidl) {
  static const int buffer_size = MAX_PATH;
  wchar_t buffer[buffer_size];
  if (SHGetSpecialFolderPathW(NULL, buffer, csidl, true)) {
    wstring root = buffer;
    root += wstring(L"/Panda3D");
    
    if (mkdir_complete_w(root, cerr)) {
      return root;
    }
  }

  // Something went wrong.
  return wstring();
}
#endif  // _WIN32

#ifdef _WIN32
////////////////////////////////////////////////////////////////////
//     Function: find_root_dir_default_w
//  Description: Wide-character implementation of
//               find_root_dir_default(), only needed for Windows.
////////////////////////////////////////////////////////////////////
static wstring
find_root_dir_default_w() {
  // First, use IEIsProtectedModeProcess() to determine if we are
  // running in IE's "protected mode" under Vista.

  wstring root;
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
            root = cache_path;
            CoTaskMemFree(cache_path);
              
            root += wstring(L"/Panda3D");
            if (mkdir_complete_w(root, cerr)) {
              FreeLibrary(shell32);
              FreeLibrary(ieframe);
              return root;
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
          root = cache_path;
          CoTaskMemFree(cache_path);
          root += wstring(L"/Panda3D");
          if (mkdir_complete_w(root, cerr)) {
            FreeLibrary(ieframe);
            return root;
          }            
        }
      }
    }

    FreeLibrary(ieframe);
  }

  // All right, here we are in the normal, unprotected mode.  This is
  // also the normal XP codepath.

  // e.g., c:/Documents and Settings/<username>/Local Settings/Application Data/Panda3D
  root = get_csidl_dir_w(CSIDL_LOCAL_APPDATA);
  if (!root.empty()) {
    return root;
  }

  // For some crazy reason, we can't get CSIDL_LOCAL_APPDATA.  Fall
  // back to the cache folder.

  // e.g. c:/Documents and Settings/<username>/Local Settings/Temporary Internet Files/Panda3D
  root = get_csidl_dir_w(CSIDL_INTERNET_CACHE);
  if (!root.empty()) {
    return root;
  }
  
  // If we couldn't get any of those folders, huh.  Punt and try for
  // the old standby GetTempPath, for lack of anything better.
  static const int buffer_size = MAX_PATH;
  wchar_t buffer[buffer_size];
  if (GetTempPathW(buffer_size, buffer) != 0) {
    root = buffer;
    root += wstring(L"Panda3D");
    if (mkdir_complete_w(root, cerr)) {
      return root;
    }
  }

  return wstring();
}
#endif  // _WIN32


////////////////////////////////////////////////////////////////////
//     Function: find_root_dir_default
//  Description: Returns the path to the system-default for the root
//               directory.  This is where we look first.
////////////////////////////////////////////////////////////////////
static string
find_root_dir_default() {
#ifdef _WIN32
  wstring root_w = find_root_dir_default_w();
  if (!root_w.empty()) {
    string root;
    if (wstring_to_string(root, root_w)) {
      return root;
    }
  }

#elif defined(__APPLE__)
  // e.g., /Users/<username>/Library/Caches/Panda3D
  string root = find_osx_root_dir();
  if (!root.empty()) {
    return root;
  }

#else  // The Linux/*BSD case
  // e.g., /home/<username>/.panda3d

  string root;
  const passwd *pwdata = getpwuid(getuid());
  if (pwdata == NULL) {
    char *home = getenv("HOME");
    if (home == NULL) {
      // Beh.  Let's hope it never gets to this point.
      return ".";
    } else {
      root = home;
    }
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


////////////////////////////////////////////////////////////////////
//     Function: find_root_dir_actual
//  Description: Returns the path to the installable Panda3D directory
//               on the user's machine.
////////////////////////////////////////////////////////////////////
static string
find_root_dir_actual() {
  string root = find_root_dir_default();

  // Now look for a config.xml file in that directory, which might
  // redirect us elsewhere.
  string config_filename = root + "/config.xml";
  TiXmlDocument doc(config_filename);
  if (!doc.LoadFile()) {
    // No config.xml found, or not valid xml.
    return root;
  }

  TiXmlElement *xconfig = doc.FirstChildElement("config");
  if (xconfig == NULL) {
    // No <config> element within config.xml.
    return root;
  }

  const char *new_root = xconfig->Attribute("root_dir");
  if (new_root == NULL || *new_root == '\0') {
    // No root_dir specified.
    return root;
  }

  if (!mkdir_complete(new_root, cerr)) {
    // The specified root_dir wasn't valid.
    return root;
  }

  // We've been redirected to another location.  Respect that.
  return new_root;
}

////////////////////////////////////////////////////////////////////
//     Function: find_root_dir
//  Description: This is the public interface to the above functions.
////////////////////////////////////////////////////////////////////
string
find_root_dir() {
  string root = find_root_dir_actual();

#ifdef _WIN32
  // Now map that (possibly utf-8) filename into its 8.3 equivalent,
  // so we can safely pass it around to Python and other tools that
  // might not understand Unicode filenames.  Silly Windows, creating
  // an entirely new and incompatible kind of filename.
  wstring root_w;
  string_to_wstring(root_w, root);
  
  DWORD length = GetShortPathNameW(root_w.c_str(), NULL, 0);
  wchar_t *short_name = new wchar_t[length];
  GetShortPathNameW(root_w.c_str(), short_name, length);

  wstring_to_string(root, short_name);
  delete[] short_name;
#endif  // _WIN32

  return root;
}
