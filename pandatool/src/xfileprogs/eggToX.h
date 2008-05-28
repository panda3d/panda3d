// Filename: eggToX.h
// Created by:  drose (19Jun01)
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
