// Filename: eggRename.h
// Created by:  masad (22Apr05)
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

#ifndef EGGRENAME_H
#define EGGRENAME_H

#include "pandatoolbase.h"

#include "eggMultiFilter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggTrans
// Description : A program to read an egg file and write an equivalent
//               egg file, with stripping prefix for now, but more
//               along the way.
////////////////////////////////////////////////////////////////////
class EggRename : public EggMultiFilter {
public:
  EggRename();

  void run();

  vector_string _strip_prefix;
};

#endif

