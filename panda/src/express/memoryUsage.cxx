// Filename: memoryUsage.cxx
// Created by:  drose (25May00)
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

#include "memoryUsage.h"

#ifdef DO_MEMORY_USAGE

#include "memoryUsagePointers.h"
#include "trueClock.h"
#include "typedReferenceCount.h"
#include "mutexImpl.h"
#include "interrogate_request.h"

#if (defined(WIN32_VC) || defined (WIN64_VC)) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "config_express.h"
#include "configVariableInt64.h"
#include <algorithm>
#include <iterator>

MemoryUsage *MemoryUsage::_global_ptr;

// This flag is used to protect the operator new/delete handlers
// against recursive entry.
bool MemoryUsage::_recursion_protect = false;

// The cutoff ages, in seconds, for the various buckets in the AgeHistogram.
double MemoryUsage::AgeHistogram::_cutoff[MemoryUsage::AgeHistogram::num_buckets] = {
  0.0,
  0.1,
  1.0,
  10.0,
  60.0,
};


////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::TypeHistogram::add_info
//       Access: Public
//  Description: Adds a single entry to the histogram.
////////////////////////////////////////////////////////////////////
void MemoryUsage::TypeHistogram::
add_info(TypeHandle type, MemoryInfo *info) {
  _counts[type].add_info(info);
}


