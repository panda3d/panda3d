// Filename: lwoScan.h
// Created by:  drose (30Apr01)
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

#ifndef LWOSCAN_H
#define LWOSCAN_H

#include "programBase.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : LwoScan
// Description : A program to read a Lightwave file and report its
//               structure and contents.
////////////////////////////////////////////////////////////////////
class LwoScan : public ProgramBase {
public:
  LwoScan();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif

