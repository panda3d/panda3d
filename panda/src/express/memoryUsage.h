// Filename: memoryUsage.h
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include <pandabase.h>

#include "typeHandle.h"

#include <map>

class ReferenceCount;
class MemoryUsagePointers;

////////////////////////////////////////////////////////////////////
// 	 Class : MemoryUsage
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

PUBLISHED:
#if defined(__GNUC__) && !defined(NDEBUG)
  // There seems to be a problem with egcs-2.91.66: it gets confused
  // with too many nested inline functions, and sets the wrong pointer
  // as 'this'.  Yucky.  The workaround is to make these functions
  // non-inline, but this is inner-loop stuff, and we'd rather not pay
  // the price universally.  So we only compile them non-inline when
  // we're building on GCC and not building in NDEBUG mode (in NDEBUG
  // mode, these functions do nothing anyway).
  static void record_pointer(ReferenceCount *ptr);
  static void update_type(ReferenceCount *ptr, TypeHandle type);
  static void remove_pointer(ReferenceCount *ptr);
#else // __GNUC__ && !NDEBUG
  INLINE static void record_pointer(ReferenceCount *ptr);
  INLINE static void update_type(ReferenceCount *ptr, TypeHandle type);
  INLINE static void remove_pointer(ReferenceCount *ptr);
#endif // __GNUC__ && !NDEBUG

#ifndef NDEBUG
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
  void ns_remove_pointer(ReferenceCount *ptr);

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

  static MemoryUsage *_global_ptr;

  class MemoryInfo {
  public:
    TypeHandle get_type();
    void determine_dynamic_type();
    void update_type_handle(TypeHandle &destination, TypeHandle refined);

    ReferenceCount *_ptr;
    TypeHandle _static_type;
    TypeHandle _dynamic_type;
    
    double _time;
    int _freeze_index;
    bool _reconsider_dynamic_type;
  };

  typedef map<ReferenceCount *, MemoryInfo> Table;
  Table _table;
  int _freeze_index;
  int _count;


  class TypeHistogram {
  public:
    void add_info(TypeHandle type);
    void show() const;
    void clear();

  private:
    typedef map<TypeHandle, int> Counts;
    Counts _counts;
  };
  TypeHistogram _trend_types;

  class AgeHistogram {
  public:
    AgeHistogram();
    void add_info(double age);
    void show() const;
    void clear();

  private:
    int choose_bucket(double age) const;

    enum { num_buckets = 5 };
    int _counts[num_buckets];
    static double _cutoff[num_buckets];
  };
  AgeHistogram _trend_ages;


  bool _track_memory_usage;

#endif
};

#include "memoryUsage.I"

#endif

