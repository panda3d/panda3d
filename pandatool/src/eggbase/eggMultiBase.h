// Filename: eggMultiBase.h
// Created by:  drose (02Nov00)
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

#ifndef EGGMULTIBASE_H
#define EGGMULTIBASE_H

#include "pandatoolbase.h"

#include "eggBase.h"
#include "coordinateSystem.h"
#include "eggData.h"
#include "pointerTo.h"

class Filename;

////////////////////////////////////////////////////////////////////
//       Class : EggMultiBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write multiple egg files.
//
//               See also EggMultiFilter, for a class that also knows
//               how to read a bunch of egg files in and write them
//               out again.
////////////////////////////////////////////////////////////////////
class EggMultiBase : public EggBase {
public:
  EggMultiBase();

  void post_process_egg_files();

protected:
  virtual PT(EggData) read_egg(const Filename &filename);

protected:
  typedef pvector< PT(EggData) > Eggs;
  Eggs _eggs;

  bool _force_complete;
};

#endif


