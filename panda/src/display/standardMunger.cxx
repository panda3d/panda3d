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
StandardMunger(GraphicsStateGuardianBase *gsg, const RenderState *state,
               int num_components,
               StandardMunger::NumericType numeric_type,
               StandardMunger::Contents contents) :
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents)
{
  _gsg = DCAST(GraphicsStateGuardian, gsg);
  _render_mode = state->get_render_mode();

  _munge_color = false;
  _munge_color_scale = false;

  CPT(ColorAttrib) color_attrib = state->get_color();
  CPT(ColorScaleAttrib) color_scale_attrib = state->get_color_scale();

  if (color_attrib != (ColorAttrib *)NULL && 
      color_attrib->get_color_type() == ColorAttrib::T_flat) {

    if (!_gsg->get_color_scale_via_lighting()) {
      // We only need to munge the color directly if the GSG says it
      // can't cheat the color via lighting (presumably, in this case,
      // by applying a material).
      _color = color_attrib->get_color();
      if (color_scale_attrib != (ColorScaleAttrib *)NULL &&
          color_scale_attrib->has_scale()) {
        const LVecBase4f &cs = color_scale_attrib->get_scale();
        _color.set(_color[0] * cs[0],
                   _color[1] * cs[1],
                   _color[2] * cs[2],
                   _color[3] * cs[3]);
      }
      _munge_color = true;
    }

  } else if (color_scale_attrib != (ColorScaleAttrib *)NULL &&
             color_scale_attrib->has_scale()) {
    _color_scale = color_scale_attrib->get_scale();
    if (!_gsg->get_color_scale_via_lighting() || _color_scale[3] < 0.999f) {
      // We only need to apply the color scale by directly munging the
      // color if the GSG says it can't cheat this via lighting (for
      // instance, by applying an ambient light).  Or, since we assume
      // lighting can't scale the alpha component, if the color scale
      // involves alpha.

      // Known bug: if there is a material on an object that would
      // obscure the effect of color_scale, we scale the lighting
      // anyway, thus applying the effect even if it should be
      // obscured.  It doesn't seem worth the effort to detect this
      // contrived situation and handle it correctly.
      _munge_color_scale = true;
    }
  }
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
CPT(GeomVertexData) StandardMunger::
munge_data_impl(const GeomVertexData *data) {
  CPT(GeomVertexData) new_data = data;

  if (_munge_color) {
    new_data = new_data->set_color(_color, _num_components, _numeric_type,
                                   _contents);
  } else if (_munge_color_scale) {
    new_data = new_data->scale_color(_color_scale, _num_components, 
                                     _numeric_type, _contents);
  }

  GeomVertexAnimationSpec animation = new_data->get_format()->get_animation();
  if (hardware_animated_vertices &&
      animation.get_animation_type() == AT_panda &&
      new_data->get_slider_table() == (SliderTable *)NULL) {
    // Maybe we can animate the vertices with hardware.
    const TransformBlendTable *table = new_data->get_transform_blend_table();
    if (table != (TransformBlendTable *)NULL &&
        table->get_num_transforms() != 0 &&
        table->get_max_simultaneous_transforms() <= 
        _gsg->get_max_vertex_transforms()) {
      if (matrix_palette && 
          table->get_num_transforms() <= _gsg->get_max_vertex_transform_indices()) {

        if (table->get_num_transforms() == table->get_max_simultaneous_transforms()) {
          // We can support an indexed palette, but since that won't
          // save us any per-vertex blends, go ahead and do a plain
          // old nonindexed table instead.
          animation.set_hardware(table->get_num_transforms(), false);

        } else {
          // We can support an indexed palette, and that means we can
          // reduce the number of blends we have to specify for each
          // vertex.
          animation.set_hardware(table->get_max_simultaneous_transforms(), true);
        }

      } else if (table->get_num_transforms() <=
                 _gsg->get_max_vertex_transforms()) {
        // We can't support an indexed palette, but we have few enough
        // transforms that we can do a nonindexed table.
        animation.set_hardware(table->get_num_transforms(), false);
      }
    }
  }
  
  CPT(GeomVertexFormat) orig_format = new_data->get_format();
  CPT(GeomVertexFormat) new_format = munge_format(orig_format, animation);

  if (new_format == orig_format) {
    // Trivial case.
    return new_data;
  }

  return new_data->convert_to(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::munge_geom_impl
//       Access: Protected, Virtual
//  Description: Converts a Geom and/or its data as necessary.
////////////////////////////////////////////////////////////////////
bool StandardMunger::
munge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &vertex_data) {
  int geom_rendering = geom->get_geom_rendering();
  int supported_geom_rendering = _gsg->get_supported_geom_rendering();

  int unsupported_bits = geom_rendering & ~supported_geom_rendering;

  if (unsupported_bits != 0) {
    // Even beyond munging the vertex format, we have to convert the
    // Geom itself into a new primitive type the GSG can render
    // directly.
    if ((unsupported_bits & Geom::GR_composite_bits) != 0) {
      // This decomposes everything in the primitive, so that if (for
      // instance) the primitive contained both strips and fans, but
      // the GSG didn't support fans, it would decompose the strips
      // too.  To handle this correctly, we'd need a separate
      // decompose_fans() and decompose_strips() call; but for now,
      // we'll just say it's good enough.  In practice, we don't have
      // any GSG's that can support strips without also supporting
      // fans.
      geom = geom->decompose();
    }
    if ((unsupported_bits & Geom::GR_shade_model_bits) != 0) {
      // Rotate the vertices to account for different shade-model
      // expectations (e.g. SM_flat_last_vertex to
      // SM_flat_first_vertex)
      geom = geom->rotate();
    }
    if ((unsupported_bits & Geom::GR_indexed_bits) != 0) {
      // Convert indexed geometry to nonindexed geometry.
      PT(Geom) new_geom = new Geom(*geom);
      new_geom->make_nonindexed(false);
      geom = new_geom;
    }
  }

  return true;
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
compare_to_impl(const GeomMunger *other) const {
  const StandardMunger *om = DCAST(StandardMunger, other);
  if (_render_mode != om->_render_mode) {
    return _render_mode < om->_render_mode ? -1 : 1;
  }

  if (_munge_color != om->_munge_color) {
    return (int)_munge_color - (int)om->_munge_color;
  }
  if (_munge_color_scale != om->_munge_color_scale) {
    return (int)_munge_color_scale - (int)om->_munge_color_scale;
  }
  if (_munge_color) {
    int compare = _color.compare_to(om->_color);
    if (compare != 0) {
      return compare;
    }
  }
  if (_munge_color_scale) {
    int compare = _color_scale.compare_to(om->_color_scale);
    if (compare != 0) {
      return compare;
    }
  }

  return StateMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Compares two GeomMungers, considering only whether
//               they would produce a different answer to
//               munge_format(), munge_data(), or munge_geom().  (They
//               still might be different in other ways, but if they
//               would produce the same answer, this function consider
//               them to be the same.)
////////////////////////////////////////////////////////////////////
int StandardMunger::
geom_compare_to_impl(const GeomMunger *other) const {
  const StandardMunger *om = DCAST(StandardMunger, other);
  if (_munge_color != om->_munge_color) {
    return (int)_munge_color - (int)om->_munge_color;
  }
  if (_munge_color_scale != om->_munge_color_scale) {
    return (int)_munge_color_scale - (int)om->_munge_color_scale;
  }
  if (_munge_color) {
    int compare = _color.compare_to(om->_color);
    if (compare != 0) {
      return compare;
    }
  }
  if (_munge_color_scale) {
    int compare = _color_scale.compare_to(om->_color_scale);
    if (compare != 0) {
      return compare;
    }
  }

  return StateMunger::geom_compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::munge_state_impl
//       Access: Protectes, Virtual
//  Description: Given an input state, returns the munged state.
////////////////////////////////////////////////////////////////////
CPT(RenderState) StandardMunger::
munge_state_impl(const RenderState *state) {
  CPT(RenderState) munged_state = state;

  if (_munge_color) {
    munged_state = munged_state->remove_attrib(ColorAttrib::get_class_type());
    munged_state = munged_state->remove_attrib(ColorScaleAttrib::get_class_type());
  } else if (_munge_color_scale) {
    munged_state = munged_state->remove_attrib(ColorScaleAttrib::get_class_type());
  }

  return munged_state;
}
