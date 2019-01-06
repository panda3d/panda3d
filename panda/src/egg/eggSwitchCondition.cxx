/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggSwitchCondition.cxx
 * @author drose
 * @date 1999-02-08
 */

#include "eggSwitchCondition.h"

#include "indent.h"

TypeHandle EggSwitchCondition::_type_handle;
TypeHandle EggSwitchConditionDistance::_type_handle;


/**
 *
 */
EggSwitchConditionDistance::
EggSwitchConditionDistance(double switch_in, double switch_out,
                           const LPoint3d &center, double fade) {
  _switch_in = switch_in;
  _switch_out = switch_out;
  _center = center;
  _fade = fade;
}


/**
 *
 */
EggSwitchCondition *EggSwitchConditionDistance::
make_copy() const {
  return new EggSwitchConditionDistance(*this);
}


/**
 *
 */
void EggSwitchConditionDistance::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << "<SwitchCondition> {\n";
  indent(out, indent_level+2)
    << "<Distance> { " << _switch_in << " " << _switch_out;

  if (_fade != 0.0) {
    out << " " << _fade;
  }

  out << " <Vertex> { " << _center << " } }\n";
  indent(out, indent_level) << "}\n";
}


/**
 * Applies the indicated transformation matrix to the switch condition
 * parameters.
 */
void EggSwitchConditionDistance::
transform(const LMatrix4d &mat) {
  _center = _center * mat;

  LVector3d in = LVector3d(_switch_in, 0.0, 0.0) * mat;
  LVector3d out = LVector3d(_switch_out, 0.0, 0.0) * mat;

  _switch_in = in.length();
  _switch_out = out.length();
}
