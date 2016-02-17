/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globalMilesManager.I
 * @author drose
 * @date 2007-07-30
 */

/**
 * Returns true if the Miles Sound System is open (and active), false if it is
 * not.
 */
INLINE bool GlobalMilesManager::
is_open() const {
  return _is_open;
}

/**
 * Returns the number of sample handles that have been allocated.
 */
INLINE int GlobalMilesManager::
get_num_samples() const {
  LightMutexHolder holder(_samples_lock);
  return _samples.size();
}

/**
 * Returns the number of sequence handles that have been allocated.
 */
INLINE int GlobalMilesManager::
get_num_sequences() const {
  LightMutexHolder holder(_sequences_lock);
  return _sequences.size();
}
