// Filename: dataRelation.cxx
// Created by:  drose (08Jun00)
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


#include "dataRelation.h"

#include "transformTransition.h"

TypeHandle DataRelation::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DataRelation::make_arc
//       Access: Public, Static
//  Description: This function is passed to the Factory to make a new
//               DataRelation by type.  Don't try to call this
//               function directly.
////////////////////////////////////////////////////////////////////
NodeRelation *DataRelation::
make_arc(const FactoryParams &) {
  return new DataRelation;
}
