// Filename: eggFilter.h
// Created by:  drose (14Feb00)
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

#ifndef EGGFILTER_H
#define EGGFILTER_H

#include "pandatoolbase.h"

#include "eggReader.h"
#include "eggWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggFilter
// Description : This is the base class for a program that reads an
//               egg file, operates on it, and writes another egg file
//               out.
////////////////////////////////////////////////////////////////////
class EggFilter : public EggReader, public EggWriter {
public:
  EggFilter(bool allow_last_param = false, bool allow_stdout = true);

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();
};

#endif


