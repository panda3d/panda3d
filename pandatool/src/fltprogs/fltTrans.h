// Filename: fltTrans.h
// Created by:  drose (11Apr01)
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

#ifndef FLTTRANS_H
#define FLTTRANS_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "withOutputFile.h"

////////////////////////////////////////////////////////////////////
//       Class : FltTrans
// Description : A program to read a flt file and write an equivalent
//               flt file, possibly performing some minor operations
//               along the way.
////////////////////////////////////////////////////////////////////
class FltTrans : public ProgramBase, public WithOutputFile {
public:
  FltTrans();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
  bool _got_new_version;
  double _new_version;
};

#endif

