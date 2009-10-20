// Filename: cConstraintInterval.cxx
// Created by:  pratt (29Sep06)
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

#include "cConstraintInterval.h"

TypeHandle CConstraintInterval::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CConstraintInterval::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CConstraintInterval::
CConstraintInterval(const string &name, double duration) :
  CInterval(name, duration, true)
{
}
