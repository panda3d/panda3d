// Filename: lwoToEgg.h
// Created by:  drose (17Apr01)
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

#ifndef LWOTOEGG_H
#define LWOTOEGG_H

#include "pandatoolbase.h"

#include "somethingToEgg.h"
#include "lwoToEggConverter.h"

#include "dSearchPath.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoToEgg
// Description : A program to read a Lightwave file and generate an egg
//               file.
////////////////////////////////////////////////////////////////////
class LwoToEgg : public SomethingToEgg {
public:
  LwoToEgg();

  void run();
};

#endif

