// Filename: eggSingleBase.h
// Created by:  drose (21Jul03)
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


