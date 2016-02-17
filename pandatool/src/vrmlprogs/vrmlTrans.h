/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrmlTrans.h
 * @author drose
 * @date 2004-10-01
 */

#ifndef VRMLTRANS_H
#define VRMLTRANS_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "withOutputFile.h"

/**
 * A program to read a VRML file and output an essentially similar VRML file.
 * This is mainly useful to test the VRML parser used in Panda.
 */
class VRMLTrans : public ProgramBase, public WithOutputFile {
public:
  VRMLTrans();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif
