// Filename: fltTrans.h
// Created by:  drose (11Apr01)
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

