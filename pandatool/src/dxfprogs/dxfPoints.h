// Filename: dxfPoints.h
// Created by:  drose (04May04)
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

#ifndef DXFPOINTS_H
#define DXFPOINTS_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "withOutputFile.h"

#include "dxfFile.h"

////////////////////////////////////////////////////////////////////
//       Class : DXFPoints
// Description : A simple program to read a dxf file and list the
//               points contained within it to a text file.
////////////////////////////////////////////////////////////////////
class DXFPoints : public ProgramBase, public WithOutputFile, public DXFFile {
public:
  DXFPoints();

  void run();

  virtual void done_entity();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif

