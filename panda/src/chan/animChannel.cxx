/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.cxx
 * @author drose
 * @date 2000-05-11
 */

#include "animChannel.h"

#include "compose_matrix.h"

template class AnimChannel<ACMatrixSwitchType>;
template class AnimChannel<ACScalarSwitchType>;

/**
 * Outputs a very brief description of a matrix.
 */
void ACMatrixSwitchType::
output_value(std::ostream &out, const ACMatrixSwitchType::ValueType &value) {
  LVecBase3 scale, shear, hpr, translate;
  if (decompose_matrix(value, scale, shear, hpr, translate)) {
    if (!scale.almost_equal(LVecBase3(1.0f, 1.0f, 1.0f))) {
      if (IS_NEARLY_EQUAL(scale[0], scale[1]) &&
          IS_NEARLY_EQUAL(scale[1], scale[2])) {
        out << " scale " << scale[0];
      } else {
        out << " scale " << scale;
      }
    }
    if (!shear.almost_equal(LVecBase3(0.0f, 0.0f, 0.0f))) {
      out << " shear " << shear;
    }

    if (!hpr.almost_equal(LVecBase3(0.0f, 0.0f, 0.0f))) {
      out << " hpr " << hpr;
    }

    if (!translate.almost_equal(LVecBase3(0.0f, 0.0f, 0.0f))) {
      out << " trans " << translate;
    }

  } else {
    out << " mat " << value;
  }
}
