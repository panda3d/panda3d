// Filename: cachedTypedWritableReferenceCount.cxx
// Created by:  drose (25Jan05)
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

#include "cachedTypedWritableReferenceCount.h"

TypeHandle CachedTypedWritableReferenceCount::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CachedTypedWritableReferenceCount::do_test_ref_count_integrity
//       Access: Protected
//  Description: Does some easy checks to make sure that the reference
//               count isn't completely bogus.
////////////////////////////////////////////////////////////////////
bool CachedTypedWritableReferenceCount::
do_test_ref_count_integrity() const {
  nassertr(this != NULL, false);

  // If this assertion fails, we're trying to delete an object that
  // was just deleted.  Possibly you used a real pointer instead of a
  // PointerTo at some point, and the object was deleted when the
  // PointerTo went out of scope.  Maybe you tried to create an
  // automatic (local variable) instance of a class that derives from
  // ReferenceCount.  Or maybe your headers are out of sync, and you
  // need to make clean in direct or some higher tree.
  nassertr(_cache_ref_count != -100, false);

  // If this assertion fails, the reference counts are all screwed
  // up altogether.  Maybe some errant code stomped all over memory
  // somewhere.
  nassertr(_cache_ref_count >= 0, false);

  return TypedWritableReferenceCount::do_test_ref_count_integrity();
}
