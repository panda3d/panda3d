// Filename: eggSwitchCondition.cxx
// Created by:  drose (08Feb99)
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

#include "eggSwitchCondition.h"

#include "indent.h"

TypeHandle EggSwitchCondition::_type_handle;
TypeHandle EggSwitchConditionDistance::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggSwitchConditionDistance::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggSwitchConditionDistance::
EggSwitchConditionDistance(double switch_in, double switch_out,
                           const LPoint3d &center, double fade) {
  _switch_in = switch_in;
  _switch_out = switch_out;
  _center = center;
  _fade = fade;
}


////////////////////////////////////////////////////////////////////
//     Function: EggSwitchConditionDistance::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggSwitchCondition *EggSwitchConditionDistance::
make_copy() const {
  return new EggSwitchConditionDistance(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: EggSwitchConditionDistance::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void EggSwitchConditionDistance::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << "<SwitchCondition> {\n";
  indent(out, indent_level+2)
    << "<Distance> { " << _switch_in << " " << _switch_out;

  if (_fade != 0.0) {
    out << " " << _fade;
  }

  out << " <Vertex> { " << _center << " } }\n";
  indent(out, indent_level) << "}\n";
}


////////////////////////////////////////////////////////////////////
//     Function: EggSwitchConditionDistance::transform
//       Access: Public, Virtual
//  Description: Applies the indicated transformation matrix to the
//               switch condition parameters.
////////////////////////////////////////////////////////////////////
void EggSwitchConditionDistance::
transform(const LMatrix4d &mat) {
  _center = _center * mat;
}
