// Filename: standardMunger.cxx
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

#include "standardMunger.h"
#include "renderState.h"
#include "graphicsStateGuardian.h"
#include "dcast.h"
#include "config_gobj.h"

TypeHandle StandardMunger::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::Constructor
//       Access: Public
//  Description: The StandardMunger constructor accepts additional
//               parameters that specify the GSG's preferred color
//               format (since we might be munging the color anyway,
//               we might as well convert it as we munge).
////////////////////////////////////////////////////////////////////
StandardMunger::
StandardMunger(const GraphicsStateGuardianBase *gsg, const RenderState *state,
               int num_components,
               qpGeomVertexDataType::NumericType numeric_type,
               qpGeomVertexDataType::Contents contents) :
  qpGeomMunger(gsg, state),
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents)
{
  _gsg = DCAST(GraphicsStateGuardian, gsg);
  _color = state->get_color();
  _color_scale = state->get_color_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
StandardMunger::
~StandardMunger() {
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::munge_data_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexData, converts it as
//               necessary for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexData) StandardMunger::
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

  qpGeomVertexAnimationSpec animation = data->get_format()->get_animation();
  if (hardware_animated_vertices &&
      animation.get_animation_type() == qpGeomVertexAnimationSpec::AT_panda &&
      data->get_slider_table() == (SliderTable *)NULL) {
    // Maybe we can animate the vertices with hardware.
    const TransformBlendPalette *palette = data->get_transform_blend_palette();
    if (palette != (TransformBlendPalette *)NULL &&
        palette->get_max_simultaneous_transforms() <= 
        _gsg->get_max_vertex_transforms()) {
      if (palette->get_num_transforms() <= 
          _gsg->get_max_vertex_transform_indices()) {

        if (palette->get_num_transforms() == palette->get_max_simultaneous_transforms()) {
          // We can support an indexed palette, but since that won't
          // save us any per-vertex blends, go ahead and do a plain
          // old nonindexed palette instead.
          animation.set_hardware(palette->get_num_transforms(), false);

        } else {
          // We can support an indexed palette, and that means we can
          // reduce the number of blends we have to specify for each
          // vertex.
          animation.set_hardware(palette->get_max_simultaneous_transforms(), true);
        }

      } else if (palette->get_num_transforms() <=
                 _gsg->get_max_vertex_transforms()) {
        // We can't support an indexed palette, but we have few enough
        // transforms that we can do a nonindexed palette.
        animation.set_hardware(palette->get_num_transforms(), false);
      }
    }
  }
  
  CPT(qpGeomVertexFormat) orig_format = data->get_format();
  CPT(qpGeomVertexFormat) new_format = munge_format(orig_format, animation);

  if (new_format == orig_format) {
    // Trivial case.
    return data;
  }

  return data->convert_to(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int StandardMunger::
compare_to_impl(const qpGeomMunger *other) const {
  const StandardMunger *om = DCAST(StandardMunger, other);
  if (_color != om->_color) {
    return _color < om->_color ? -1 : 1;
  }
  if (_color_scale != om->_color_scale) {
    return _color_scale < om->_color_scale ? -1 : 1;
  }

  return qpGeomMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int StandardMunger::
geom_compare_to_impl(const qpGeomMunger *other) const {
  const StandardMunger *om = DCAST(StandardMunger, other);
  if (_color != om->_color) {
    return _color < om->_color ? -1 : 1;
  }
  if (_color_scale != om->_color_scale) {
    return _color_scale < om->_color_scale ? -1 : 1;
  }

  return qpGeomMunger::geom_compare_to_impl(other);
}
