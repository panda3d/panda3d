/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxfPoints.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef DXFPOINTS_H
#define DXFPOINTS_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "withOutputFile.h"

#include "dxfFile.h"

/**
 * A simple program to read a dxf file and list the points contained within it
 * to a text file.
 */
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
