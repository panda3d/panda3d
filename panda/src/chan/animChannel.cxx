// Filename: animChannel.cxx
// Created by:  drose (11May00)
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


#include "animChannel.h"

#include "compose_matrix.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

////////////////////////////////////////////////////////////////////
//     Function: ACMatrixSwitchType::output_value
//       Access: Public, Static
//  Description: Outputs a very brief description of a matrix.
////////////////////////////////////////////////////////////////////
void ACMatrixSwitchType::
output_value(ostream &out, const ACMatrixSwitchType::ValueType &value) {
  LVecBase3f scale, shear, hpr, translate;
  if (decompose_matrix(value, scale, shear, hpr, translate)) {
    if (!scale.almost_equal(LVecBase3f(1.0f, 1.0f, 1.0f))) {
      if (IS_NEARLY_EQUAL(scale[0], scale[1]) &&
          IS_NEARLY_EQUAL(scale[1], scale[2])) {
        out << " scale " << scale[0];
      } else {
        out << " scale " << scale;
      }
    }
    if (!shear.almost_equal(LVecBase3f(0.0f, 0.0f, 0.0f))) {
      out << " shear " << shear;
    }

    if (!hpr.almost_equal(LVecBase3f(0.0f, 0.0f, 0.0f))) {
      out << " hpr " << hpr;
    }

    if (!translate.almost_equal(LVecBase3f(0.0f, 0.0f, 0.0f))) {
      out << " trans " << translate;
    }

  } else {
    out << " mat " << value;
  }
}

