// Filename: ipc_library.h
// Created by:  frang (10Feb00)
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
