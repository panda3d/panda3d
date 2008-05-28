// Filename: eggMakeSomething.cxx
// Created by:  drose (01Oct03)
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

#include "eggMakeSomething.h"

////////////////////////////////////////////////////////////////////
//     Function: EggMakeSomething::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggMakeSomething::
EggMakeSomething() :
  EggWriter(true, true)
{
  add_normals_options();
  add_transform_options();
}

