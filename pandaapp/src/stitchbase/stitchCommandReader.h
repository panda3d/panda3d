// Filename: stitchCommandReader.h
// Created by:  drose (16Mar00)
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

#ifndef STITCHCOMMANDREADER_H
#define STITCHCOMMANDREADER_H

#include <pandatoolbase.h>

#include "stitchFile.h"

#include <programBase.h>

////////////////////////////////////////////////////////////////////
//       Class : StitchCommandReader
// Description : This specialization of ProgramBase is intended for
//               programs in this directory that read and process a
//               stitch command file.
//////////////////////////////////////////////////////////////////////
class StitchCommandReader : public ProgramBase {
public:
  StitchCommandReader();

protected:
  virtual bool handle_args(Args &args);

protected:
  StitchFile _command_file;
};

#endif


