// Filename: load_dso.cxx
// Created by:  drose (12May00)
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

#include "load_dso.h"

#if defined(PENV_OSX)
// These are not used on Macintosh OS X

void *
load_dso(const Filename &) {
  return (void *) NULL;
}

bool
unload_dso(void *dso_handle) {
 return false;
}

string
load_dso_error() {
  const char *message="load_dso_error() unsupported on OS X";
  return message;
}

#elif defined(PENV_PS2)

// The playstation2 can't do any of this stuff, so these functions are B O G U S

void *
load_dso(const Filename &)
{
  return (void *) NULL;
}

bool
unload_dso(void *dso_handle) {
 return false;
}

string
load_dso_error()
{
  ostringstream ps2errmsg;
  ps2errmsg << "load_dso_error() unsupported on PS2";

  return ps2errmsg.str();
}

#else


#if defined(WIN32)
/* begin Win32-specific code */

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

void *
load_dso(const Filename &filename) {
  string os_specific = filename.to_os_specific();
  return LoadLibrary(os_specific.c_str());
}

bool
unload_dso(void *dso_handle) {
  HMODULE dll_handle = (HMODULE) dso_handle;

  // true indicates success
  return (FreeLibrary(dll_handle)!=0);
}

string
load_dso_error() {
  DWORD last_error = GetLastError();
  switch (last_error) {
    case 2: return "File not found";
    case 3: return "Path not found";
    case 4: return "Too many open files";
    case 5: return "Access denied";
    case 14: return "Out of memory";
    case 18: return "No more files";
    case 126: return "Module not found";
    case 998: return "Invalid access to memory location";
  }

  // Some unknown error code.
  ostringstream errmsg;
  errmsg << "Unknown error " << last_error;
  return errmsg.str();
}

/* end Win32-specific code */

#elif defined(PENV_OSX)
/* begin Mac OS X code */

#include <mach-o/dyld.h>

void *
load_dso(const Filename &filename) {
  enum DYLD_BOOL result;
  cerr << "_dyld_present() = " << _dyld_present() << endl;
  cerr << "_dyld_image_count() = " << _dyld_image_count() << endl;
  string os_specific = filename.to_os_specific();
  result = NSAddLibrary(os_specific().c_str());
  if (result == FALSE) {
    cerr << "Failed to load '" << filename << "'" << endl;
  }
  // need to reference a symbol in the lib in order for static init to happen
  // need to figure out how to do this
  cerr << "_dyld_present() = " << _dyld_present() << endl;
  cerr << "_dyld_image_count() = " << _dyld_image_count() << endl;
  for (unsigned long i=0; i<_dyld_image_count(); ++i)
    cerr << "_dyld_get_image_name(" << i << ") = '" << _dyld_get_image_name(i)
         << "'" << endl;
  string stmp = filename;
  cerr << "filename = '" << filename << "'" << endl;
  int i = stmp.rfind(".dylib");
  stmp.erase(i, i+6);
  stmp += "_so";
  cerr << "filename with tail patched = '" << stmp << "'" << endl;
  i = stmp.rfind("lib");
  if (i != 0) {
    if (stmp[i-1] != '/')
      i = stmp.rfind("lib", i-1);
    stmp.erase(0, i);
  }
  cerr << "final patched filename = '" << stmp << "'" << endl;
  stmp = "___" + stmp + "_find_me__";
  cerr << "symbol name searching for = '" << stmp << "'" << endl;
  unsigned long foo1;
  void *foo2;
  _dyld_lookup_and_bind(stmp.c_str(), &foo1, &foo2);
  char *foo3 = (char*)foo1;
  cerr << "symbol value = '" << foo3 << "'" << endl;
  return (void*)0L;
}

string
load_dso_error() {
  return "No DSO loading yet!";
}

#else
/* begin generic code */

#include <dlfcn.h>

void *
load_dso(const Filename &filename) {
  string os_specific = filename.to_os_specific();
  return dlopen(os_specific.c_str(), RTLD_NOW);
}

bool
unload_dso(void *dso_handle) {
  return dlclose(dso_handle)==0;
}

string
load_dso_error() {
  return dlerror();
}

#endif

#endif // PS2
