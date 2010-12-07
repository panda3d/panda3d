// Filename: objToEgg.h
// Created by:  drose (07Dec10)
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
 
#ifndef OBJTOEGG_H
#define OBJTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "objToEggConverter.h"

////////////////////////////////////////////////////////////////////
//       Class : ObjToEgg
// Description : A program to read a Obj file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class ObjToEgg : public SomethingToEgg {
public:
  ObjToEgg();

  void run();
};

#endif
