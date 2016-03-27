/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoScan.h
 * @author drose
 * @date 2001-04-30
 */

#ifndef LWOSCAN_H
#define LWOSCAN_H

#include "programBase.h"
#include "filename.h"

/**
 * A program to read a Lightwave file and report its structure and contents.
 */
class LwoScan : public ProgramBase {
public:
  LwoScan();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif
