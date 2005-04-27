// Filename: eggRename.h
// Created by:  masad (22Apr05)
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

