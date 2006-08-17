// Filename: load_dso.cxx
// Created by:  drose (12May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "load_dso.h"

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

#elif defined(IS_OSX)
/* begin Mac OS X code */

#include <mach-o/dyld.h>
#include <dlfcn.h>

void * load_dso(const Filename &filename) 
{

	std::string fname = filename.to_os_specific();
	void * answer = dlopen(fname.c_str(),RTLD_NOW| RTLD_LOCAL);
	//void * answer = dlopen(fname.c_str(),RTLD_NOW);

	return answer;

}

string
load_dso_error() {
  return std::string(dlerror());
  //return "No DSO loading yet!";
}

#else
/* begin generic code */

#include <dlfcn.h>

void *
load_dso(const Filename &filename) {
  string os_specific = filename.to_os_specific();
  return dlopen(os_specific.c_str(), RTLD_NOW | RTLD_GLOBAL);
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
