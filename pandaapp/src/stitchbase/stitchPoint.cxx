// Filename: stitchPoint.cxx
// Created by:  drose (04Nov99)
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

#include "stitchPoint.h"

StitchPoint::
StitchPoint(const string &name) :
  _name(name)
{
  _space_known = false;
}

void StitchPoint::
set_space(const LVector3d &space) {
  _space_known = true;
  _space = space;
}

