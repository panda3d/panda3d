// Filename: cullResult.cxx
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "cullResult.h"
#include "cullBinManager.h"

////////////////////////////////////////////////////////////////////
//     Function: CullResult::make_next
//       Access: Public
//  Description: Returns a newly-allocated CullResult object that
//               contains a copy of just the subset of the data from
//               this CullResult object that is worth keeping around
//               for next frame.
////////////////////////////////////////////////////////////////////
PT(CullResult) CullResult::
make_next() const {
  PT(CullResult) new_result = new CullResult(_gsg);
  new_result->_bins.reserve(_bins.size());

  for (Bins::const_iterator bi = _bins.begin(); bi != _bins.end(); ++bi) {
    CullBin *old_bin = (*bi);
    if (old_bin == (CullBin *)NULL) {
      new_result->_bins.push_back((CullBin *)NULL);
    } else {
      new_result->_bins.push_back(old_bin->make_next());
    }
  }

  return new_result;
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::finish_cull
//       Access: Public
//  Description: Called after all the geoms have been added, this
//               indicates that the cull process is finished for this
//               frame and gives the bins a chance to do any
//               post-processing (like sorting) before moving on to
//               draw.
////////////////////////////////////////////////////////////////////
void CullResult::
finish_cull() {
  for (Bins::iterator bi = _bins.begin(); bi != _bins.end(); ++bi) {
    CullBin *bin = (*bi);
    if (bin != (CullBin *)NULL) {
      bin->finish_cull();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::draw
//       Access: Public
//  Description: Asks all the bins to draw themselves in the correct
//               order.
////////////////////////////////////////////////////////////////////
void CullResult::
draw() {
  // Ask the bin manager for the correct order to draw all the bins.
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  int num_bins = bin_manager->get_num_bins();
  for (int i = 0; i < num_bins; i++) {
    int bin_index = bin_manager->get_bin(i);
    nassertv(bin_index >= 0);

    if (bin_index < (int)_bins.size() && _bins[bin_index] != (CullBin *)NULL) {
      _bins[bin_index]->draw();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::bin_removed
//       Access: Public, Static
//  Description: Intended to be called by
//               CullBinManager::remove_bin(), this informs all the
//               CullResults in the world to remove the indicated
//               bin_index from their cache if it has been cached.
////////////////////////////////////////////////////////////////////
void CullResult::
bin_removed(int bin_index) {
  // Do something here.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: CullResult::make_new_bin
//       Access: Private
//  Description: Allocates a new CullBin for the given bin_index and
//               stores it for next time.
////////////////////////////////////////////////////////////////////
CullBin *CullResult::
make_new_bin(int bin_index) {
  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  PT(CullBin) bin = bin_manager->make_new_bin(bin_index, _gsg);
  if (bin != (CullBin *)NULL) {
    // Now store it in the vector.
    while (bin_index >= (int)_bins.size()) {
      _bins.push_back((CullBin *)NULL);
    }
    nassertr(bin_index >= 0 && bin_index < (int)_bins.size(), NULL);
    _bins[bin_index] = bin;
  }

  return bin;
}
