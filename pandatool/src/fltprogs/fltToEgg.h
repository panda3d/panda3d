// Filename: fltToEgg.h
// Created by:  drose (17Apr01)
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

