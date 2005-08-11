// Filename: eggToSomething.h
// Created by:  drose (15Feb00)
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

#ifndef EGGTOSOMETHING_H
#define EGGTOSOMETHING_H

#include "pandatoolbase.h"

#include "eggConverter.h"
#include "distanceUnit.h"

////////////////////////////////////////////////////////////////////
//       Class : EggToSomething
// Description : This is the general base class for a file-converter
//               program that reads some model file format and
//               generates an egg file.
////////////////////////////////////////////////////////////////////
class EggToSomething : public EggConverter {
public:
  EggToSomething(const string &format_name,
                 const string &preferred_extension = string(),
                 bool allow_last_param = true,
                 bool allow_stdout = true);

  void add_units_options();

protected:
  void apply_units_scale(EggData *data);
  virtual void pre_process_egg_file();
  virtual bool handle_args(Args &args);

  DistanceUnit _input_units;
  DistanceUnit _output_units;
};

#endif


