// Filename: memoryUsage.cxx
// Created by:  drose (25May00)
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

#include "memoryUsage.h"
#include "memoryUsagePointers.h"
#include "trueClock.h"
#include "typedReferenceCount.h"

#ifndef NDEBUG
// Nothing in this module gets compiled in NDEBUG mode.

#include "config_express.h"

#include <algorithm>

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
//     Function: MemoryUsage::MemoryInfo::get_type
//       Access: Public
//  Description: Returns the best known type, dynamic or static, of
//               the pointer.
////////////////////////////////////////////////////////////////////
TypeHandle MemoryUsage::MemoryInfo::
get_type() {
  // If we don't want to consider the dynamic type any further, use
  // what we've got.
  if (!_reconsider_dynamic_type) {
    if (_dynamic_type == TypeHandle::none()) {
      return _static_type;
    }
    return _dynamic_type;
  }

  // Otherwise, examine the pointer again and make sure it's still the
  // best information we have.  We have to do this each time because
  // if we happen to be examining the pointer from within the
  // constructor or destructor, its dynamic type will appear to be
  // less-specific than it actually is, so our idea of what type this
  // thing is could change from time to time.
  determine_dynamic_type();

  // Now return the more specific of the two.
  TypeHandle type = _static_type;
  update_type_handle(type, _dynamic_type);

  return type;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::MemoryInfo::determine_dynamic_type
//       Access: Public
//  Description: Tries to determine the actual type of the object to
//               which this thing is pointed, if possible.
////////////////////////////////////////////////////////////////////
void MemoryUsage::MemoryInfo::
determine_dynamic_type() {
  if (_reconsider_dynamic_type && _static_type != TypeHandle::none()) {
    // See if we know enough now to infer the dynamic type from the
    // pointer.

    if (_typed_ptr == (TypedObject *)NULL) {
      // If our static type is known to inherit from
      // TypedReferenceCount, then we can directly downcast to get the
      // TypedObject pointer.
      if (_static_type.is_derived_from(TypedReferenceCount::get_class_type())) {
        _typed_ptr = (TypedReferenceCount *)_ptr;
      }
    }

    if (_typed_ptr != (TypedObject *)NULL) {
      // If we have a TypedObject pointer, we can determine the type.
      // This might still not return the exact type, particularly if
      // we are being called within the destructor or constructor of
      // this object.
      TypeHandle got_type = _typed_ptr->get_type();

      if (got_type == TypeHandle::none()) {
        express_cat.warning()
          << "Found an unregistered type in a " << _static_type
          << " pointer:\n"
          << "Check derived types of " << _static_type
          << " and make sure that all are being initialized.\n";
        _dynamic_type = _static_type;
        _reconsider_dynamic_type = false;
        return;
      }

      update_type_handle(_dynamic_type, got_type);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::MemoryInfo::update_type_handle
//       Access: Public
//  Description: Updates the given destination TypeHandle with the
//               refined TypeHandle, if it is in fact more specific
//               than the original value for the destination.
////////////////////////////////////////////////////////////////////
void MemoryUsage::MemoryInfo::
update_type_handle(TypeHandle &destination, TypeHandle refined) {
  if (refined == TypeHandle::none()) {
    express_cat.error()
      << "Attempt to update type of " << (void *)_ptr
      << "(type is " << get_type()
      << ") to an undefined type!\n";

  } else if (destination == refined) {
    // Updating with the same type, no problem.

  } else if (destination.is_derived_from(refined)) {
    // Updating with a less-specific type, no problem.

  } else if (refined.is_derived_from(destination)) {
    // Updating with a more-specific type, no problem.
    destination = refined;

  } else {
    express_cat.error()
      << "Pointer " << (void *)_ptr << " previously indicated as type "
      << destination << " is now type " << refined << "!\n";
    destination = refined;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::TypeHistogram::add_info
//       Access: Public
//  Description: Adds a single entry to the histogram.
////////////////////////////////////////////////////////////////////
void MemoryUsage::TypeHistogram::
add_info(TypeHandle type) {
  _counts[type]++;
}


// This class is a temporary class used only in TypeHistogram::show(),
// below, to sort the types in descending order by counts.
class TypeHistogramCountSorter {
public:
  TypeHistogramCountSorter(int count, TypeHandle type) {
    _count = count;
    _type = type;
  }
  bool operator < (const TypeHistogramCountSorter &other) const {
    return _count > other._count;
  }
  int _count;
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
  // by counts.
  vector<TypeHistogramCountSorter> count_sorter;
  Counts::const_iterator ci;
  for (ci = _counts.begin(); ci != _counts.end(); ++ci) {
    count_sorter.push_back
      (TypeHistogramCountSorter((*ci).second, (*ci).first));
  }

  sort(count_sorter.begin(), count_sorter.end());

  vector<TypeHistogramCountSorter>::const_iterator vi;
  for (vi = count_sorter.begin(); vi != count_sorter.end(); ++vi) {
    nout << (*vi)._type << " : " << (*vi)._count << " pointers.\n";
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
add_info(double age) {
  int bucket = choose_bucket(age);
  nassertv(bucket >= 0 && bucket < num_buckets);
  _counts[bucket]++;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::show
//       Access: Public
//  Description: Shows the contents of the histogram to nout.
////////////////////////////////////////////////////////////////////
void MemoryUsage::AgeHistogram::
show() const {
  for (int i = 0; i < num_buckets - 1; i++) {
    nout << _cutoff[i] << " to " << _cutoff[i + 1] << " seconds old : "
         << _counts[i] << " pointers.\n";
  }
  nout << _cutoff[num_buckets - 1] << " seconds old and up : "
       << _counts[num_buckets - 1] << " pointers.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::AgeHistogram::clear
//       Access: Public
//  Description: Resets the histogram in preparation for new data.
////////////////////////////////////////////////////////////////////
void MemoryUsage::AgeHistogram::
clear() {
  for (int i = 0; i < num_buckets; i++) {
    _counts[i] = 0;
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

#if defined(__GNUC__) && !defined(NDEBUG)

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

#endif // __GNUC__ && !NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
MemoryUsage::
MemoryUsage() {
  // We must get this here instead of in config_express.cxx, because we
  // need to know it at static init time, and who knows when the code
  // in config_express will be executed.
  _track_memory_usage =
    config_express.GetBool("track-memory-usage", false);

  _freeze_index = 0;
  _count = 0;
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
    MemoryInfo info;
    info._ptr = ptr;
    info._typed_ptr = (TypedObject *)NULL;
    info._static_type = ReferenceCount::get_class_type();
    info._dynamic_type = ReferenceCount::get_class_type();
    info._time = TrueClock::get_ptr()->get_real_time();
    info._freeze_index = _freeze_index;
    info._reconsider_dynamic_type = true;

    Table::iterator ti;
    ti = _table.find(ptr);
    if (ti != _table.end()) {
      express_cat.error() << "Pointer " << (void *)ptr << " recorded twice!\n";
      (*ti).second = info;
    } else {
      _table[ptr] = info;
      _count++;
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
      express_cat.error()
        << "Attempt to update type to " << type << " for unrecorded pointer "
        << (void *)ptr << "!\n";
      return;
    }

    MemoryInfo &info = (*ti).second;
    info.update_type_handle(info._static_type, type);
    info.determine_dynamic_type();
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

    // Since the pointer has been destructed, we can't safely call its
    // TypedObject virtual methods any more.  Better clear out the
    // typed_ptr for good measure.
    info._typed_ptr = (TypedObject *)NULL;

    if (info._freeze_index == _freeze_index) {
      double now = TrueClock::get_ptr()->get_real_time();
      _count--;
      _trend_types.add_info(info.get_type());
      _trend_ages.add_info(now - info._time);
    }

    _table.erase(ti);
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

  double now = TrueClock::get_ptr()->get_real_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      result.add_entry(info._ptr, info._typed_ptr, info.get_type(),
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

  double now = TrueClock::get_ptr()->get_real_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      TypeHandle info_type = info.get_type();
      if (info_type != TypeHandle::none() &&
          info_type.is_derived_from(type)) {
        result.add_entry(info._ptr, info._typed_ptr, info_type,
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

  double now = TrueClock::get_ptr()->get_real_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      double age = now - info._time;
      if ((age >= from && age <= to) ||
          (age >= to && age <= from)) {
        result.add_entry(info._ptr, info._typed_ptr, info.get_type(), age);
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

  double now = TrueClock::get_ptr()->get_real_time();
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      if ((*ti).first->get_ref_count() == 0) {
        (*ti).first->ref();
        result.add_entry(info._ptr, info._typed_ptr, info.get_type(),
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
  TypeHistogram hist;

  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      hist.add_info(info.get_type());
    }
  }

  hist.show();
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
  AgeHistogram hist;
  double now = TrueClock::get_ptr()->get_real_time();

  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    MemoryInfo &info = (*ti).second;
    if (info._freeze_index == _freeze_index) {
      hist.add_info(now - info._time);
    }
  }

  hist.show();
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



#endif  // NDEBUG
