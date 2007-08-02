// Filename: globalMilesManager.I
// Created by:  drose (30Jul07)
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
  MutexHolder holder(_samples_lock);
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
  MutexHolder holder(_sequences_lock);
  return _sequences.size();
}
