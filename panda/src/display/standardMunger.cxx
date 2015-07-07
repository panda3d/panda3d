// Filename: standardMunger.cxx
// Created by:  drose (21Mar05)
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

#include "standardMunger.h"
#include "renderState.h"
#include "graphicsStateGuardian.h"
#include "dcast.h"
#include "config_gobj.h"
#include "displayRegion.h"

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
  StateMunger(gsg),
  _num_components(num_components),
  _numeric_type(numeric_type),
  _contents(contents),
  _munge_color(false),
  _munge_color_scale(false),
  _auto_shader(false),
  _shader_skinning(false)
{
  _render_mode = DCAST(RenderModeAttrib, state->get_attrib(RenderModeAttrib::get_class_slot()));

  if (!get_gsg()->get_runtime_color_scale()) {
    // We might need to munge the colors.
    const ColorAttrib *color_attrib = (const ColorAttrib *)
      state->get_attrib(ColorAttrib::get_class_slot());
    const ColorScaleAttrib *color_scale_attrib = (const ColorScaleAttrib *)
      state->get_attrib(ColorScaleAttrib::get_class_slot());

    if (color_attrib != (ColorAttrib *)NULL && 
        color_attrib->get_color_type() == ColorAttrib::T_flat) {

      if (!get_gsg()->get_color_scale_via_lighting()) {
        // We only need to munge the color directly if the GSG says it
        // can't cheat the color via lighting (presumably, in this case,
        // by applying a material).
        _color = color_attrib->get_color();
        if (color_scale_attrib != (ColorScaleAttrib *)NULL &&
            color_scale_attrib->has_scale()) {
          const LVecBase4 &cs = color_scale_attrib->get_scale();
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
      
      const TextureAttrib *tex_attrib = (const TextureAttrib *)
        state->get_attrib(TextureAttrib::get_class_slot());

      // If the GSG says it can't cheat this RGB or alpha scale, we have
      // to apply the color scale directly.
      if ((color_scale_attrib->has_rgb_scale() && !get_gsg()->get_color_scale_via_lighting()) ||
          (color_scale_attrib->has_alpha_scale() && !get_gsg()->get_alpha_scale_via_texture(tex_attrib))) {
        _munge_color_scale = true;
      }

      // Known bug: if there is a material on an object that would
      // obscure the effect of color_scale, we scale the lighting
      // anyway, thus applying the effect even if it should be obscured.
      // It doesn't seem worth the effort to detect this contrived
      // situation and handle it correctly.
    }
  }

  const ShaderAttrib *shader_attrib = (const ShaderAttrib *)
    state->get_attrib_def(ShaderAttrib::get_class_slot());
  if (shader_attrib->auto_shader()) {
    _auto_shader = true;
  }
  if (shader_attrib->get_flag(ShaderAttrib::F_hardware_skinning)) {
    _shader_skinning = true;
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
  if (_shader_skinning || (_auto_shader && hardware_animated_vertices &&
      !basic_shaders_only && animation.get_animation_type() == AT_panda)) {
    animation.set_hardware(4, true);

  } else if (hardware_animated_vertices &&
             animation.get_animation_type() == AT_panda &&
             new_data->get_slider_table() == (SliderTable *)NULL) {
    // Maybe we can animate the vertices with hardware.
    const TransformBlendTable *table = new_data->get_transform_blend_table();
    if (table != (TransformBlendTable *)NULL &&
        table->get_num_transforms() != 0 &&
        table->get_max_simultaneous_transforms() <= 
        get_gsg()->get_max_vertex_transforms()) {
      if (matrix_palette && 
          table->get_num_transforms() <= get_gsg()->get_max_vertex_transform_indices()) {

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
                 get_gsg()->get_max_vertex_transforms()) {
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
void StandardMunger::
munge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &vertex_data, 
                Thread *) {
  int supported_geom_rendering = get_gsg()->get_supported_geom_rendering();

  int unsupported_bits = geom->get_geom_rendering() & ~supported_geom_rendering;
  if (unsupported_bits != 0) {
    // Even beyond munging the vertex format, we have to convert the
    // Geom itself into a new primitive type the GSG can render
    // directly.
    // If we don't support a strip cut index, it might be faster to
    // just decompose it rather than draw them one by one.
    if ((unsupported_bits & Geom::GR_composite_bits) != 0 ||
        (unsupported_bits & Geom::GR_strip_cut_index) != 0) {
      // This decomposes everything in the primitive, so that if (for
      // instance) the primitive contained both strips and fans, but
      // the GSG didn't support fans, it would decompose the strips
      // too.  To handle this correctly, we'd need a separate
      // decompose_fans() and decompose_strips() call; but for now,
      // we'll just say it's good enough.  In practice, we don't have
      // any GSG's that can support strips without also supporting
      // fans.
      geom = geom->decompose();

      // Decomposing might produce an indexed Geom, so re-check the
      // unsupported bits.
      unsupported_bits = geom->get_geom_rendering() & ~supported_geom_rendering;
    }
    if ((unsupported_bits & Geom::GR_shade_model_bits) != 0) {
      // Rotate the vertices to account for different shade-model
      // expectations (e.g. SM_flat_last_vertex to
      // SM_flat_first_vertex)
      geom = geom->rotate();
    }
    if ((unsupported_bits & Geom::GR_indexed_bits) != 0) {
      // Convert indexed geometry to nonindexed geometry.
      PT(Geom) new_geom = geom->make_copy();
      new_geom->set_vertex_data(vertex_data);
      new_geom->make_nonindexed(false);
      geom = new_geom;
      vertex_data = new_geom->get_vertex_data();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StandardMunger::premunge_geom_impl
//       Access: Protected, Virtual
//  Description: Converts a Geom and/or its data as necessary.
////////////////////////////////////////////////////////////////////
void StandardMunger::
premunge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &vertex_data) {
  int supported_geom_rendering = get_gsg()->get_supported_geom_rendering();

  int unsupported_bits = geom->get_geom_rendering() & ~supported_geom_rendering;
  if (unsupported_bits != 0) {
    // Even beyond munging the vertex format, we have to convert the
    // Geom itself into a new primitive type the GSG can render
    // directly.
    // If we don't support a strip cut index, it might be faster to
    // just decompose it rather than draw them one by one.
    if ((unsupported_bits & Geom::GR_composite_bits) != 0 ||
        (unsupported_bits & Geom::GR_strip_cut_index) != 0) {
      // This decomposes everything in the primitive, so that if (for
      // instance) the primitive contained both strips and fans, but
      // the GSG didn't support fans, it would decompose the strips
      // too.  To handle this correctly, we'd need a separate
      // decompose_fans() and decompose_strips() call; but for now,
      // we'll just say it's good enough.  In practice, we don't have
      // any GSG's that can support strips without also supporting
      // fans.
      geom = geom->decompose();

      // Decomposing might produce an indexed Geom, so re-check the
      // unsupported bits.
      unsupported_bits = geom->get_geom_rendering() & ~supported_geom_rendering;
    }
    if ((unsupported_bits & Geom::GR_shade_model_bits) != 0) {
      // Rotate the vertices to account for different shade-model
      // expectations (e.g. SM_flat_last_vertex to
      // SM_flat_first_vertex)
      geom = geom->rotate();
    }
    if ((unsupported_bits & Geom::GR_indexed_bits) != 0) {
      // Convert indexed geometry to nonindexed geometry.
      PT(Geom) new_geom = geom->make_copy();
      new_geom->set_vertex_data(vertex_data);
      new_geom->make_nonindexed(false);
      geom = new_geom;
      vertex_data = new_geom->get_vertex_data();
    }
  }
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
  if (_shader_skinning != om->_shader_skinning) {
    return (int)_shader_skinning - (int)om->_shader_skinning;
  }
  if (_auto_shader != om->_auto_shader) {
    return (int)_auto_shader - (int)om->_auto_shader;
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
  if (_shader_skinning != om->_shader_skinning) {
    return (int)_shader_skinning - (int)om->_shader_skinning;
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
    munged_state = munged_state->remove_attrib(ColorAttrib::get_class_slot());
    munged_state = munged_state->remove_attrib(ColorScaleAttrib::get_class_slot());
  } else if (_munge_color_scale) {
    munged_state = munged_state->remove_attrib(ColorScaleAttrib::get_class_slot());
  }

#ifdef HAVE_CG
  if (_auto_shader) {
    CPT(RenderState) shader_state = munged_state->get_auto_shader_state();
    ShaderGenerator *shader_generator = get_gsg()->get_shader_generator();
    if (shader_generator == NULL) {
      pgraph_cat.error()
        << "auto_shader enabled, but GSG has no shader generator assigned!\n";
      return munged_state;
    }
    if (shader_state->_generated_shader == NULL) {
      // Cache the generated ShaderAttrib on the shader state.
      GeomVertexAnimationSpec spec;

      // Currently we overload this flag to request vertex animation
      // for the shader generator.
      const ShaderAttrib *sattr;
      shader_state->get_attrib_def(sattr);
      if (sattr->get_flag(ShaderAttrib::F_hardware_skinning)) {
        spec.set_hardware(4, true);
      }

      shader_state->_generated_shader = shader_generator->synthesize_shader(shader_state, spec);
    }
    munged_state = munged_state->set_attrib(shader_state->_generated_shader);
  }
#endif

  return munged_state;
}