// This class is a temporary class used only in TypeHistogram::show(),
// below, to sort the types in descending order by counts.
class TypeHistogramCountSorter {
public:
  TypeHistogramCountSorter(const MemoryUsagePointerCounts &count, 
                           TypeHandle type) :
    _count(count),
    _type(type)
  {
  }
  bool operator < (const TypeHistogramCountSorter &other) const {
    return other._count < _count;
  }
  MemoryUsagePointerCounts _count;
  TypeHandle _type;
};

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::TypeHistogram::show
//       Access: Public
//  Description: Shows the contents of the histogram to nout.
////////////////////////////////////////////////////////////////////
void MemoryUsage::TypeHistogram::
show() const {
  // First, copy the relevant information to a vector so we can sort
  // by counts.  Don't use a pvector.
  typedef vector<TypeHistogramCountSorter> CountSorter;
  CountSorter count_sorter;
  Counts::const_iterator ci;
  for (ci = _counts.begin(); ci != _counts.end(); ++ci) {
    count_sorter.push_back
      (TypeHistogramCountSorter((*ci).second, (*ci).first));
  }

  sort(count_sorter.begin(), count_sorter.end());

  CountSorter::const_iterator vi;
  for (vi = count_sorter.begin(); vi != count_sorter.end(); ++vi) {
    TypeHandle type = (*vi)._type;
    if (type == TypeHandle::none()) {
      nout << "unknown";
    } else {
      nout << type;
    }
    nout << " : " << (*vi)._count << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::TypeHistogram::clear
//       Access: Public
//  Description: Resets the histogram in preparation for new data.
////////////////////////////////////////////////////////////////////
void MemoryUsage::TypeHistogram::
clear() {
  _counts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MemoryUsage::AgeHistogram::
AgeHistogram() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::add_info
//       Access: Public
//  Description: Adds a single entry to the histogram.
////////////////////////////////////////////////////////////////////
void MemoryUsage::AgeHistogram::
add_info(double age, MemoryInfo *info) {
  int bucket = choose_bucket(age);
  nassertv(bucket >= 0 && bucket < num_buckets);
  _counts[bucket].add_info(info);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::show
//       Access: Public
//  Description: Shows the contents of the histogram to nout.
////////////////////////////////////////////////////////////////////
void MemoryUsage::AgeHistogram::
show() const {
  for (int i = 0; i < num_buckets - 1; i++) {
    nout << _cutoff[i] << " to " << _cutoff[i + 1] << " seconds old : ";
    _counts[i].output(nout);
    nout << "\n";
  }
  nout << _cutoff[num_buckets - 1] << " seconds old and up : ";
  _counts[num_buckets - 1].output(nout);
  nout << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::clear
//       Access: Public
//  Description: Resets the histogram in preparation for new data.
////////////////////////////////////////////////////////////////////
void MemoryUsage::AgeHistogram::
clear() {
  for (int i = 0; i < num_buckets; i++) {
    _counts[i].clear();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::choose_bucket
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int MemoryUsage::AgeHistogram::
choose_bucket(double age) const {
  for (int i = num_buckets - 1; i >= 0; i--) {
    if (age >= _cutoff[i]) {
      return i;
    }
  }
  express_cat.error()
    << "No suitable bucket for age " << age << "\n";
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::heap_alloc_single
//       Access: Public, Virtual
//  Description: Allocates a block of memory from the heap, similar to
//               malloc().  This will never return NULL; it will abort
//               instead if memory is not available.
////////////////////////////////////////////////////////////////////
void *MemoryUsage::
heap_alloc_single(size_t size) {
  void *ptr;

  if (_recursion_protect) {
    ptr = MemoryHook::heap_alloc_single(size);
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Allocating pointer " << (void *)ptr
        << " during recursion protect.\n";
    }

  } else {
    if (_track_memory_usage) {
      ptr = MemoryHook::heap_alloc_single(size);
      /*
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Allocating pointer " << (void *)ptr
          << " of size " << size << ".\n";
      }
      */

      get_global_ptr()->ns_record_void_pointer(ptr, size);

    } else {
      ptr = MemoryHook::heap_alloc_single(size);
    }
  }

  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::heap_free_single
//       Access: Public, Virtual
//  Description: Releases a block of memory previously allocated via
//               heap_alloc_single.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
heap_free_single(void *ptr) {
  if (_recursion_protect) {
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Deleting pointer " << (void *)ptr
        << " during recursion protect.\n";
    }
    MemoryHook::heap_free_single(ptr);

  } else {
    if (_track_memory_usage) {
      /*
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Removing pointer " << (void *)ptr << "\n";
      }
      */
      ns_remove_void_pointer(ptr);
      MemoryHook::heap_free_single(ptr);
    } else {
      MemoryHook::heap_free_single(ptr);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::heap_alloc_array
//       Access: Public, Virtual
//  Description: Allocates a block of memory from the heap, similar to
//               malloc().  This will never return NULL; it will abort
//               instead if memory is not available.
////////////////////////////////////////////////////////////////////
void *MemoryUsage::
heap_alloc_array(size_t size) {
  void *ptr;

  if (_recursion_protect) {
    ptr = MemoryHook::heap_alloc_array(size);
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Allocating array pointer " << (void *)ptr
        << " during recursion protect.\n";
    }

  } else {
    if (_track_memory_usage) {
      ptr = MemoryHook::heap_alloc_array(size);
      /*
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Allocating array pointer " << (void *)ptr
          << " of size " << size << ".\n";
      }
      */

      get_global_ptr()->ns_record_void_pointer(ptr, size);

    } else {
      ptr = MemoryHook::heap_alloc_array(size);
    }
  }

  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::heap_realloc_array
//       Access: Public, Virtual
//  Description: Resizes a block of memory previously returned from
//               heap_alloc_array.
////////////////////////////////////////////////////////////////////
void *MemoryUsage::
heap_realloc_array(void *ptr, size_t size) {
  if (_recursion_protect) {
    ptr = MemoryHook::heap_realloc_array(ptr, size);
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Reallocating array pointer " << (void *)ptr
        << " during recursion protect.\n";
    }

  } else {
    if (_track_memory_usage) {
      get_global_ptr()->ns_remove_void_pointer(ptr);
      ptr = MemoryHook::heap_realloc_array(ptr, size);
      /*
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Reallocating array pointer " << (void *)ptr
          << " to size " << size << ".\n";
      }
      */

      get_global_ptr()->ns_record_void_pointer(ptr, size);

    } else {
      ptr = MemoryHook::heap_realloc_array(ptr, size);
    }
  }

  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::heap_free_array
//       Access: Public, Virtual
//  Description: Releases a block of memory previously allocated via
//               heap_alloc_array.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
heap_free_array(void *ptr) {
  if (_recursion_protect) {
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Deleting pointer " << (void *)ptr
        << " during recursion protect.\n";
    }
    MemoryHook::heap_free_array(ptr);

  } else {
    if (_track_memory_usage) {
      /*
      if (express_cat.is_spam()) {
        express_cat.spam()
          << "Removing pointer " << (void *)ptr << "\n";
      }
      */
      ns_remove_void_pointer(ptr);
      MemoryHook::heap_free_array(ptr);
    } else {
      MemoryHook::heap_free_array(ptr);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::mark_pointer
//       Access: Public, Virtual
//  Description: This special method exists only to provide a callback
//               hook into MemoryUsage.  It indicates that the
//               indicated pointer, allocated from somewhere other
//               than a call to heap_alloc(), now contains a pointer
//               to the indicated ReferenceCount object.  If orig_size
//               is 0, it indicates that the ReferenceCount object has
//               been destroyed.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
mark_pointer(void *ptr, size_t size, ReferenceCount *ref_ptr) {
  if (_recursion_protect || !_track_memory_usage) {
    return;
  }

  if (express_cat.is_spam()) {
    express_cat.spam()
      << "Marking pointer " << ptr << ", size " << size 
      << ", ref_ptr = " << ref_ptr << "\n";
  }

  if (size != 0) {
    // We're recording this pointer as now in use.
    ns_record_void_pointer(ptr, size);

    if (ref_ptr != (ReferenceCount *)NULL) {
      // Make the pointer typed.  This is particularly necessary in
      // case the ref_ptr is a different value than the base void
      // pointer; this may be our only opportunity to associate the
      // two pointers.
      Table::iterator ti;
      ti = _table.find(ptr);
      nassertv(ti != _table.end());
      MemoryInfo *info = (*ti).second;

      info->_ref_ptr = ref_ptr;
      info->_static_type = ReferenceCount::get_class_type();
      info->_dynamic_type = ReferenceCount::get_class_type();
      info->_flags |= MemoryInfo::F_reconsider_dynamic_type;
      
      if (ref_ptr != ptr) {
        _recursion_protect = true;
        
        pair<Table::iterator, bool> insert_result =
          _table.insert(Table::value_type((void *)ref_ptr, info));
        assert(insert_result.first != _table.end());
        if (!insert_result.second) {
          express_cat.warning()
            << "Attempt to mark pointer " << ptr << " as ReferenceCount "
            << ref_ptr << ", which was already allocated.\n";
        }
        
        _recursion_protect = false;
      }
    }

  } else {
    // We're removing this pointer from use.
    ns_remove_void_pointer(ptr);
  }
}

#if (defined(WIN32_VC) || defined (WIN64_VC))&& defined(_DEBUG)
////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::win32_malloc_hook
//       Access: Public, Static
//  Description: This callback is attached to the Win32 debug malloc
//               system to be called whenever a pointer is allocated,
//               reallocated, or freed.  It's used to track the total
//               memory allocated via calls to malloc().
////////////////////////////////////////////////////////////////////
int MemoryUsage::
win32_malloc_hook(int alloc_type, void *ptr, 
                  size_t size, int block_use, long request, 
                  const unsigned char *filename, int line) {
  MemoryUsage *mu = get_global_ptr();
  int increment = 0;
  switch (alloc_type) {
  case _HOOK_ALLOC:
    increment = size;
    break;
    
  case _HOOK_REALLOC:
    increment = size - _msize(ptr);
    break;
    
  case _HOOK_FREE:
    increment = - ((int)_msize(ptr));
    break;
  }
  
  mu->_total_size += increment;
  return true;
}
#endif  // WIN32_VC && _DEBUG



////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
MemoryUsage::
MemoryUsage(const MemoryHook &copy) : MemoryHook(copy) {
  // We must get these variables here instead of in
  // config_express.cxx, because we need to know it at static init
  // time, and who knows when the code in config_express will be
  // executed.

  _track_memory_usage = ConfigVariableBool
    ("track-memory-usage", false,
     PRC_DESC("Set this to true to enable full-force tracking of C++ allocations "
              "and recordkeeping by type.  It's quite expensive."));

  // Since enabling this after startup might cause bogus errors, we'd
  // like to know if this happened, so we can squelch those error
  // messages.
  _startup_track_memory_usage = _track_memory_usage;

  // Make sure the express category has been instantiated.
  express_cat->is_info();

  _report_memory_usage = ConfigVariableBool
    ("report-memory-usage", false,
     PRC_DESC("Set this true to enable automatic reporting of allocated objects "
              "at the interval specified by report-memory-interval.  This also "
              "requires track-memory-usage."));
  _report_memory_interval = ConfigVariableDouble
    ("report-memory-interval", 5.0,
     PRC_DESC("This is the interval, in seconds, for reports of currently allocated "
              "memory, when report-memory-usage is true."));
  _last_report_time = 0.0;

  _count_memory_usage = false;

  PN_int64 max_heap_size = ConfigVariableInt64
    ("max-heap-size", 0,
     PRC_DESC("If this is nonzero, it is the maximum number of bytes expected "
              "to be allocated on the heap before we enter report-memory-usage "
              "mode automatically.  The assumption is that once this limit "
              "has been crossed, we must be leaking."));
  if (max_heap_size != 0) {
    _max_heap_size = (size_t)max_heap_size;
  }

#ifdef USE_MEMORY_NOWRAPPERS
#error Cannot compile MemoryUsage without malloc wrappers!
#endif

#if (defined(WIN32_VC) || defined(WIN64_VC)) && defined(_DEBUG)
  // On a debug Windows build, we can set this malloc hook which
  // allows tracking every malloc call, even from subordinate
  // libraries.
  _CrtSetAllocHook(&win32_malloc_hook);
  _count_memory_usage = true;
#endif

  _info_set_dirty = false;
  _freeze_index = 0;
  _count = 0;
  _current_cpp_size = 0;
  _total_cpp_size = 0;
  _total_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::overflow_heap_size
//       Access: Protected, Virtual
//  Description: This callback method is called whenever the total
//               allocated heap size exceeds _max_heap_size.  It's
//               mainly intended for reporting memory leaks, on the
//               assumption that once we cross some specified
//               threshold, we're just leaking memory.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
overflow_heap_size() {
  MemoryHook::overflow_heap_size();

  express_cat.error()
    << "Total allocated memory has reached "
    << get_panda_heap_single_size() + get_panda_heap_array_size()
    << " bytes."
    << "\n  heap single: " << get_panda_heap_single_size()
    << "\n  heap array: " << get_panda_heap_array_size()
    << "\n  heap overhead: " << get_panda_heap_overhead()
    << "\n  mmap: " << get_panda_mmap_size()
    << "\n  external: " << get_external_size()
    << "\n  total: " << get_total_size()
    << "\n";

  // Turn on spamful debugging.
  _track_memory_usage = true;
  _report_memory_usage = true;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_record_pointer
//       Access: Private
//  Description: Indicates that the given pointer has been recently
//               allocated.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_record_pointer(ReferenceCount *ptr) {
  if (_track_memory_usage) {
    // We have to protect modifications to the table from recursive
    // calls by toggling _recursion_protect while we adjust it.
    _recursion_protect = true;
    pair<Table::iterator, bool> insert_result =
      _table.insert(Table::value_type((void *)ptr, (MemoryInfo *)NULL));
    
    // This shouldn't fail.
    assert(insert_result.first != _table.end());

    if (insert_result.second) {
      (*insert_result.first).second = new MemoryInfo;
      _info_set_dirty = true;
      ++_count;
    }

    MemoryInfo *info = (*insert_result.first).second;

    // We might already have a ReferenceCount pointer, thanks to a
    // previous call to mark_pointer().
    nassertv(info->_ref_ptr == NULL || info->_ref_ptr == ptr);

    info->_ref_ptr = ptr;
    info->_static_type = ReferenceCount::get_class_type();
    info->_dynamic_type = ReferenceCount::get_class_type();
    info->_time = TrueClock::get_global_ptr()->get_long_time();
    info->_freeze_index = _freeze_index;
    info->_flags |= MemoryInfo::F_reconsider_dynamic_type;

    // We close the recursion_protect flag all the way down here, so
    // that we also protect ourselves against a possible recursive
    // call in TrueClock::get_global_ptr().
    _recursion_protect = false;

    if (_report_memory_usage) {
      double now = TrueClock::get_global_ptr()->get_long_time();
      if (now - _last_report_time > _report_memory_interval) {
        _last_report_time = now;
        express_cat.info()
          << "*** Current memory usage: " << get_total_size() << "\n";
        show_current_types();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_update_type
//       Access: Private
//  Description: Associates the indicated type with the given pointer.
//               This should be called by functions (e.g. the
//               constructor) that know more specifically what type of
//               thing we've got; otherwise, the MemoryUsage database
//               will know only that it's a "ReferenceCount".
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_update_type(ReferenceCount *ptr, TypeHandle type) {
  if (_track_memory_usage) {
    Table::iterator ti;
    ti = _table.find(ptr);
    if (ti == _table.end()) {
      if (_startup_track_memory_usage) {
        express_cat.error()
          << "Attempt to update type to " << type << " for unrecorded pointer "
          << (void *)ptr << "!\n";
        nassertv(false);
      }
      return;
    }

    MemoryInfo *info = (*ti).second;

    info->update_type_handle(info->_static_type, type);
    info->determine_dynamic_type();

    consolidate_void_ptr(info);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_update_type
//       Access: Private
//  Description: Associates the indicated type with the given pointer.
//               This flavor of update_type() also passes in the
//               pointer as a TypedObject, and useful for objects that
//               are, in fact, TypedObjects.  Once the MemoryUsage
//               database has the pointer as a TypedObject it doesn't
//               need any more help.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_update_type(ReferenceCount *ptr, TypedObject *typed_ptr) {
  if (_track_memory_usage) {
    Table::iterator ti;
    ti = _table.find(ptr);
    if (ti == _table.end()) {
      if (_startup_track_memory_usage) {
        express_cat.error()
          << "Attempt to update type to " << typed_ptr->get_type()
          << " for unrecorded pointer "
          << (void *)ptr << "!\n";
      }
      return;
    }

    MemoryInfo *info = (*ti).second;
    info->_typed_ptr = typed_ptr;
    info->determine_dynamic_type();

    consolidate_void_ptr(info);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_remove_pointer
//       Access: Private
//  Description: Indicates that the given pointer has been recently
//               freed.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_remove_pointer(ReferenceCount *ptr) {
  if (_track_memory_usage) {
    Table::iterator ti;
    ti = _table.find(ptr);
    if (ti == _table.end()) {
      if (_startup_track_memory_usage) {
        express_cat.error()
          << "Attempt to remove pointer " << (void *)ptr
          << ", not in table.\n"
          << "Possibly a double-destruction.\n";
        nassertv(false);
      }
      return;
    }

    MemoryInfo *info = (*ti).second;

    if (info->_ref_ptr == NULL) {
      express_cat.error()
        << "Pointer " << (void *)ptr << " deleted twice!\n";
      return;
    }
    nassertv(info->_ref_ptr == ptr);

    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Removing ReferenceCount pointer " << (void *)ptr << "\n";
    }

    info->_ref_ptr = (ReferenceCount *)NULL;
    info->_typed_ptr = (TypedObject *)NULL;

    if (info->_freeze_index == _freeze_index) {
      double now = TrueClock::get_global_ptr()->get_long_time();

      // We have to protect modifications to the table from recursive
      // calls by toggling _recursion_protect while we adjust it.
      _recursion_protect = true;
      _trend_types.add_info(info->get_type(), info);
      _trend_ages.add_info(now - info->_time, info);
      _recursion_protect = false;
    }

    if (ptr != info->_void_ptr || info->_void_ptr == NULL) {
      // Remove the entry from the table.

      // We have to protect modifications to the table from recursive
      // calls by toggling _recursion_protect while we adjust it.
      _recursion_protect = true;
      _table.erase(ti);
      _recursion_protect = false;

      if (info->_void_ptr == NULL) {
        // That was the last entry.  Remove it altogether.
        _total_cpp_size -= info->_size;
        if (info->_freeze_index == _freeze_index) {
          _current_cpp_size -= info->_size;
          _count--;
        }

        _info_set_dirty = true;
        delete info;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_record_void_pointer
//       Access: Private
//  Description: Records a pointer that's not even necessarily a
//               ReferenceCount object (but for which we know the size
//               of the allocated structure).
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_record_void_pointer(void *ptr, size_t size) {
  if (_track_memory_usage) {
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Recording void pointer " << (void *)ptr << "\n";
    }

    // We have to protect modifications to the table from recursive
    // calls by toggling _recursion_protect while we adjust it.

    _recursion_protect = true;
    pair<Table::iterator, bool> insert_result =
      _table.insert(Table::value_type((void *)ptr, (MemoryInfo *)NULL));

    assert(insert_result.first != _table.end());

    if (insert_result.second) {
      (*insert_result.first).second = new MemoryInfo;
      _info_set_dirty = true;
      ++_count;
    }

    MemoryInfo *info = (*insert_result.first).second;

    // We shouldn't already have a void pointer.
    if (info->_void_ptr != (void *)NULL) {
      express_cat.error()
        << "Void pointer " << (void *)ptr << " recorded twice!\n";
      nassertv(false);
    }

    if (info->_freeze_index == _freeze_index) {
      _current_cpp_size += size - info->_size;
    } else {
      _current_cpp_size += size;
    }
    _total_cpp_size += size - info->_size;

    info->_void_ptr = ptr;
    info->_size = size;
    info->_time = TrueClock::get_global_ptr()->get_long_time();
    info->_freeze_index = _freeze_index;
    info->_flags |= MemoryInfo::F_size_known;

    // We close the recursion_protect flag all the way down here, so
    // that we also protect ourselves against a possible recursive
    // call in TrueClock::get_global_ptr().
    _recursion_protect = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_remove_void_pointer
//       Access: Private
//  Description: Removes a pointer previously recorded via
//               record_void_pointer.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_remove_void_pointer(void *ptr) {
  if (_track_memory_usage) {
    if (express_cat.is_spam()) {
      express_cat.spam()
        << "Removing void pointer " << (void *)ptr << "\n";
    }

    Table::iterator ti;
    ti = _table.find(ptr);
    if (ti == _table.end()) {
      // The pointer we tried to delete was not recorded in the table.

      // We can't report this as an error, because (a) we might have
      // removed the void pointer entry already when we consolidated,
      // and (b) a few objects might have been created during static
      // init time, before we grabbed the operator new/delete function
      // handlers.
      return;
    }

    MemoryInfo *info = (*ti).second;

    if (info->_void_ptr == (void *)NULL) {
      express_cat.error()
        << "Pointer " << (void *)ptr << " deleted twice!\n";
      return;
    }
    nassertv(info->_void_ptr == ptr);

    if (info->_ref_ptr != (ReferenceCount *)NULL) {
      express_cat.error()
        << "Pointer " << (void *)ptr
        << " did not destruct before being deleted!\n";
      if (info->_ref_ptr != ptr) {
        remove_pointer(info->_ref_ptr);
      }
    }

    info->_void_ptr = NULL;

    // Remove it from the table.

    // We have to protect modifications to the table from recursive
    // calls by toggling _recursion_protect while we adjust it.
    _recursion_protect = true;
    _table.erase(ti);
    _recursion_protect = false;

    _total_cpp_size -= info->_size;
    if (info->_freeze_index == _freeze_index) {
      --_count;
      _current_cpp_size -= info->_size;
    }

    _info_set_dirty = true;
    delete info;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_num_pointers
//       Access: Private
//  Description: Returns the number of pointers currently active.
////////////////////////////////////////////////////////////////////
int MemoryUsage::
ns_get_num_pointers() {
  nassertr(_track_memory_usage, 0);
  return _count;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_pointers
//       Access: Private
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers currently active.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_get_pointers(MemoryUsagePointers &result) {
  nassertv(_track_memory_usage);
  result.clear();

  if (_info_set_dirty) {
    refresh_info_set();
  }

  double now = TrueClock::get_global_ptr()->get_long_time();
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index &&
        info->_ref_ptr != (ReferenceCount *)NULL) {
      result.add_entry(info->_ref_ptr, info->_typed_ptr, info->get_type(),
                       now - info->_time);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_pointers_of_type
//       Access: Private
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers of the indicated type currently
//               active.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_get_pointers_of_type(MemoryUsagePointers &result, TypeHandle type) {
  nassertv(_track_memory_usage);
  result.clear();

  if (_info_set_dirty) {
    refresh_info_set();
  }

  double now = TrueClock::get_global_ptr()->get_long_time();
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index &&
        info->_ref_ptr != (ReferenceCount *)NULL) {
      TypeHandle info_type = info->get_type();
      if (info_type != TypeHandle::none() &&
          info_type.is_derived_from(type)) {
        result.add_entry(info->_ref_ptr, info->_typed_ptr, info_type,
                         now - info->_time);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_pointers_of_age
//       Access: Private
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers that were allocated within the range
//               of the indicated number of seconds ago.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_get_pointers_of_age(MemoryUsagePointers &result,
                       double from, double to) {
  nassertv(_track_memory_usage);
  result.clear();

  if (_info_set_dirty) {
    refresh_info_set();
  }

  double now = TrueClock::get_global_ptr()->get_long_time();
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index &&
        info->_ref_ptr != (ReferenceCount *)NULL) {
      double age = now - info->_time;
      if ((age >= from && age <= to) ||
          (age >= to && age <= from)) {
        result.add_entry(info->_ref_ptr, info->_typed_ptr, info->get_type(), age);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_pointers_with_zero_count
//       Access: Private
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all currently active pointers (that is, pointers
//               allocated since the last call to freeze(), and not
//               yet freed) that have a zero reference count.
//
//               Generally, an undeleted pointer with a zero reference
//               count means its reference count has never been
//               incremented beyond zero (since once it has been
//               incremented, the only way it can return to zero would
//               free the pointer).  This may include objects that are
//               allocated statically or on the stack, which are never
//               intended to be deleted.  Or, it might represent a
//               programmer or compiler error.
//
//               This function has the side-effect of incrementing
//               each of their reference counts by one, thus
//               preventing them from ever being freed--but since they
//               hadn't been freed anyway, probably no additional harm
//               is done.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_get_pointers_with_zero_count(MemoryUsagePointers &result) {
  nassertv(_track_memory_usage);
  result.clear();

  if (_info_set_dirty) {
    refresh_info_set();
  }

  double now = TrueClock::get_global_ptr()->get_long_time();
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index && 
        info->_ref_ptr != (ReferenceCount *)NULL) {
      if (info->_ref_ptr->get_ref_count() == 0) {
        info->_ref_ptr->ref();
        result.add_entry(info->_ref_ptr, info->_typed_ptr, info->get_type(),
                         now - info->_time);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_freeze
//       Access: Private
//  Description: 'Freezes' all pointers currently stored so that they
//               are no longer reported; only newly allocate pointers
//               from this point on will appear in future information
//               requests.  This makes it easier to differentiate
//               between continuous leaks and one-time memory
//               allocations.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_freeze() {
  _count = 0;
  _current_cpp_size = 0;
  _trend_types.clear();
  _trend_ages.clear();
  _freeze_index++;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_show_current_types
//       Access: Private
//  Description: Shows the breakdown of types of all of the
//               active pointers.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_show_current_types() {
  nassertv(_track_memory_usage);
  TypeHistogram hist;

  if (_info_set_dirty) {
    refresh_info_set();
  }

  _recursion_protect = true;
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index) {
      hist.add_info(info->get_type(), info);
    }
  }
  hist.show();
  _recursion_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_show_trend_types
//       Access: Private
//  Description: Shows the breakdown of types of all of the
//               pointers allocated and freed since the last call to
//               freeze().
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_show_trend_types() {
  _trend_types.show();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_show_current_ages
//       Access: Private
//  Description: Shows the breakdown of ages of all of the
//               active pointers.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_show_current_ages() {
  nassertv(_track_memory_usage);

  AgeHistogram hist;
  double now = TrueClock::get_global_ptr()->get_long_time();

  _recursion_protect = true;
  InfoSet::iterator si;
  for (si = _info_set.begin(); si != _info_set.end(); ++si) {
    MemoryInfo *info = (*si);
    if (info->_freeze_index == _freeze_index) {
      hist.add_info(now - info->_time, info);
    }
  }

  hist.show();
  _recursion_protect = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_show_trend_ages
//       Access: Private
//  Description: Shows the breakdown of ages of all of the
//               pointers allocated and freed since the last call to
//               freeze().
////////////////////////////////////////////////////////////////////
void MemoryUsage::
ns_show_trend_ages() {
  _trend_ages.show();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::consolidate_void_ptr
//       Access: Private
//  Description: If the size information has not yet been determined
//               for this pointer, checks to see if it has possibly
//               been recorded under the TypedObject pointer (this
//               will happen when the class inherits from TypedObject
//               before ReferenceCount, e.g. TypedReferenceCount).
////////////////////////////////////////////////////////////////////
void MemoryUsage::
consolidate_void_ptr(MemoryInfo *info) {
  if (info->is_size_known()) {
    // We already know the size, so no sweat.
    return;
  }

  if (info->_typed_ptr == (TypedObject *)NULL) {
    // We don't have a typed pointer for this thing yet.
    return;
  }
  
  TypedObject *typed_ptr = info->_typed_ptr;

  if ((void *)typed_ptr == (void *)info->_ref_ptr) {
    // The TypedObject pointer is the same pointer as the
    // ReferenceCount pointer, so there's no point in looking it up
    // separately.  Actually, this really shouldn't even be possible.
    return;
  }

  nassertv(info->_void_ptr == NULL);

  Table::iterator ti;
  ti = _table.find(typed_ptr);
  if (ti == _table.end()) {
    // No entry for the typed pointer, either.
    return;
  }

  // We do have an entry!  Copy over the relevant pieces.
  MemoryInfo *typed_info = (*ti).second;

  nassertv(typed_info->_void_ptr == typed_ptr &&
           typed_info->_ref_ptr == NULL);

  info->_void_ptr = typed_info->_void_ptr;
  if (typed_info->is_size_known()) {
    info->_size = typed_info->get_size();
    info->_flags |= MemoryInfo::F_size_known;
    if (typed_info->_freeze_index == _freeze_index) {
      _current_cpp_size += info->_size;
    }
  }

  // Now that we've consolidated the pointers, remove the entry for
  // the typed pointer.
  if (info->_freeze_index == _freeze_index) {
    _count--;
    _current_cpp_size -= info->_size;
  }

  _info_set_dirty = true;
  delete typed_info;

  (*ti).second = info;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::refresh_info_set
//       Access: Private
//  Description: Recomputes the _info_set table, if necessary.  This
//               table stores a unique entry for each MemoryInfo
//               object in _table.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
refresh_info_set() {
  if (!_info_set_dirty) {
    return;
  }

  // We have to protect modifications to the table from recursive
  // calls by toggling _recursion_protect while we adjust it.
  _recursion_protect = true;
  
  _info_set.clear();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    _info_set.insert((*ti).second);
  }

  _recursion_protect = false;

  _info_set_dirty = false;
}


#endif  // DO_MEMORY_USAGE
