// Filename: memoryUsage.I
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


////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::track_memory_usage
//       Access: Public, Static
//  Description: Returns true if the user has Configured the variable
//               'track-memory-usage' to true, indicating that this
//               class will be in effect.  If this returns false, the
//               user has indicated not to do any of this.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
get_track_memory_usage() {
  return get_global_ptr()->_track_memory_usage;
}

#ifndef __GNUC__

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::record_pointer
//       Access: Public, Static
//  Description: Indicates that the given pointer has been recently
//               allocated.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
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
INLINE void MemoryUsage::
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
INLINE void MemoryUsage::
update_type(ReferenceCount *ptr, TypedObject *typed_ptr) {
  get_global_ptr()->ns_update_type(ptr, typed_ptr);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::remove_pointer
//       Access: Public, Static
//  Description: Indicates that the given pointer has been recently
//               freed.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
remove_pointer(ReferenceCount *ptr) {
  get_global_ptr()->ns_remove_pointer(ptr);
}

#endif  // __GNUC__

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::is_tracking
//       Access: Public, Static
//  Description: Returns true if the MemoryUsage object is currently
//               tracking memory (e.g. track-memory-usage is
//               configured #t).
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
is_tracking() {
  return get_global_ptr()->_track_memory_usage;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::is_counting
//       Access: Public, Static
//  Description: Returns true if the MemoryUsage object is currently
//               at least counting memory (e.g. count-memory-usage is
//               configured #t), even if it's not fully tracking it.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
is_counting() {
  return get_global_ptr()->_count_memory_usage;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_current_cpp_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               via the C++ operators new and delete as counted,
//               not including the memory previously frozen.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_current_cpp_size() {
  return get_global_ptr()->ns_get_current_cpp_size();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::has_cpp_size
//       Access: Public, Static
//  Description: Returns true if the value returned by
//               get_cpp_size() is meaningful on this particular
//               system with this particular configuration, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
has_cpp_size() {
#if defined(WIN32_VC) && defined(_DEBUG)
  // Windows in debug mode can count C++ size without having to do a
  // full track.
  return is_counting();
#else
  // Other systems require the full hammer.
  return is_tracking();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_cpp_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               via the C++ operators new and delete as counted,
//               including the memory previously frozen.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_cpp_size() {
  return get_global_ptr()->ns_get_cpp_size();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::has_interpreter_size
//       Access: Public, Static
//  Description: Returns true if the value returned by
//               get_interpreter_size() is meaningful on this particular
//               system with this particular configuration, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
has_interpreter_size() {
#ifdef TRACK_IN_INTERPRETER
  return is_counting();
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_interpreter_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               while the high-level languange code is running.  This
//               number is only meaningful if both Panda and the
//               high-level language are single-threaded, and running
//               in the same thread.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_interpreter_size() {
  return get_global_ptr()->ns_get_interpreter_size();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::has_total_size
//       Access: Public, Static
//  Description: Returns true if the value returned by
//               get_total_size() is meaningful on this particular
//               system with this particular configuration, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
has_total_size() {
#if defined(WIN32_VC) && defined(_DEBUG)
  // Windows in debug mode can count total size without having to do a
  // full track.
  return is_counting();
#else
  // Other systems require the full hammer.
  return is_tracking();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_total_size
//       Access: Public, Static
//  Description: Returns the total size of the dynamic heap, as nearly
//               as can be determined, including all allocated memory
//               if possible, in addition to that tracked by
//               get_cpp_size().
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_total_size() {
  return get_global_ptr()->ns_get_total_size();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_num_pointers
//       Access: Public, Static
//  Description: Returns the number of pointers currently active.
////////////////////////////////////////////////////////////////////
INLINE int MemoryUsage::
get_num_pointers() {
  return get_global_ptr()->ns_get_num_pointers();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_pointers
//       Access: Public, Static
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers currently active.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
get_pointers(MemoryUsagePointers &result) {
  get_global_ptr()->ns_get_pointers(result);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_pointers_of_type
//       Access: Public, Static
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers of the indicated type currently
//               active.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
get_pointers_of_type(MemoryUsagePointers &result, TypeHandle type) {
  get_global_ptr()->ns_get_pointers_of_type(result, type);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_pointers_of_age
//       Access: Public, Static
//  Description: Fills the indicated MemoryUsagePointers with the set
//               of all pointers that were allocated within the range
//               of the indicated number of seconds ago.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
get_pointers_of_age(MemoryUsagePointers &result, double from, double to) {
  get_global_ptr()->ns_get_pointers_of_age(result, from, to);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_pointers_with_zero_count
//       Access: Public, Static
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
INLINE void MemoryUsage::
get_pointers_with_zero_count(MemoryUsagePointers &result) {
  get_global_ptr()->ns_get_pointers_with_zero_count(result);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::freeze
//       Access: Public, Static
//  Description: 'Freezes' all pointers currently stored so that they
//               are no longer reported; only newly allocate pointers
//               from this point on will appear in future information
//               requests.  This makes it easier to differentiate
//               between continuous leaks and one-time memory
//               allocations.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
freeze() {
  get_global_ptr()->ns_freeze();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::show_current_types
//       Access: Public, Static
//  Description: Shows the breakdown of types of all of the
//               active pointers.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
show_current_types() {
  get_global_ptr()->ns_show_current_types();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::show_trend_types
//       Access: Public, Static
//  Description: Shows the breakdown of types of all of the
//               pointers allocated and freed since the last call to
//               freeze().
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
show_trend_types() {
  get_global_ptr()->ns_show_trend_types();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::show_current_ages
//       Access: Public, Static
//  Description: Shows the breakdown of ages of all of the
//               active pointers.
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
show_current_ages() {
  get_global_ptr()->ns_show_current_ages();
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::show_trend_ages
//       Access: Public, Static
//  Description: Shows the breakdown of ages of all of the
//               pointers allocated and freed since the last call to
//               freeze().
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
show_trend_ages() {
  get_global_ptr()->ns_show_trend_ages();
}
