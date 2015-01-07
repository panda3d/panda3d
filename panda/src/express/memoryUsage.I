// Filename: memoryUsage.I
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
//               at least counting memory (e.g. this is a Windows
//               debug build), even if it's not fully tracking it.
////////////////////////////////////////////////////////////////////
INLINE bool MemoryUsage::
is_counting() {
  return get_global_ptr()->_count_memory_usage;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_current_cpp_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               consumed by C++ objects, not including the memory
//               previously frozen.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_current_cpp_size() {
  return get_global_ptr()->_current_cpp_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_total_cpp_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               consumed by C++ objects, including the memory
//               previously frozen.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_total_cpp_size() {
  return get_global_ptr()->_total_cpp_size;
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_panda_heap_single_size
//       Access: Public, Static
//  Description: Returns the total number of bytes allocated from the
//               heap from code within Panda, for individual objects.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_panda_heap_single_size() {
  return AtomicAdjust::get(get_global_ptr()->_total_heap_single_size);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_panda_heap_array_size
//       Access: Public, Static
//  Description: Returns the total number of bytes allocated from the
//               heap from code within Panda, for arrays.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_panda_heap_array_size() {
  return AtomicAdjust::get(get_global_ptr()->_total_heap_array_size);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_panda_heap_overhead
//       Access: Public, Static
//  Description: Returns the extra bytes allocated from the system
//               that are not immediately used for holding allocated
//               objects.  This can only be determined if
//               ALTERNATIVE_MALLOC is enabled.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_panda_heap_overhead() {
#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
  MemoryUsage *mu = get_global_ptr();
  return AtomicAdjust::get(mu->_requested_heap_size) - AtomicAdjust::get(mu->_total_heap_single_size) - AtomicAdjust::get(mu->_total_heap_array_size);
#else
  return 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_panda_mmap_size
//       Access: Public, Static
//  Description: Returns the total number of bytes allocated from the
//               virtual memory pool from code within Panda.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_panda_mmap_size() {
  return AtomicAdjust::get(get_global_ptr()->_total_mmap_size);
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_external_size
//       Access: Public, Static
//  Description: Returns the total number of bytes of allocated memory
//               in the heap that Panda didn't seem to be responsible
//               for.  This includes a few bytes for very low-level
//               objects (like ConfigVariables) that cannot use Panda
//               memory tracking because they are so very low-level.
//
//               This also includes all of the memory that might have
//               been allocated by a high-level interpreter, like
//               Python.
//
//               This number is only available if Panda is able to
//               hook into the actual heap callback.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_external_size() {
  MemoryUsage *mu = get_global_ptr();
  if (mu->_count_memory_usage) {
    // We can only possibly know this with memory counting, which
    // tracks every malloc call.

#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
    // With alternative malloc, none of the Panda allocated memory
    // shows up in total_size, so anything there is external.
    return mu->_total_size;
#else
    // Without alternative malloc, the Panda allocated memory is also
    // included in total_size, so we have to subtract it out.
    return mu->_total_size - mu->_total_heap_single_size - mu->_total_heap_array_size;
#endif
  } else {
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_total_size
//       Access: Public, Static
//  Description: Returns the total size of allocated memory consumed
//               by the process, as nearly as can be determined.
////////////////////////////////////////////////////////////////////
INLINE size_t MemoryUsage::
get_total_size() {
  MemoryUsage *mu = get_global_ptr();
  if (mu->_count_memory_usage) {
    return mu->_total_size + mu->_requested_heap_size;
  } else {
#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
    return mu->_requested_heap_size;
#else
    return AtomicAdjust::get(mu->_total_heap_single_size) + AtomicAdjust::get(mu->_total_heap_array_size);
#endif
  }
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

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsage::get_global_ptr
//       Access: Private, Static
//  Description: Returns the pointer to the only MemoryUsage object in
//               the world.
////////////////////////////////////////////////////////////////////
INLINE MemoryUsage *MemoryUsage::
get_global_ptr() {
  if (_global_ptr == (MemoryUsage *)NULL) {
    init_memory_hook();
    _global_ptr = new MemoryUsage(*memory_hook);
    memory_hook = _global_ptr;
  }

  return _global_ptr;
}
