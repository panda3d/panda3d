// Filename: colorMatrixTransition.cxx
// Created by:  jason (01Aug00)
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

#include "colorMatrixTransition.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamWriter.h>
#include <bamReader.h>

PT(NodeTransition) ColorMatrixTransition::_initial;
TypeHandle ColorMatrixTransition::_type_handle;

////////////////////////////////////////////////////////////////////s
//     Function: ColorMatrixTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorMatrixTransition::
make_copy() const {
  return new ColorMatrixTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorMatrixTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorMatrixTransition::
make_initial() const {
  if (_initial.is_null()) {
    _initial = new ColorMatrixTransition;
  }
  return _initial;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorMatrixTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color_transform(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_with_matrix
//       Access: Protected, Virtual
//  Description: Returns a new transition with the indicated matrix.
////////////////////////////////////////////////////////////////////
MatrixTransition<LMatrix4f> *ColorMatrixTransition::
make_with_matrix(const LMatrix4f &matrix) const {
  return new ColorMatrixTransition(matrix);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::make_ColorMatrixTransition
//       Access: Protected
//  Description: Factory method to generate a ColorMatrixTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* ColorMatrixTransition::
make_ColorMatrixTransition(const FactoryParams &params)
{
  ColorMatrixTransition *me = new ColorMatrixTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMatrixTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a ColorMatrixTransition object
////////////////////////////////////////////////////////////////////
void ColorMatrixTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_ColorMatrixTransition);
}
