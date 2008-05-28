// Filename: eggMakeSomething.h
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

#ifndef EGGMAKESOMETHING_H
#define EGGMAKESOMETHING_H

#include "pandatoolbase.h"

#include "eggWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggMakeSomething
// Description : A base class for a family of programs that generate
//               egg models of various fundamental shapes.
////////////////////////////////////////////////////////////////////
class EggMakeSomething : public EggWriter {
public:
  EggMakeSomething();
};

#endif

