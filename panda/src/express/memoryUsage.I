// Filename: memoryUsage.I
// Created by:  drose (25May00)
// 
////////////////////////////////////////////////////////////////////

#ifdef NDEBUG

// In NDEBUG mode, these are all defined to do nothing.

INLINE bool MemoryUsage::
get_track_memory_usage() {
  return false;
}

INLINE void MemoryUsage::
record_pointer(ReferenceCount *) {
}

INLINE void MemoryUsage::
update_type(ReferenceCount *, TypeHandle) {
}

INLINE void MemoryUsage::
remove_pointer(ReferenceCount *) {
}

#else  // NDEBUG

// In the normal compilation mode, most of these functions vector into
// their corresponding ns_* non-static variants.


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
////////////////////////////////////////////////////////////////////
INLINE void MemoryUsage::
update_type(ReferenceCount *ptr, TypeHandle type) {
  get_global_ptr()->ns_update_type(ptr, type);
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

#endif // NDEBUG
