// Filename: eggBase.h
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

#ifndef EGGBASE_H
#define EGGBASE_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "coordinateSystem.h"
#include "eggData.h"

class EggReader;
class EggWriter;
class EggNode;
class PathReplace;

////////////////////////////////////////////////////////////////////
//       Class : EggBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write a single egg file.
//               (See EggMultiBase for programs that operate on
//               multiple egg files at once.)
//
//               This is just a base class; see EggReader, EggWriter,
//               or EggFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class EggBase : public ProgramBase {
public:
  EggBase();

  virtual EggReader *as_reader();
  virtual EggWriter *as_writer();

  static void convert_paths(EggNode *node, PathReplace *path_replace,
                            const DSearchPath &additional_path);

protected:
  virtual bool post_command_line();
  void append_command_comment(EggData &_data);

protected:
  bool _got_coordinate_system;
  CoordinateSystem _coordinate_system;
  EggData _data;
};

#endif


