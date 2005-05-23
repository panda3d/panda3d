// Filename: eggListTextures.h
// Created by:  drose (23May05)
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

#ifndef EGGLISTTEXTURES_H
#define EGGLISTTEXTURES_H

#include "pandatoolbase.h"

#include "eggReader.h"

////////////////////////////////////////////////////////////////////
//       Class : EggListTextures
// Description : Reads an egg file and outputs the list of textures it
//               uses.
////////////////////////////////////////////////////////////////////
class EggListTextures : public EggReader {
public:
  EggListTextures();

  void run();
};

#endif

