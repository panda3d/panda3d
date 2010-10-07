// Filename: stTransform.cxx
// Created by:  drose (06Oct10)
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

#include "stTransform.h"

STTransform STTransform::_ident_mat;

////////////////////////////////////////////////////////////////////
//     Function: STTransform::Constructor
//       Access: Published
//  Description: This constructor accepts a Panda TransformState, for
//               instance as extracted from the scene graph.
////////////////////////////////////////////////////////////////////
STTransform::
STTransform(const TransformState *trans) {
#ifndef NDEBUG
  // Ensure these are initialized to reasonable values in case we fail
  // an assertion below.
  _pos.set(0.0f, 0.0f, 0.0f);
  _rotate = 0.0f;
  _scale = 1.0f;
#endif

  nassertv(trans->has_components());
  _pos = trans->get_pos();

  const LVecBase3f &hpr = trans->get_hpr();
  nassertv(IS_NEARLY_ZERO(hpr[1]) && IS_NEARLY_ZERO(hpr[2]));
  _rotate = hpr[0];

  nassertv(trans->has_uniform_scale());
  _scale = trans->get_uniform_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: STTransform::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void STTransform::
output(ostream &out) const {
  out << "STTransform(" << _pos << ", " << _rotate << ", " << _scale << ")";
}
