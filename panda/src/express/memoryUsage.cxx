// Filename: memoryUsage.cxx
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

#ifdef DO_MEMORY_USAGE

#include "memoryUsagePointers.h"
#include "trueClock.h"
#include "typedReferenceCount.h"

#include "memoryUsage.h"
#include "interrogate_request.h"

#if defined(WIN32_VC) && defined(_DEBUG)
#include <crtdbg.h>
#endif

#include "config_express.h"
#include <algorithm>

// This flag is set true in is_counting() mode to indicate that the
// malloc operation is coming from C++ operator new or delete.
static bool _is_cpp_operator = false;

// This flag is used to protect the operator new/delete handlers
// against recursive entry.
static bool _recursion_protect = false;

MemoryUsage *MemoryUsage::_global_ptr = (MemoryUsage *)NULL;

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
add_info(TypeHandle type, MemoryInfo &info) {
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
  typedef vector<TypeHistogramCountSorter, dallocator<TypeHistogramCountSorter> > CountSorter;
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
add_info(double age, MemoryInfo &info) {
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

#if defined(__GNUC__)

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::record_pointer
//       Access: Public, Static
//  Description: Indicates that the given pointer has been recently
//               allocated.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
record_pointer(ReferenceCount *ptr) {
  get_global_ptr()->ns_record_pointer(ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::update_type
//       Access: Public, Static
//  Description: Associates the indicated type with the given pointer.
//               This should be called by functions (e.g. the
//               constructor) that know more specifically what type of
//               thing we've got; otherwise, the MemoryUsage database
//               will know only that it's a "ReferenceCount".
////////////////////////////////////////////////////////////////////
void MemoryUsage::
update_type(ReferenceCount *ptr, TypeHandle type) {
  get_global_ptr()->ns_update_type(ptr, type);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::update_type
//       Access: Public, Static
//  Description: Associates the indicated type with the given pointer.
//               This flavor of update_type() also passes in the
//               pointer as a TypedObject, and useful for objects that
//               are, in fact, TypedObjects.  Once the MemoryUsage
//               database has the pointer as a TypedObject it doesn't
//               need any more help.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
update_type(ReferenceCount *ptr, TypedObject *typed_ptr) {
  get_global_ptr()->ns_update_type(ptr, typed_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::remove_pointer
//       Access: Public, Static
//  Description: Indicates that the given pointer has been recently
//               freed.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
remove_pointer(ReferenceCount *ptr) {
  get_global_ptr()->ns_remove_pointer(ptr);
}

#endif // __GNUC__

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::operator_new_handler
//       Access: Public, Static
//  Description: This is set up as a global handler function (by
//               redefining a function pointer in Dtool) for the
//               operator new function.  If track-memory-usage is
//               enabled, this function will be called whenever any
//               new operator within the Panda source is invoked.
////////////////////////////////////////////////////////////////////
void *MemoryUsage::
operator_new_handler(size_t size) {
  void *ptr;

  if (_recursion_protect) {
    ptr = default_operator_new(size);

  } else {
    MemoryUsage *mu = get_global_ptr();
    if (mu->_track_memory_usage) {
      ptr = default_operator_new(size);
      get_global_ptr()->ns_record_void_pointer(ptr, size);
      
    } else {
      _is_cpp_operator = true;
      ptr = default_operator_new(size);
      _is_cpp_operator = false;
    }
  }

  return ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::operator_delete_handler
//       Access: Public, Static
//  Description: This is set up as a global handler function (by
//               redefining a function pointer in Dtool) for the
//               operator delete function.  If track-memory-usage is
//               enabled, this function will be called whenever any
//               delete operator within the Panda source is invoked.
////////////////////////////////////////////////////////////////////
void MemoryUsage::
operator_delete_handler(void *ptr) {
  if (_recursion_protect) {
    default_operator_delete(ptr);

  } else {
    MemoryUsage *mu = get_global_ptr();
    if (mu->_track_memory_usage) {
      mu->ns_remove_void_pointer(ptr);
      default_operator_delete(ptr);
    } else {
      _is_cpp_operator = true;
      default_operator_delete(ptr);
      _is_cpp_operator = false;
    }
  }
}

#if defined(WIN32_VC) && defined(_DEBUG)
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
  if (!_recursion_protect) {
    MemoryUsage *mu = get_global_ptr();
    if (mu->_count_memory_usage) {
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

      /*
        This isn't working reliably right now.
      if (_is_cpp_operator) {
        mu->_cpp_size += increment;
      }
      */

#ifdef TRACK_IN_INTERPRETER
      if (in_interpreter) {
        mu->_interpreter_size += increment;
      }
#endif
    }
  }

  return true;
}
#endif  // WIN32_VC && _DEBUG



////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
MemoryUsage::
MemoryUsage() {
  // We must get these variables here instead of in
  // config_express.cxx, because we need to know it at static init
  // time, and who knows when the code in config_express will be
  // executed.

  // track-memory-usage should be true to enable full-force tracking
  // of C++ allocations and recordkeeping by type.  It's quite
  // expensive.
  _track_memory_usage =
    config_express.GetBool("track-memory-usage", false);

  // count-memory-usage is a much lighter-weight version, and only
  // tracks the total memory allocation.  However, it only works for
  // certain build environments (in particular, only in an Opt1 or
  // Opt2 build on Windows).
#if defined(WIN32_VC) && defined(_DEBUG)
  _count_memory_usage = config_express.GetBool("count-memory-usage", 
                                               _track_memory_usage);
#else
  _count_memory_usage = false;
#endif

  if (_track_memory_usage) {
    // Redefine the global pointers for operator new and operator
    // delete (these pointers are defined up in DTOOL) to vector into
    // this class.
    global_operator_new = &operator_new_handler;
    global_operator_delete = &operator_delete_handler;
  }

#if defined(WIN32_VC) && defined(_DEBUG)
  if (_count_memory_usage) {
    global_operator_new = &operator_new_handler;
    global_operator_delete = &operator_delete_handler;
    _CrtSetAllocHook(&win32_malloc_hook);
  }
#endif

  _freeze_index = 0;
  _count = 0;
  _current_cpp_size = 0;
  _cpp_size = 0;
  _interpreter_size = 0;
  _total_size = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_global_ptr
//       Access: Private, Static
//  Description: Returns the pointer to the only MemoryUsage object in
//               the world.
////////////////////////////////////////////////////////////////////
MemoryUsage *MemoryUsage::
get_global_ptr() {
  if (_global_ptr == (MemoryUsage *)NULL) {
    _global_ptr = new MemoryUsage;
  }

  return _global_ptr;
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
      _table.insert(Table::value_type((void *)ptr, MemoryInfo()));
    
    // This shouldn't fail.
    assert(insert_result.first != _table.end());

    if (insert_result.second) {
      _count++;
    }

    MemoryInfo &info = (*insert_result.first).second;

    // We shouldn't already have a ReferenceCount pointer.
    if ((info._flags & MemoryInfo::F_got_ref) != 0) {
      express_cat.error()
        << "ReferenceCount pointer " << (void *)ptr << " recorded twice!\n";
    }

    info._void_ptr = (void *)ptr;
    info._ref_ptr = ptr;
    info._static_type = ReferenceCount::get_class_type();
    info._dynamic_type = ReferenceCount::get_class_type();
    info._time = TrueClock::get_ptr()->get_long_time();
    info._freeze_index = _freeze_index;
    info._flags |= (MemoryInfo::F_reconsider_dynamic_type | MemoryInfo::F_got_ref);

    // We close the recursion_protect flag all the way down here, so
    // that we also protect ourselves against a possible recursive
    // call in TrueClock::get_ptr().
    _recursion_protect = false;
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
      express_cat.error()
        << "Attempt to update type to " << type << " for unrecorded pointer "
        << (void *)ptr << "!\n";
      return;
    }

    MemoryInfo &info = (*ti).second;
    info.update_type_handle(info._static_type, type);
    info.determine_dynamic_type();

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
      express_cat.error()
        << "Attempt to update type to " << typed_ptr->get_type()
        << " for unrecorded pointer "
        << (void *)ptr << "!\n";
      return;
    }

    MemoryInfo &info = (*ti).second;
    info._typed_ptr = typed_ptr;
    info.determine_dynamic_type();

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
      express_cat.error()
        << "Attempt to remove pointer " << (void *)ptr
        << ", not in table.\n"
        << "Possibly a double-destruction.\n";
      nassertv(false);
      return;
    }

    MemoryInfo &info = (*ti).second;

    if ((info._flags & MemoryInfo::F_got_ref) == 0) {
      express_cat.error()
        << "Pointer " << (void *)ptr << " deleted twice!\n";
      return;
    }

    info._flags &= ~MemoryInfo::F_got_ref;

    // Since the pointer has been destructed, we can't safely call its
    // TypedObject virtual methods any more.  Better clear out the
    // typed_ptr for good measure.
    info._typed_ptr = (TypedObject *)NULL;

    if (info._freeze_index == _freeze_index) {
      double now = TrueClock::get_ptr()->get_long_time();

      // We have to protect modifications to the table from recursive
      // calls by toggling _recursion_protect while we adjust it.
      _recursion_protect = true;
      _trend_types.add_info(info.get_type(), info);
      _trend_ages.add_info(now - info._time, info);
      _recursion_protect = false;
    }

    if ((info._flags & (MemoryInfo::F_got_ref | MemoryInfo::F_got_void)) == 0) {
      // If we don't expect to call any more remove_*_pointer on this
      // pointer, remove it from the table.
      if (info._freeze_index == _freeze_index) {
        _count--;
        _current_cpp_size -= info._size;
      }
      _cpp_size -= info._size;

      // We have to protect modifications to the table from recursive
      // calls by toggling _recursion_protect while we adjust it.
      _recursion_protect = true;
      _table.erase(ti);
      _recursion_protect = false;
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
    // We have to protect modifications to the table from recursive
    // calls by toggling _recursion_protect while we adjust it.
    _recursion_protect = true;
    pair<Table::iterator, bool> insert_result =
      _table.insert(Table::value_type((void *)ptr, MemoryInfo()));

    assert(insert_result.first != _table.end());

    if (insert_result.second) {
      _count++;
    }

    MemoryInfo &info = (*insert_result.first).second;

    // We shouldn't already have a void pointer.
    if ((info._flags & MemoryInfo::F_got_void) != 0) {
      express_cat.error()
        << "Void pointer " << (void *)ptr << " recorded twice!\n";
    }

    if (info._freeze_index == _freeze_index) {
      _current_cpp_size += size - info._size;
    } else {
      _current_cpp_size += size;
    }
    _cpp_size += size - info._size;

    info._void_ptr = ptr;
    info._size = size;
    info._time = TrueClock::get_ptr()->get_long_time();
    info._freeze_index = _freeze_index;
    info._flags |= (MemoryInfo::F_got_void | MemoryInfo::F_size_known);

    // We close the recursion_protect flag all the way down here, so
    // that we also protect ourselves against a possible recursive
    // call in TrueClock::get_ptr().
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

    MemoryInfo &info = (*ti).second;

    if ((info._flags & MemoryInfo::F_got_void) == 0) {
      express_cat.error()
        << "Pointer " << (void *)ptr << " deleted twice!\n";
      return;
    }

    if ((info._flags & MemoryInfo::F_got_ref) != 0) {
      express_cat.error()
        << "Pointer " << (void *)ptr << " did not destruct before being deleted!\n";
    }

    info._flags &= ~MemoryInfo::F_got_void;

    if ((info._flags & (MemoryInfo::F_got_ref | MemoryInfo::F_got_void)) == 0) {
      // If we don't expect to call any more remove_*_pointer on this
      // pointer, remove it from the table.

      if (info._freeze_index == _freeze_index) {
        _count--;
        _current_cpp_size -= info._size;
      }
      _cpp_size -= info._size;

      // We have to protect modifications to the table from recursive
      // calls by toggling _recursion_protect while we adjust it.
      _recursion_protect = true;
      _table.erase(ti);
      _recursion_protect = false;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_current_cpp_size
//       Access: Private
//  Description: Returns the total number of bytes of allocated memory
//               via the C++ operators new and delete as counted,
//               not including the memory previously frozen.
////////////////////////////////////////////////////////////////////
size_t MemoryUsage::
ns_get_current_cpp_size() {
  return _current_cpp_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_cpp_size
//       Access: Private
//  Description: Returns the total number of bytes of allocated memory
//               via the C++ operators new and delete as counted,
//               including the memory previously frozen.
////////////////////////////////////////////////////////////////////
size_t MemoryUsage::
ns_get_cpp_size() {
  return _cpp_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_interpreter_size
//       Access: Private
//  Description: Returns the total number of bytes of allocated memory
//               while the high-level languange code is running.  This
//               number is only meaningful if both Panda and the
//               high-level language are single-threaded, and running
//               in the same thread.
////////////////////////////////////////////////////////////////////
size_t MemoryUsage::
ns_get_interpreter_size() {
  return _interpreter_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::ns_get_total_size
//       Access: Private
//  Description: Returns the total size of the dynamic heap, as nearly
//               as can be determined, including all allocated memory
//               if possible, in addition to that tracked by
//               get_cpp_size().
////////////////////////////////////////////////////////////////////
size_t MemoryUsage::
ns_get_total_size() {
#if defined(WIN32_VC) && defined(_DEBUG)
  return _total_size;
#else
  // If we aren't tracking _total_size, report _cpp_size as the next
  // best thing.
  return _cpp_size;
#endif
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

  double now = TrueClock::get_ptr()->get_long_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index &&
        info._ref_ptr != (ReferenceCount *)NULL) {
      result.add_entry(info._ref_ptr, info._typed_ptr, info.get_type(),
                       now - info._time);
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

  double now = TrueClock::get_ptr()->get_long_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index &&
        info._ref_ptr != (ReferenceCount *)NULL) {
      TypeHandle info_type = info.get_type();
      if (info_type != TypeHandle::none() &&
          info_type.is_derived_from(type)) {
        result.add_entry(info._ref_ptr, info._typed_ptr, info_type,
                         now - info._time);
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

  double now = TrueClock::get_ptr()->get_long_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index &&
        info._ref_ptr != (ReferenceCount *)NULL) {
      double age = now - info._time;
      if ((age >= from && age <= to) ||
          (age >= to && age <= from)) {
        result.add_entry(info._ref_ptr, info._typed_ptr, info.get_type(), age);
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

  double now = TrueClock::get_ptr()->get_long_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index && 
        info._ref_ptr != (ReferenceCount *)NULL) {
      if (info._ref_ptr->get_ref_count() == 0) {
        info._ref_ptr->ref();
        result.add_entry(info._ref_ptr, info._typed_ptr, info.get_type(),
                         now - info._time);
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
  // We have to protect modifications to the table from recursive
  // calls by toggling _recursion_protect while we adjust it.
  _recursion_protect = true;

  TypeHistogram hist;
  
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      hist.add_info(info.get_type(), info);
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

  // We have to protect modifications to the table from recursive
  // calls by toggling _recursion_protect while we adjust it.
  _recursion_protect = true;

  AgeHistogram hist;
  double now = TrueClock::get_ptr()->get_long_time();

  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      hist.add_info(now - info._time, info);
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
//               for this pointer, checks to see if it has possible
//               been recorded under the TypedObject pointer (this
//               will happen when the class inherits from TypedObject
//               before ReferenceCount, e.g. TypedReferenceCount).
////////////////////////////////////////////////////////////////////
void MemoryUsage::
consolidate_void_ptr(MemoryInfo &info) {
  if (info.is_size_known()) {
    // We already know the size, so no sweat.
    return;
  }

  if (info.get_typed_ptr() == (TypedObject *)NULL) {
    // We don't have a typed pointer for this thing yet.
    return;
  }

  void *typed_ptr = (void *)info.get_typed_ptr();

  if (typed_ptr == (void *)info.get_ref_ptr()) {
    // The TypedObject pointer is the same pointer as the
    // ReferenceCount pointer, so there's no point in looking it up
    // separately.  Actually, this really shouldn't even be possible.
    return;
  }

  Table::iterator ti;
  ti = _table.find(typed_ptr);
  if (ti == _table.end()) {
    // No entry for the typed pointer, either.
    return;
  }

  // We do have an entry!  Copy over the relevant pieces.
  MemoryInfo &typed_info = (*ti).second;

  if (typed_info.is_size_known()) {
    info._size = typed_info.get_size();
    info._flags |= MemoryInfo::F_size_known;
    if (typed_info._freeze_index == _freeze_index) {
      _current_cpp_size += info._size;
    }
  }

  // The typed_ptr is clearly the more accurate pointer to the
  // beginning of the structure.
  info._void_ptr = typed_ptr;

  // Now that we've consolidated the pointers, remove the void pointer
  // entry.
  if (info._freeze_index == _freeze_index) {
    _count--;
    _current_cpp_size -= info._size;
  }
    
  // We have to protect modifications to the table from recursive
  // calls by toggling _recursion_protect while we adjust it.
  _recursion_protect = true;
  _table.erase(ti);
  _recursion_protect = false;
}


#endif  // DO_MEMORY_USAGE
