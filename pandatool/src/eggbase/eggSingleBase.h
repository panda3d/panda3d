// Filename: eggSingleBase.h
// Created by:  drose (21Jul03)
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

#ifndef EGGSINGLEBASE_H
#define EGGSINGLEBASE_H

#include "pandatoolbase.h"

#include "eggBase.h"
#include "coordinateSystem.h"
#include "eggData.h"
#include "pointerTo.h"

class EggReader;
class EggWriter;
class EggNode;
class PathReplace;

////////////////////////////////////////////////////////////////////
//       Class : EggSingleBase
// Description : This specialization of EggBase is intended for
//               programs that read and/or write a single egg file.
//               (See EggMultiBase for programs that operate on
//               multiple egg files at once.)
//
//               This is just a base class; see EggReader, EggWriter,
//               or EggFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class EggSingleBase : public EggBase {
public:
  EggSingleBase();

  virtual EggReader *as_reader();
  virtual EggWriter *as_writer();

protected:
  virtual bool post_command_line();

protected:
  PT(EggData) _data;
};

#endif


