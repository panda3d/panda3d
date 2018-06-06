/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file binToC.h
 * @author drose
 * @date 2003-07-18
 */

#ifndef BINTOC_H
#define BINTOC_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "withOutputFile.h"

/**
 * A utility program to read a (binary) file and output a table that can be
 * compiled via a C compiler to generate the same data.  Handy for portably
 * importing binary data into a library or executable.
 */
class BinToC : public ProgramBase, public WithOutputFile {
public:
  BinToC();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
  std::string _table_name;
  bool _static_table;
  bool _for_string;
};

#endif
