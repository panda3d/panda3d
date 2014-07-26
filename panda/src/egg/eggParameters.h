// Filename: eggParameters.h
// Created by:  drose (16Jan99)
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

#ifndef EGGPARAMETERS_H
#define EGGPARAMETERS_H

#include "pandabase.h"

#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
//       Class : EggParameters
// Description : The values stored in this structure are global
//               parameters that control some aspects of the egg
//               library.  User code may adjust these parameters by
//               meddling with the values in structure directly, or by
//               fiddling with the pointer to completely replace the
//               structure.
//
//               However, these parameters should not be changed at
//               any time during the processing of any egg structure:
//               set the parameters, load an egg file, process it, and
//               write the egg file out again before resetting the
//               parameters again.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggParameters {
public:
  EggParameters();
  EggParameters(const EggParameters &copy);

  // The per-component difference below which two vertices are deemed
  // to be at the same position.
  double _pos_threshold;

  // The per-component difference below which two vertices are deemed
  // to have the same normal.
  double _normal_threshold;

  // The per-component difference below which two vertices are deemed
  // to have the same texture coordinates.
  double _uv_threshold;

  // The per-component difference below which two vertices are deemed
  // to have the same color.
  PN_stdfloat _color_threshold;

  // The per-component difference below which two anim table values
  // are deemed to be equivalent.
  double _table_threshold;
};

extern EXPCL_PANDAEGG EggParameters *egg_parameters;

#endif
