// Filename: memoryUsage.h
// Created by:  drose (25May00)
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

#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include "pandabase.h"

#ifdef DO_MEMORY_USAGE

#include "typedObject.h"
#include "memoryInfo.h"
#include "memoryUsagePointerCounts.h"
#include "pmap.h"
#include "dallocator.h"

class ReferenceCount;
class MemoryUsagePointers;

////////////////////////////////////////////////////////////////////
//       Class : MemoryUsage
// Description : This class is used strictly for debugging purposes,
//               specifically for tracking memory leaks of
//               reference-counted objects: it keeps a record of every
//               such object currently allocated.
//
//               When compiled with NDEBUG set, this entire class does
//               nothing and compiles to nothing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS MemoryUsage {
public:
  INLINE static bool get_track_memory_usage();

#if defined(__GNUC__)
  // There seems to be a problem with egcs-2.91.66: it gets confused
  // with too many nested inline functions, and sets the wrong pointer
  // as 'this'.  Yucky.  The workaround is to make these functions
  // non-inline, but this is inner-loop stuff, and we'd rather not pay
  // the price universally.  So we only compile them non-inline when
  // we're building on GCC.
  static void record_pointer(ReferenceCount *ptr);
  static void update_type(ReferenceCount *ptr, TypeHandle type);
  static void update_type(ReferenceCount *ptr, TypedObject *typed_ptr);
  static void remove_pointer(ReferenceCount *ptr);
#else // __GNUC__
  INLINE static void record_pointer(ReferenceCount *ptr);
  INLINE static void update_type(ReferenceCount *ptr, TypeHandle type);
  INLINE static void update_type(ReferenceCount *ptr, TypedObject *typed_ptr);
  INLINE static void remove_pointer(ReferenceCount *ptr);
#endif // __GNUC__

public:
  static void *operator_new_handler(size_t size);
  static void operator_delete_handler(void *ptr);

#if defined(WIN32_VC) && defined(_DEBUG)
  static int win32_malloc_hook(int alloc_type, void *ptr, 
                               size_t size, int block_use, long request, 
                               const unsigned char *filename, int line);
#endif
    
PUBLISHED:
  INLINE static bool is_tracking();
  INLINE static bool is_counting();
  INLINE static size_t get_current_cpp_size();
  INLINE static bool has_cpp_size();
  INLINE static size_t get_cpp_size();
  INLINE static bool has_interpreter_size();
  INLINE static size_t get_interpreter_size();
  INLINE static bool has_total_size();
  INLINE static size_t get_total_size();

  INLINE static int get_num_pointers();
  INLINE static void get_pointers(MemoryUsagePointers &result);
  INLINE static void get_pointers_of_type(MemoryUsagePointers &result,
                                          TypeHandle type);
  INLINE static void get_pointers_of_age(MemoryUsagePointers &result,
                                         double from, double to);
  INLINE static void get_pointers_with_zero_count(MemoryUsagePointers &result);

  INLINE static void freeze();

  INLINE static void show_current_types();
  INLINE static void show_trend_types();
  INLINE static void show_current_ages();
  INLINE static void show_trend_ages();

private:
  MemoryUsage();
  static MemoryUsage *get_global_ptr();

  void ns_record_pointer(ReferenceCount *ptr);
  void ns_update_type(ReferenceCount *ptr, TypeHandle type);
  void ns_update_type(ReferenceCount *ptr, TypedObject *typed_ptr);
  void ns_remove_pointer(ReferenceCount *ptr);

  void ns_record_void_pointer(void *ptr, size_t size);
  void ns_remove_void_pointer(void *ptr);

  size_t ns_get_current_cpp_size();
  size_t ns_get_cpp_size();
  size_t ns_get_interpreter_size();
  size_t ns_get_total_size();
  int ns_get_num_pointers();
  void ns_get_pointers(MemoryUsagePointers &result);
  void ns_get_pointers_of_type(MemoryUsagePointers &result,
                               TypeHandle type);
  void ns_get_pointers_of_age(MemoryUsagePointers &result,
                              double from, double to);
  void ns_get_pointers_with_zero_count(MemoryUsagePointers &result);
  void ns_freeze();

  void ns_show_current_types();
  void ns_show_trend_types();
  void ns_show_current_ages();
  void ns_show_trend_ages();

  void consolidate_void_ptr(MemoryInfo &info);

  static MemoryUsage *_global_ptr;

  // We shouldn't use a pmap, since that would be recursive!
  // Actually, it turns out that it doesn't matter, since somehow the
  // pallocator gets used even though we specify dallocator here, so
  // we have to make special code that handles the recursion anyway.
  typedef map<void *, MemoryInfo, less<void *>, dallocator<MemoryInfo> > Table;
  Table _table;
  int _freeze_index;
  int _count;
  size_t _current_cpp_size;
  size_t _cpp_size;
  size_t _interpreter_size;
  size_t _total_size;

  class TypeHistogram {
  public:
    void add_info(TypeHandle type, MemoryInfo &info);
    void show() const;
    void clear();

  private:
    // Cannot use a pmap, since that would be recursive!
    typedef map<TypeHandle, MemoryUsagePointerCounts, 
      less<TypeHandle>, dallocator<MemoryUsagePointerCounts> > Counts;
    Counts _counts;
  };
  TypeHistogram _trend_types;

  class AgeHistogram {
  public:
    AgeHistogram();
    void add_info(double age, MemoryInfo &info);
    void show() const;
    void clear();

  private:
    int choose_bucket(double age) const;

    enum { num_buckets = 5 };
    MemoryUsagePointerCounts _counts[num_buckets];
    static double _cutoff[num_buckets];
  };
  AgeHistogram _trend_ages;


  bool _track_memory_usage;
  bool _count_memory_usage;
};

#include "memoryUsage.I"

#endif  // DO_MEMORY_USAGE

#endif

