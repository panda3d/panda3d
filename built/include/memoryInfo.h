/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryInfo.h
 * @author drose
 * @date 2001-06-04
 */

#ifndef MEMORYINFO_H
#define MEMORYINFO_H

#include "pandabase.h"

#ifdef DO_MEMORY_USAGE

#include "typeHandle.h"

class ReferenceCount;
class TypedObject;

/**
 * This is a supporting class for MemoryUsage.  It records the detailed
 * information for a particular pointer allocated by Panda code.  This record
 * is only kept if track-mem-usage is configured #t.
 *
 * It's not exported from the DLL, and it doesn't even exist if we're
 * compiling NDEBUG.
 */
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
    F_size_known              = 0x0001,
    F_reconsider_dynamic_type = 0x0002,
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

#else
class MemoryInfo;

#endif  // DO_MEMORY_USAGE

#endif
