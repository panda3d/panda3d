// Filename: bam2X.h
// Created by:  drose (19Jun01)
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

#ifndef BAM2X_H
#define BAM2X_H

#include <pandatoolbase.h>
#include "xFileMaker.h"

#include <programBase.h>
#include <withOutputFile.h>
#include <filename.h>

class Node;

////////////////////////////////////////////////////////////////////
//       Class : Bam2X
// Description : A program to read in a bam file and write an
//               equivalent, or nearly equivalent, DirectX-style "x"
//               file.
////////////////////////////////////////////////////////////////////
class Bam2X : public ProgramBase, public WithOutputFile {
public:
  Bam2X();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  void convert_scene_graph(Node *root);

  Filename _input_filename;
  XFileMaker _x;
};

#endif
