// Filename: animChannel.cxx
// Created by:  drose (11May00)
// 
////////////////////////////////////////////////////////////////////

#include "animChannel.h"

#include <compose_matrix.h>

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
  LVecBase3f scale, hpr, translate;
  if (decompose_matrix(value, scale, hpr, translate)) {
    if (!scale.almost_equal(LVecBase3f(1.0, 1.0, 1.0))) {
      if (IS_NEARLY_EQUAL(scale[0], scale[1]) &&
          IS_NEARLY_EQUAL(scale[1], scale[2])) {
        out << " scale " << scale[0];
      } else {
        out << " scale " << scale;
      }
    }

    if (!hpr.almost_equal(LVecBase3f(0.0, 0.0, 0.0))) {
      out << " hpr " << hpr;
    }

    if (!translate.almost_equal(LVecBase3f(0.0, 0.0, 0.0))) {
      out << " trans " << translate;
    }

  } else {
    out << " mat " << value;
  }
}

