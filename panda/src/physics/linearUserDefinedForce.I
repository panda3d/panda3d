// Filename: linearUserDefinedForce.I
// Created by:  charles (31Jul00)
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
// Function : set_proc
//   Access : public
////////////////////////////////////////////////////////////////////
void LinearUserDefinedForce::
set_proc(LVector3 (*proc)(const PhysicsObject *)) {
  _proc = proc;
}
