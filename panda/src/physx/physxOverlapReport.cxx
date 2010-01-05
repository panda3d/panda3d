// Filename: physOverlapReport.cxx
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

#include "physxOverlapReport.h"
#include "physxShape.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxOverlapReport::onEvent
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxOverlapReport::
onEvent(NxU32 nbEntities, NxShape **entities) {

  for (unsigned int i=0; i<nbEntities; i++) {
    PhysxShape *shape = (PhysxShape *)entities[i]->userData;
    _overlaps.push_back(shape);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOverlapReport::get_num_overlaps
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxOverlapReport::
get_num_overlaps() const {

  return _overlaps.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOverlapReport::get_first_overlap
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxOverlapReport::
get_first_overlap() {

  _iterator = _overlaps.begin();
  return get_next_overlap();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOverlapReport::get_next_overlap
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxOverlapReport::
get_next_overlap() {

  if (_iterator != _overlaps.end()) {
    return *_iterator++;
  }

  // No more items. Return empty overlap.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxOverlapReport::get_overlap
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PhysxShape *PhysxOverlapReport::
get_overlap(unsigned int idx) {

  nassertr(idx < get_num_overlaps(), NULL);
  return _overlaps[idx];
}

