// Filename: eggSliderData.h
// Created by:  drose (26Feb01)
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

#ifndef EGGSLIDERDATA_H
#define EGGSLIDERDATA_H

#include <pandatoolbase.h>

#include "eggComponentData.h"


////////////////////////////////////////////////////////////////////
//       Class : EggSliderData
// Description : This corresponds to a single morph slider control.
//               It contains back pointers to all the vertices and
//               primitives that reference this slider across all
//               models, as well as all the tables in which it appears
//               in all animation files.
////////////////////////////////////////////////////////////////////
class EggSliderData : public EggComponentData {
public:
  EggSliderData(EggCharacterCollection *collection,
                EggCharacterData *char_data);

  virtual void add_back_pointer(int model_index, EggObject *egg_object);
  virtual void write(ostream &out, int indent_level = 0) const;
};

#include "eggSliderData.I"

#endif


