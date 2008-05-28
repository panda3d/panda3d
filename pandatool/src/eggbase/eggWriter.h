// Filename: eggWriter.h
// Created by:  drose (14Feb00)
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

#ifndef EGGWRITER_H
#define EGGWRITER_H

#include "pandatoolbase.h"
#include "eggSingleBase.h"
#include "withOutputFile.h"

#include "filename.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggWriter
// Description : This is the base class for a program that generates
//               an egg file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class EggWriter : virtual public EggSingleBase, public WithOutputFile {
public:
  EggWriter(bool allow_last_param = false, bool allow_stdout = true);

  virtual EggWriter *as_writer();

  virtual void post_process_egg_file();
  void write_egg_file();

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

private:
  ofstream _output_stream;
  ostream *_output_ptr;
};

#endif


