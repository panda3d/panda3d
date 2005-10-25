// Filename: eggToMaya.h
// Created by:  drose (11Aug05)
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

#ifndef EGGTOMAYA_H
#define EGGTOMAYA_H

#include "pandatoolbase.h"

#include "eggToSomething.h"

////////////////////////////////////////////////////////////////////
//       Class : EggToMaya
// Description : A program to read an egg file and write a maya file.
////////////////////////////////////////////////////////////////////
class EggToMaya : public EggToSomething {
public:
  EggToMaya();

  void run();

private:
  bool _convert_anim;
  bool _convert_model;
};

#endif

