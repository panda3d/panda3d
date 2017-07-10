/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsage.I
 * @author drose
 * @date 2000-05-25
 */

/**
 * Returns true if the user has Configured the variable 'track-memory-usage'
 * to true, indicating that this class will be in effect.  If this returns
 * false, the user has indicated not to do any of this.
 */
ALWAYS_INLINE bool MemoryUsage::
get_track_memory_usage() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->_track_memory_usage;
#else
  return false;
#endif
}

/**
 * Indicates that the given pointer has been recently allocated.
 */
INLINE void MemoryUsage::
record_pointer(ReferenceCount *ptr) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_record_pointer(ptr);
#endif
}

/**
 * Indicates that the given pointer has been recently allocated.
 */
INLINE void MemoryUsage::
record_pointer(void *ptr, TypeHandle type) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_record_pointer(ptr, type);
#endif
}

/**
 * Associates the indicated type with the given pointer.  This should be
 * called by functions (e.g.  the constructor) that know more specifically
 * what type of thing we've got; otherwise, the MemoryUsage database will know
 * only that it's a "ReferenceCount".
 */
INLINE void MemoryUsage::
update_type(ReferenceCount *ptr, TypeHandle type) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_update_type((void *)ptr, type);
#endif
}

/**
 * Associates the indicated type with the given pointer.  This flavor of
 * update_type() also passes in the pointer as a TypedObject, and useful for
 * objects that are, in fact, TypedObjects.  Once the MemoryUsage database has
 * the pointer as a TypedObject it doesn't need any more help.
 */
INLINE void MemoryUsage::
update_type(ReferenceCount *ptr, TypedObject *typed_ptr) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_update_type((void *)ptr, typed_ptr);
#endif
}

/**
 * Associates the indicated type with the given pointer.  This should be
 * called by functions (e.g.  the constructor) that know more specifically
 * what type of thing we've got.
 */
INLINE void MemoryUsage::
update_type(void *ptr, TypeHandle type) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_update_type(ptr, type);
#endif
}

/**
 * Indicates that the given pointer has been recently freed.
 */
INLINE void MemoryUsage::
remove_pointer(ReferenceCount *ptr) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_remove_pointer(ptr);
#endif
}

/**
 * Returns true if the MemoryUsage object is currently tracking memory (e.g.
 * track-memory-usage is configured #t).
 */
INLINE bool MemoryUsage::
is_tracking() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->_track_memory_usage;
#else
  return false;
#endif
}

/**
 * Returns true if the MemoryUsage object is currently at least counting
 * memory (e.g.  this is a Windows debug build), even if it's not fully
 * tracking it.
 */
INLINE bool MemoryUsage::
is_counting() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->_count_memory_usage;
#else
  return false;
#endif
}

/**
 * Returns the total number of bytes of allocated memory consumed by C++
 * objects, not including the memory previously frozen.
 */
INLINE size_t MemoryUsage::
get_current_cpp_size() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->_current_cpp_size;
#else
  return 0;
#endif
}

/**
 * Returns the total number of bytes of allocated memory consumed by C++
 * objects, including the memory previously frozen.
 */
INLINE size_t MemoryUsage::
get_total_cpp_size() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->_total_cpp_size;
#else
  return 0;
#endif
}

/**
 * Returns the total number of bytes allocated from the heap from code within
 * Panda, for individual objects.
 */
INLINE size_t MemoryUsage::
get_panda_heap_single_size() {
#ifdef DO_MEMORY_USAGE
  return (size_t)AtomicAdjust::get(get_global_ptr()->_total_heap_single_size);
#else
  return 0;
#endif
}

/**
 * Returns the total number of bytes allocated from the heap from code within
 * Panda, for arrays.
 */
INLINE size_t MemoryUsage::
get_panda_heap_array_size() {
#ifdef DO_MEMORY_USAGE
  return (size_t)AtomicAdjust::get(get_global_ptr()->_total_heap_array_size);
#else
  return 0;
#endif
}

/**
 * Returns the extra bytes allocated from the system that are not immediately
 * used for holding allocated objects.  This can only be determined if
 * ALTERNATIVE_MALLOC is enabled.
 */
INLINE size_t MemoryUsage::
get_panda_heap_overhead() {
#if defined(DO_MEMORY_USAGE) && (defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2))
  MemoryUsage *mu = get_global_ptr();
  return (size_t)(AtomicAdjust::get(mu->_requested_heap_size) - AtomicAdjust::get(mu->_total_heap_single_size) - AtomicAdjust::get(mu->_total_heap_array_size));
#else
  return 0;
#endif
}

/**
 * Returns the total number of bytes allocated from the virtual memory pool
 * from code within Panda.
 */
INLINE size_t MemoryUsage::
get_panda_mmap_size() {
#ifdef DO_MEMORY_USAGE
  return (size_t)AtomicAdjust::get(get_global_ptr()->_total_mmap_size);
#else
  return 0;
#endif
}

/**
 * Returns the total number of bytes of allocated memory in the heap that
 * Panda didn't seem to be responsible for.  This includes a few bytes for
 * very low-level objects (like ConfigVariables) that cannot use Panda memory
 * tracking because they are so very low-level.
 *
 * This also includes all of the memory that might have been allocated by a
 * high-level interpreter, like Python.
 *
 * This number is only available if Panda is able to hook into the actual heap
 * callback.
 */
