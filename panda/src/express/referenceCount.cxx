/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file referenceCount.cxx
 * @author drose
 * @date 1998-10-23
 */

#include "referenceCount.h"
#include "atomicAdjust.h"
#include "mutexImpl.h"

TypeHandle ReferenceCount::_type_handle;

/**
 * Does some easy checks to make sure that the reference count isn't
 * completely bogus.  Returns true if ok, false otherwise.
 */
bool ReferenceCount::
do_test_ref_count_integrity() const {
  // If this assertion fails, we're trying to delete an object that was just
  // deleted.  Possibly you used a real pointer instead of a PointerTo at some
  // point, and the object was deleted when the PointerTo went out of scope.
  // Maybe you tried to create an automatic (local variable) instance of a
  // class that derives from ReferenceCount.  Or maybe your headers are out of
  // sync, and you need to make clean in direct or some higher tree.
  nassertr(_ref_count != deleted_ref_count, false);

  // If this assertion fails, the reference counts are all screwed up
  // altogether.  Maybe some errant code stomped all over memory somewhere.
  nassertr(_ref_count >= 0, false);

  return true;
}

/**
 * Returns true if the reference count is nonzero, false otherwise.
 */
bool ReferenceCount::
do_test_ref_count_nonzero() const {
  nassertr(do_test_ref_count_integrity(), false);
  nassertr(_ref_count > 0, false);

  return true;
}

/**
 * Allocates a new WeakReferenceList structure and stores it on the object.
 */
void ReferenceCount::
create_weak_list() {
  WeakReferenceList *weak_list = new WeakReferenceList;
  void *orig =
    AtomicAdjust::compare_and_exchange_ptr(_weak_list, nullptr, weak_list);
  if (orig != nullptr) {
    // Someone else created it first.
    delete weak_list;
  }
}
