// Filename: referenceCount.cxx
// Created by:  drose (23Oct98)
// 
////////////////////////////////////////////////////////////////////

#include "referenceCount.h"

TypeHandle ReferenceCount::_type_handle;

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: ReferenceCount::unref_consider_delete
//       Access: Public
//  Description: This function is called by the global unref_delete()
//               function.  This function unrefs the pointer and
//               returns true if it should be deleted (i.e. the count
//               has gone to zero), or false otherwise.  It doesn't
//               delete itself because (a) a method cannot safely
//               delete this, and (b) we don't have a virtual
//               destructor anyway.  The decision of whether to delete
//               is left up to unref_delete().
////////////////////////////////////////////////////////////////////
bool ReferenceCount::
unref_consider_delete() {
  unref();

  if (get_leak_memory()) {
    // In leak-memory mode, we don't actually delete anything.
    // However, we do want to call prepare_delete() when the count
    // reaches zero, to reset the refcount to -100 as a deleted flag.
    if (get_ref_count() == 0) {
      prepare_delete();
      MemoryUsage::remove_pointer(this);
    }
    return false;
  }

  return (get_ref_count() == 0);
}
#endif
