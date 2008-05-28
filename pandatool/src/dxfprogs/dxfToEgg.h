// Filename: dxfToEgg.h
// Created by:  drose (04May04)
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
 
#ifndef DXFTOEGG_H
#define DXFTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "dxfToEggConverter.h"

////////////////////////////////////////////////////////////////////
//       Class : DXFToEgg
// Description : A program to read a DXF file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class DXFToEgg : public SomethingToEgg {
public:
  DXFToEgg();

  void run();
};

#endif


