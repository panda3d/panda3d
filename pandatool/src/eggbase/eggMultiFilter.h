// Filename: eggMultiFilter.h
// Created by:  drose (02Nov00)
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

#ifndef EGGMULTIFILTER_H
#define EGGMULTIFILTER_H

#include "pandatoolbase.h"

#include "eggMultiBase.h"

////////////////////////////////////////////////////////////////////
//       Class : EggMultiFilter
// Description : This is a base class for a program that reads in a
//               number of egg files, operates on them, and writes
//               them out again (presumably to a different directory).
////////////////////////////////////////////////////////////////////
class EggMultiFilter : public EggMultiBase {
public:
  EggMultiFilter(bool allow_empty = false);

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  Filename get_output_filename(const Filename &source_filename) const;
  virtual void write_eggs();

protected:
  bool _allow_empty;
  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _inplace;

  bool _read_only;
};

#endif


