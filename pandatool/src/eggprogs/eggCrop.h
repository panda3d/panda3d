// Filename: eggCrop.h
// Created by:  drose (10Jun02)
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

#ifndef EGGCROP_H
#define EGGCROP_H

#include "pandatoolbase.h"

#include "eggFilter.h"

class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : EggCrop
// Description : A program to read an egg file and write an equivalent
//               egg file, possibly performing some minor operations
//               along the way.
////////////////////////////////////////////////////////////////////
class EggCrop : public EggFilter {
public:
  EggCrop();

  virtual bool post_command_line();
  void run();

private:
  int strip_prims(EggGroupNode *group);

  bool _got_min, _got_max;
  LVecBase3d _min, _max;
};

#endif

