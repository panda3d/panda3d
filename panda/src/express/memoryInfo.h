// Filename: memoryInfo.h
// Created by:  drose (04Jun01)
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

#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include "pandabase.h"

#ifdef DO_MEMORY_USAGE

#include "typeHandle.h"

class ReferenceCount;
class TypedObject;

////////////////////////////////////////////////////////////////////
//       Class : MemoryInfo
// Description : This is a supporting class for MemoryUsage.  It
//               records the detailed information for a particular
//               pointer allocated by Panda code.  This record is only
//               kept if track-mem-usage is configured #t.
//
//               It's not exported from the DLL, and it doesn't even
//               exist if we're compiling NDEBUG.
////////////////////////////////////////////////////////////////////
class MemoryInfo {
public:
  MemoryInfo();

public:
  // Members to view the MemoryInfo structure.
  TypeHandle get_type();

  INLINE void *get_void_ptr() const;
  INLINE ReferenceCount *get_ref_ptr() const;
  INLINE TypedObject *get_typed_ptr() const;

  INLINE bool is_size_known() const;
  INLINE size_t get_size() const;

  INLINE double get_time() const;

private:
  // Members to set up the MemoryInfo structure.
  void determine_dynamic_type();
  bool update_type_handle(TypeHandle &destination, TypeHandle refined);

private:
  enum Flags {
    F_got_ref                 = 0x0001,
    F_got_void                = 0x0002,
    F_size_known              = 0x0004,
    F_reconsider_dynamic_type = 0x0008,
  };
  
  void *_void_ptr;
  ReferenceCount *_ref_ptr;
  TypedObject *_typed_ptr;
  size_t _size;
  TypeHandle _static_type;
  TypeHandle _dynamic_type;
  int _flags;
  
  double _time;
  int _freeze_index;

  friend class MemoryUsage;
};

#include "memoryInfo.I"

#endif  // DO_MEMORY_USAGE

#endif

