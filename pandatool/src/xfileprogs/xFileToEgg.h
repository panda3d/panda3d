// Filename: xFileToEgg.h
// Created by:  drose (21Jun01)
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

#ifndef XFILETOEGG_H
#define XFILETOEGG_H

#include "pandatoolbase.h"
#include "somethingToEgg.h"
#include "xFileToEggConverter.h"

#include "dSearchPath.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileToEgg
// Description : A program to read a DirectX "x" file and generate an
//               egg file.
////////////////////////////////////////////////////////////////////
class XFileToEgg : public SomethingToEgg {
public:
  XFileToEgg();

  void run();
};

#endif

