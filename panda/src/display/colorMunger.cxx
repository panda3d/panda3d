// Filename: colorMunger.cxx
// Created by:  drose (21Mar05)
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

#include "colorMunger.h"
#include "renderState.h"
#include "dcast.h"

TypeHandle ColorMunger::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorMunger::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ColorMunger::
ColorMunger(const GraphicsStateGuardianBase *gsg, const RenderState *state,
            int num_components,
            qpGeomVertexDataType::NumericType numeric_type,
            qpGeomVertexDataType::Contents contents) :
  qpGeomMunger(gsg, state),
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents)
{
  _color = state->get_color();
  _color_scale = state->get_color_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ColorMunger::
~ColorMunger() {
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMunger::munge_data_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexData, converts it as
//               necessary for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) ColorMunger::
munge_data_impl(const qpGeomVertexData *data) {
  CPT(qpGeomVertexData) new_data = data;

  if (_color != (ColorAttrib *)NULL && 
      _color->get_color_type() == ColorAttrib::T_flat) {
    Colorf color = _color->get_color();
    if (_color_scale != (ColorScaleAttrib *)NULL &&
        _color_scale->has_scale()) {
      const LVecBase4f &cs = _color_scale->get_scale();
      color.set(color[0] * cs[0],
                color[1] * cs[1],
                color[2] * cs[2],
                color[3] * cs[3]);
    }
    new_data = new_data->set_color(color, _num_components, _numeric_type,
                                   _contents);

  } else if (_color_scale != (ColorScaleAttrib *)NULL &&
             _color_scale->has_scale()) {
    const LVecBase4f &cs = _color_scale->get_scale();
    new_data = new_data->scale_color(cs, _num_components, _numeric_type,
                                     _contents);
  }

  return qpGeomMunger::munge_data_impl(new_data);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMunger::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int ColorMunger::
compare_to_impl(const qpGeomMunger *other) const {
  const ColorMunger *om = DCAST(ColorMunger, other);
  if (_color != om->_color) {
    return _color < om->_color ? -1 : 1;
  }
  if (_color_scale != om->_color_scale) {
    return _color_scale < om->_color_scale ? -1 : 1;
  }

  return qpGeomMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int ColorMunger::
geom_compare_to_impl(const qpGeomMunger *other) const {
  const ColorMunger *om = DCAST(ColorMunger, other);
  if (_color != om->_color) {
    return _color < om->_color ? -1 : 1;
  }
  if (_color_scale != om->_color_scale) {
    return _color_scale < om->_color_scale ? -1 : 1;
  }

  return qpGeomMunger::geom_compare_to_impl(other);
}
