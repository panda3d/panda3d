// Filename: testCopy.h
// Created by:  drose (31Oct00)
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

#ifndef TESTCOPY_H
#define TESTCOPY_H

#include "pandatoolbase.h"

#include "cvsCopy.h"

////////////////////////////////////////////////////////////////////
//       Class : TestCopy
// Description : A program to copy ordinary files into the cvs tree.
//               Mainly to test CVSCopy.
////////////////////////////////////////////////////////////////////
class TestCopy : public CVSCopy {
public:
  TestCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
                         CVSSourceDirectory *dir, void *extra_data,
                         bool new_file);
};

#endif
