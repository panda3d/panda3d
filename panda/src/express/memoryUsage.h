/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsage.h
 * @author drose
 * @date 2000-05-25
 */

#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include "pandabase.h"
#include "typedObject.h"
#include "memoryInfo.h"
#include "memoryUsagePointerCounts.h"
#include "pmap.h"
#include "memoryHook.h"

class ReferenceCount;
class MemoryUsagePointers;

/**
 * This class is used strictly for debugging purposes, specifically for
 * tracking memory leaks of reference-counted objects: it keeps a record of
 * every such object currently allocated.
 *
 * When compiled with NDEBUG set, this entire class does nothing and compiles
 * to a stub.
 */
class EXPCL_PANDA_EXPRESS MemoryUsage : public MemoryHook {
public:
  ALWAYS_INLINE static bool get_track_memory_usage();

  INLINE static void record_pointer(ReferenceCount *ptr);
  INLINE static void record_pointer(void *ptr, TypeHandle type);
  INLINE static void update_type(ReferenceCount *ptr, TypeHandle type);
  INLINE static void update_type(ReferenceCount *ptr, TypedObject *typed_ptr);
  INLINE static void update_type(void *ptr, TypeHandle type);
  INLINE static void remove_pointer(ReferenceCount *ptr);

protected:
  // These are not marked public, but they can be accessed via the MemoryHook
  // base class.
  virtual void *heap_alloc_single(size_t size);
  virtual void heap_free_single(void *ptr);

  virtual void *heap_alloc_array(size_t size);
  virtual void *heap_realloc_array(void *ptr, size_t size);
  virtual void heap_free_array(void *ptr);

  virtual void mark_pointer(void *ptr, size_t orig_size, ReferenceCount *ref_ptr);

#if (defined(WIN32_VC) || defined(WIN64_VC)) && defined(_DEBUG)
  static int win32_malloc_hook(int alloc_type, void *ptr,
                               size_t size, int block_use, long request,
                               const unsigned char *filename, int line);
#endif

PUBLISHED:
  INLINE static bool is_tracking();
  INLINE static bool is_counting();
  INLINE static size_t get_current_cpp_size();
  INLINE static size_t get_total_cpp_size();

  INLINE static size_t get_panda_heap_single_size();
  INLINE static size_t get_panda_heap_array_size();
  INLINE static size_t get_panda_heap_overhead();
  INLINE static size_t get_panda_mmap_size();
  INLINE static size_t get_external_size();
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

PUBLISHED:
  MAKE_PROPERTY(tracking, is_tracking);
  MAKE_PROPERTY(counting, is_counting);
  MAKE_PROPERTY(current_cpp_size, get_current_cpp_size);
  MAKE_PROPERTY(total_cpp_size, get_total_cpp_size);

  MAKE_PROPERTY(panda_heap_single_size, get_panda_heap_single_size);
  MAKE_PROPERTY(panda_heap_array_size, get_panda_heap_array_size);
  MAKE_PROPERTY(panda_heap_overhead, get_panda_heap_overhead);
  MAKE_PROPERTY(panda_mmap_size, get_panda_mmap_size);
  MAKE_PROPERTY(external_size, get_external_size);
  MAKE_PROPERTY(total_size, get_total_size);

protected:
  virtual void overflow_heap_size();

private:
  MemoryUsage(const MemoryHook &copy);
  INLINE static MemoryUsage *get_global_ptr();

  static void init_memory_usage();

  void ns_record_pointer(ReferenceCount *ptr);
  void ns_record_pointer(void *ptr, TypeHandle type);
  void ns_update_type(void *ptr, TypeHandle type);
  void ns_update_type(void *ptr, TypedObject *typed_ptr);
  void ns_remove_pointer(ReferenceCount *ptr);

  void ns_record_void_pointer(void *ptr, size_t size);
  void ns_remove_void_pointer(void *ptr);

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

#ifdef DO_MEMORY_USAGE
  void consolidate_void_ptr(MemoryInfo *info);
  void refresh_info_set();
#endif

  static MemoryUsage *_global_ptr;

  // We shouldn't use a pmap, since that would be recursive!  Actually, it
  // turns out that it doesn't matter, since somehow the pallocator gets used
  // even though we don't specify it here, so we have to make special code
  // that handles the recursion anyway.

/*
 * This table stores up to two entiries for each MemoryInfo object: one for
 * the void pointer (the pointer to the beginning of the allocated memory
 * block), and one for the ReferenceCount pointer.  For a particular object,
 * these two pointers may be the same or they may be different.  Some objects
 * may be stored under both pointers, while others may be stored under only
 * one pointer or the other.  We don't store an entry for an object's
 * TypedObject pointer.
 */
  typedef std::map<void *, MemoryInfo *> Table;
  Table _table;

  // This table indexes the individual MemoryInfo objects, for unique
  // iteration.
  typedef std::set<MemoryInfo *> InfoSet;
  InfoSet _info_set;
  bool _info_set_dirty;

  int _freeze_index;
  int _count;
  size_t _current_cpp_size;
  size_t _total_cpp_size;
  size_t _total_size;

  class TypeHistogram {
  public:
    void add_info(TypeHandle type, MemoryInfo *info);
    void show() const;
    void clear();

  private:
    // Cannot use a pmap, since that would be recursive!
    typedef std::map<TypeHandle, MemoryUsagePointerCounts> Counts;
    Counts _counts;
  };
  TypeHistogram _trend_types;

  class AgeHistogram {
  public:
    AgeHistogram();
    void add_info(double age, MemoryInfo *info);
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
  bool _startup_track_memory_usage;
  bool _count_memory_usage;
  bool _report_memory_usage;
  double _report_memory_interval;
  double _last_report_time;

  static bool _recursion_protect;
};

#include "memoryUsage.I"

#endif
