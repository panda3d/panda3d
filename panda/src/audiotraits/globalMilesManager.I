// Filename: globalMilesManager.I
// Created by:  drose (30Jul07)
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
//     Function: GlobalMilesManager::is_open
//       Access: Public
//  Description: Returns true if the Miles Sound System is open (and
//               active), false if it is not.
////////////////////////////////////////////////////////////////////
INLINE bool GlobalMilesManager::
is_open() const {
  return _is_open;
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::get_num_samples
//       Access: Public
//  Description: Returns the number of sample handles that have been
//               allocated.
////////////////////////////////////////////////////////////////////
INLINE int GlobalMilesManager::
get_num_samples() const {
  LightMutexHolder holder(_samples_lock);
  return _samples.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GlobalMilesManager::get_num_sequences
//       Access: Public
//  Description: Returns the number of sequence handles that have been
//               allocated.
////////////////////////////////////////////////////////////////////
INLINE int GlobalMilesManager::
get_num_sequences() const {
  LightMutexHolder holder(_sequences_lock);
  return _sequences.size();
}
