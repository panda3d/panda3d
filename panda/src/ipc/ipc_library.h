// Filename: ipc_library.h
// Created by:  frang (10Feb00)
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

#ifndef __IPC_LIBRARY_H__
#define __IPC_LIBRARY_H__

#include <pandabase.h>

#include "ipc_traits.h"

class base_library {
public:
  typedef ipc_traits traits;
  typedef traits::library_class library_class;

  static base_library* const Null;
  INLINE base_library(std::string& lib) : _lib(traits::make_library(lib)) {}
  INLINE ~base_library(void) { delete _lib; }
  INLINE void* get_symbol(std::string& sym) { return _lib->get_symbol(sym); }
private:
  library_class* _lib;
};

typedef base_library library;

#endif /* __IPC_LIBRARY_H__ */