INLINE size_t MemoryUsage::
get_external_size() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage *mu = get_global_ptr();
  if (mu->_count_memory_usage) {
    // We can only possibly know this with memory counting, which tracks every
    // malloc call.

#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
    // With alternative malloc, none of the Panda allocated memory shows up in
    // total_size, so anything there is external.
    return mu->_total_size;
#else
    // Without alternative malloc, the Panda allocated memory is also included
    // in total_size, so we have to subtract it out.
    return mu->_total_size - (size_t)mu->_total_heap_single_size - (size_t)mu->_total_heap_array_size;
#endif
  } else {
    return 0;
  }
#else
  return 0;
#endif
}

/**
 * Returns the total size of allocated memory consumed by the process, as
 * nearly as can be determined.
 */
INLINE size_t MemoryUsage::
get_total_size() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage *mu = get_global_ptr();
  if (mu->_count_memory_usage) {
    return mu->_total_size + (size_t)mu->_requested_heap_size;
  } else {
#if defined(USE_MEMORY_DLMALLOC) || defined(USE_MEMORY_PTMALLOC2)
    return (size_t)mu->_requested_heap_size;
#else
    return (size_t)(AtomicAdjust::get(mu->_total_heap_single_size) + AtomicAdjust::get(mu->_total_heap_array_size));
#endif
  }
#else
  return 0;
#endif
}

/**
 * Returns the number of pointers currently active.
 */
INLINE int MemoryUsage::
get_num_pointers() {
#ifdef DO_MEMORY_USAGE
  return get_global_ptr()->ns_get_num_pointers();
#else
  return 0;
#endif
}

/**
 * Fills the indicated MemoryUsagePointers with the set of all pointers
 * currently active.
 */
INLINE void MemoryUsage::
get_pointers(MemoryUsagePointers &result) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_get_pointers(result);
#endif
}

/**
 * Fills the indicated MemoryUsagePointers with the set of all pointers of the
 * indicated type currently active.
 */
INLINE void MemoryUsage::
get_pointers_of_type(MemoryUsagePointers &result, TypeHandle type) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_get_pointers_of_type(result, type);
#endif
}

/**
 * Fills the indicated MemoryUsagePointers with the set of all pointers that
 * were allocated within the range of the indicated number of seconds ago.
 */
INLINE void MemoryUsage::
get_pointers_of_age(MemoryUsagePointers &result, double from, double to) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_get_pointers_of_age(result, from, to);
#endif
}

/**
 * Fills the indicated MemoryUsagePointers with the set of all currently
 * active pointers (that is, pointers allocated since the last call to
 * freeze(), and not yet freed) that have a zero reference count.
 *
 * Generally, an undeleted pointer with a zero reference count means its
 * reference count has never been incremented beyond zero (since once it has
 * been incremented, the only way it can return to zero would free the
 * pointer).  This may include objects that are allocated statically or on the
 * stack, which are never intended to be deleted.  Or, it might represent a
 * programmer or compiler error.
 *
 * This function has the side-effect of incrementing each of their reference
 * counts by one, thus preventing them from ever being freed--but since they
 * hadn't been freed anyway, probably no additional harm is done.
 */
INLINE void MemoryUsage::
get_pointers_with_zero_count(MemoryUsagePointers &result) {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_get_pointers_with_zero_count(result);
#endif
}

/**
 * 'Freezes' all pointers currently stored so that they are no longer
 * reported; only newly allocate pointers from this point on will appear in
 * future information requests.  This makes it easier to differentiate between
 * continuous leaks and one-time memory allocations.
 */
INLINE void MemoryUsage::
freeze() {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_freeze();
#endif
}

/**
 * Shows the breakdown of types of all of the active pointers.
 */
INLINE void MemoryUsage::
show_current_types() {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_show_current_types();
#endif
}

/**
 * Shows the breakdown of types of all of the pointers allocated and freed
 * since the last call to freeze().
 */
INLINE void MemoryUsage::
show_trend_types() {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_show_trend_types();
#endif
}

/**
 * Shows the breakdown of ages of all of the active pointers.
 */
INLINE void MemoryUsage::
show_current_ages() {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_show_current_ages();
#endif
}

/**
 * Shows the breakdown of ages of all of the pointers allocated and freed
 * since the last call to freeze().
 */
INLINE void MemoryUsage::
show_trend_ages() {
#ifdef DO_MEMORY_USAGE
  get_global_ptr()->ns_show_trend_ages();
#endif
}

/**
 * Returns the pointer to the only MemoryUsage object in the world.
 */
INLINE MemoryUsage *MemoryUsage::
get_global_ptr() {
#ifdef DO_MEMORY_USAGE
#ifdef __GNUC__
  // Tell the compiler that this is an unlikely branch.
  if (__builtin_expect(_global_ptr == nullptr, 0)) {
#else
  if (_global_ptr == nullptr) {
#endif
    init_memory_usage();
  }

  return _global_ptr;
#else
  return nullptr;
#endif
}
