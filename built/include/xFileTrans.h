/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileTrans.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILETRANS_H
#define XFILETRANS_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "withOutputFile.h"

/**
 * A program to read a X file and output an essentially similar X file.  This
 * is mainly useful to test the X file parser used in Panda.
 */
class XFileTrans : public ProgramBase, public WithOutputFile {
public:
  XFileTrans();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif
