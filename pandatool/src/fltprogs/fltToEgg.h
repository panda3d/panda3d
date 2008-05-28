// Filename: fltToEgg.h
// Created by:  drose (17Apr01)
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
 
#ifndef FLTTOEGG_H
#define FLTTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "fltToEggConverter.h"

#include "dSearchPath.h"

////////////////////////////////////////////////////////////////////
//       Class : FltToEgg
// Description : A program to read a flt file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class FltToEgg : public SomethingToEgg {
public:
  FltToEgg();

  void run();

  bool _compose_transforms;
};

#endif

