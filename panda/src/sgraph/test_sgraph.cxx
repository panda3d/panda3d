// Filename: test_sgraph.cxx
// Created by:  mike (02Feb00)
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

#include "camera.h"

#include <notify.h>

int main() {
  nout << "running test_sgraph" << endl;
  PT(Camera) cam = new Camera("camera");
  return 0;
}
