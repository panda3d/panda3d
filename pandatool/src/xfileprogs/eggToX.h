// Filename: eggToX.h
// Created by:  drose (19Jun01)
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

#ifndef EGGTOX_H
#define EGGTOX_H

#include "pandatoolbase.h"
#include "eggToSomething.h"
#include "xFileMaker.h"

#include "programBase.h"
#include "withOutputFile.h"
#include "filename.h"

class Node;

////////////////////////////////////////////////////////////////////
//       Class : EggToX
// Description : A program to read in a egg file and write an
//               equivalent, or nearly equivalent, DirectX-style "x"
//               file.
////////////////////////////////////////////////////////////////////
class EggToX : public EggToSomething {
public:
  EggToX();

  void run();

private:
  void convert_scene_graph(Node *root);

  Filename _input_filename;
  XFileMaker _x;
};

#endif
