/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferResidencyTracker.I
 * @author drose
 * @date 2006-03-16
 */

/**
 *
 */
INLINE BufferContextChain &BufferResidencyTracker::
get_inactive_nonresident() {
  return _chains[S_inactive_nonresident];
}

/**
 *
 */
INLINE BufferContextChain &BufferResidencyTracker::
get_active_nonresident() {
  return _chains[S_active_nonresident];
}

/**
 *
 */
INLINE BufferContextChain &BufferResidencyTracker::
get_inactive_resident() {
  return _chains[S_inactive_resident];
}

/**
 *
 */
INLINE BufferContextChain &BufferResidencyTracker::
get_active_resident() {
  return _chains[S_active_resident];
}
