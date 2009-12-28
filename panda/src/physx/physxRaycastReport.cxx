// Filename: physxRaycastReport.cxx
// Created by:  enn0x (21Oct09)
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

#include "physxRaycastReport.h"
#include "physxRaycastHit.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastReport::onRaycast
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxRaycastReport::
onHit(const NxRaycastHit& hit) {

  _hits.push_back(PhysxRaycastHit(hit));
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastReport::get_num_hits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxRaycastReport::
get_num_hits() const {

  return _hits.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastReport::get_first_hit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxRaycastHit PhysxRaycastReport::
get_first_hit() {

  _iterator = _hits.begin();
  return get_next_hit();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastReport::get_next_hit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxRaycastHit PhysxRaycastReport::
get_next_hit() {

  if (_iterator != _hits.end()) {
    return *_iterator++;
  }

  // No more items. Return an empty hit.
  NxRaycastHit hit;
  hit.shape = NULL;
  return PhysxRaycastHit(hit);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxRaycastReport::get_hit
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxRaycastHit PhysxRaycastReport::
get_hit(unsigned int idx) {

  if (!(idx < _hits.size()))
  {
    // Index out of bounds. Return an empty hit.
    NxRaycastHit hit;
    hit.shape = NULL;
    return PhysxRaycastHit(hit);
  }

  return _hits[idx];
}

