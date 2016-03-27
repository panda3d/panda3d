/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundleHandle.I
 * @author drose
 * @date 2007-10-01
 */

/**
 *
 */
INLINE PartBundleHandle::
PartBundleHandle(PartBundle *bundle) :
  _bundle(bundle)
{
}

/**
 *
 */
INLINE PartBundleHandle::
~PartBundleHandle() {
}

/**
 * Returns the actual PartBundle embedded within the handle.
 */
INLINE PartBundle *PartBundleHandle::
get_bundle() {
  return _bundle;
}

/**
 * Changes the actual PartBundle embedded within the handle.
 */
INLINE void PartBundleHandle::
set_bundle(PartBundle *bundle) {
  _bundle = bundle;
}
