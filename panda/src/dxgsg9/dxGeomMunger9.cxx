/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxGeomMunger9.cxx
 * @author drose
 * @date 2005-03-11
 */

#include "dxGeomMunger9.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "config_dxgsg9.h"

GeomMunger *DXGeomMunger9::_deleted_chain = nullptr;
TypeHandle DXGeomMunger9::_type_handle;

/**
 *
 */
DXGeomMunger9::
DXGeomMunger9(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 1, NT_packed_dabc, C_color),
  _texture(nullptr),
  _tex_gen(nullptr)
{
  const TextureAttrib *texture = nullptr;
  const TexGenAttrib *tex_gen = nullptr;
  state->get_attrib(texture);
  state->get_attrib(tex_gen);
  _texture = texture;
  _tex_gen = tex_gen;

  if (!gsg->get_color_scale_via_lighting()) {
    // We might need to munge the colors, if we are overriding the vertex
    // colors and the GSG can't cheat the color via lighting.

    const ColorAttrib *color_attrib;
    const ShaderAttrib *shader_attrib;
    state->get_attrib_def(shader_attrib);

    if (!shader_attrib->auto_shader() &&
        shader_attrib->get_shader() == nullptr &&
        state->get_attrib(color_attrib) &&
        color_attrib->get_color_type() != ColorAttrib::T_vertex) {

      if (color_attrib->get_color_type() == ColorAttrib::T_off) {
        _color.set(1, 1, 1, 1);
      } else {
        _color = color_attrib->get_color();
      }

      const ColorScaleAttrib *color_scale_attrib;
      if (state->get_attrib(color_scale_attrib) &&
          color_scale_attrib->has_scale()) {
        _color.componentwise_mult(color_scale_attrib->get_scale());
      }
      _munge_color = true;
      _should_munge_state = true;
    }
  }

  _filtered_texture = nullptr;
  _reffed_filtered_texture = false;
  if (texture != nullptr) {
    _filtered_texture = texture->filter_to_max(gsg->get_max_texture_stages());
    if (_filtered_texture != texture) {
      _filtered_texture->ref();
      _reffed_filtered_texture = true;
    }
  }
  // Set a callback to unregister ourselves when either the Texture or the
  // TexGen object gets deleted.
  _texture.add_callback(this);
  _tex_gen.add_callback(this);
}

/**
 *
 */
DXGeomMunger9::
~DXGeomMunger9() {
  if (_reffed_filtered_texture) {
    unref_delete(_filtered_texture);
    _reffed_filtered_texture = false;
  }

  _texture.remove_callback(this);
  _tex_gen.remove_callback(this);
}

/**
 * This callback is set to be made whenever the associated _texture or
 * _tex_gen attributes are destructed, in which case the GeomMunger is invalid
 * and should no longer be used.
 */
void DXGeomMunger9::
wp_callback(void *) {
  unregister_myself();

  if (_reffed_filtered_texture) {
    unref_delete(_filtered_texture);
    _reffed_filtered_texture = false;
  }
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) DXGeomMunger9::
munge_format_impl(const GeomVertexFormat *orig,
                  const GeomVertexAnimationSpec &animation) {
  if (dxgsg9_cat.is_debug()) {
    if (animation.get_animation_type() != AT_none) {
      dxgsg9_cat.debug()
        << "preparing animation type " << animation << " for " << *orig
        << "\n";
    }
  }
  // We have to build a completely new format that includes only the
  // appropriate components, in the appropriate order, in just one array.
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
  new_format->set_animation(animation);
  PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

  const GeomVertexColumn *vertex_type = orig->get_vertex_column();
  const GeomVertexColumn *normal_type = orig->get_normal_column();
  const GeomVertexColumn *color_type = orig->get_color_column();

  if (vertex_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_vertex(), 3, NT_float32,
       vertex_type->get_contents());
    new_format->remove_column(vertex_type->get_name());

  } else {
    // If we don't have a vertex type, not much we can do.
    return orig;
  }

  if (animation.get_animation_type() == AT_hardware &&
      animation.get_num_transforms() > 0) {
    if (animation.get_num_transforms() > 1) {
      // If we want hardware animation, we need to reserve space for the blend
      // weights.
      new_array_format->add_column
        (InternalName::get_transform_weight(), animation.get_num_transforms() - 1,
         NT_float32, C_other);
    }

    if (animation.get_indexed_transforms()) {
      // Also, if we'll be indexing into the transform table, reserve space
      // for the index.
      new_array_format->add_column
        (InternalName::get_transform_index(), 1,
         NT_packed_dcba, C_index);
    }

    // Make sure the old weights and indices are removed, just in case.
    new_format->remove_column(InternalName::get_transform_weight());
    new_format->remove_column(InternalName::get_transform_index());

    // And we don't need the transform_blend table any more.
    new_format->remove_column(InternalName::get_transform_blend());
  }

  if (normal_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_normal(), 3, NT_float32, C_normal);
    new_format->remove_column(normal_type->get_name());
  }

  if (color_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_color(), 1, NT_packed_dabc, C_color);
    new_format->remove_column(color_type->get_name());
  }

  // To support multitexture, we will need to add all of the relevant texcoord
  // types, and in the order specified by the TextureAttrib.

  // Now set up each of the active texture coordinate stages--or at least
  // those for which we're not generating texture coordinates automatically.

  if (_filtered_texture != nullptr) {
    int num_stages = _filtered_texture->get_num_on_ff_stages();
    vector_int ff_tc_index(num_stages, 0);

    // Be sure we add the texture coordinates in the right order, as specified
    // by the attrib.  To ensure this, we first walk through the stages of the
    // attrib and get the index numbers in the appropriate order.
    int si, tc_index;
    int max_tc_index = -1;
    for (si = 0; si < num_stages; ++si) {
      int tc_index = _filtered_texture->get_ff_tc_index(si);
      nassertr(tc_index < num_stages, orig);
      ff_tc_index[tc_index] = si;
      max_tc_index = std::max(tc_index, max_tc_index);
    }

    // Now walk through the texture coordinates in the order they will appear
    // on the final geometry.  For each one, get the texture coordinate name
    // from the associated stage.
    for (tc_index = 0; tc_index <= max_tc_index; ++tc_index) {
      si = ff_tc_index[tc_index];
      TextureStage *stage = _filtered_texture->get_on_ff_stage(si);
      InternalName *name = stage->get_texcoord_name();

      const GeomVertexColumn *texcoord_type = orig->get_column(name);

      if (texcoord_type != nullptr) {
        new_array_format->add_column
          (name, texcoord_type->get_num_values(), NT_float32, C_texcoord);
      } else {
        // We have to add something as a placeholder, even if the texture
        // coordinates aren't defined.
        new_array_format->add_column(name, 2, NT_float32, C_texcoord);
      }
      new_format->remove_column(name);
    }
  }

  // Now go through the remaining arrays and make sure they are tightly
  // packed.  If not, repack them.
  for (size_t i = 0; i < new_format->get_num_arrays(); ++i) {
    CPT(GeomVertexArrayFormat) orig_a = new_format->get_array(i);
    if (orig_a->count_unused_space() != 0) {
      PT(GeomVertexArrayFormat) new_a = new GeomVertexArrayFormat;
      for (int j = 0; j < orig_a->get_num_columns(); ++j) {
        const GeomVertexColumn *column = orig_a->get_column(j);
        new_a->add_column(column->get_name(), column->get_num_components(),
                          column->get_numeric_type(), column->get_contents());
      }
      new_format->set_array(i, new_a);
    }
  }

  // Make sure the FVF-style array we just built up is first in the list.
  new_format->insert_array(0, new_array_format);

  return GeomVertexFormat::register_format(new_format);
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) DXGeomMunger9::
premunge_format_impl(const GeomVertexFormat *orig) {
  // We have to build a completely new format that includes only the
  // appropriate components, in the appropriate order, in just one array.
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
  PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

  const GeomVertexColumn *vertex_type = orig->get_vertex_column();
  const GeomVertexColumn *normal_type = orig->get_normal_column();
  const GeomVertexColumn *color_type = orig->get_color_column();

  if (vertex_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_vertex(), 3, NT_float32,
       vertex_type->get_contents());
    new_format->remove_column(vertex_type->get_name());

  } else {
    // If we don't have a vertex type, not much we can do.
    return orig;
  }

  if (normal_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_normal(), 3, NT_float32, C_normal);
    new_format->remove_column(normal_type->get_name());
  }

  if (color_type != nullptr) {
    new_array_format->add_column
      (InternalName::get_color(), 1, NT_packed_dabc, C_color);
    new_format->remove_column(color_type->get_name());
  }

  // To support multitexture, we will need to add all of the relevant texcoord
  // types, and in the order specified by the TextureAttrib.

  // Now set up each of the active texture coordinate stages--or at least
  // those for which we're not generating texture coordinates automatically.

  if (_filtered_texture != nullptr) {
    int num_stages = _filtered_texture->get_num_on_ff_stages();
    vector_int ff_tc_index(num_stages, 0);

    // Be sure we add the texture coordinates in the right order, as specified
    // by the attrib.  To ensure this, we first walk through the stages of the
    // attrib and get the index numbers in the appropriate order.
    int si, tc_index;
    int max_tc_index = -1;
    for (si = 0; si < num_stages; ++si) {
      int tc_index = _filtered_texture->get_ff_tc_index(si);
      nassertr(tc_index < num_stages, orig);
      ff_tc_index[tc_index] = si;
      max_tc_index = std::max(tc_index, max_tc_index);
    }

    // Now walk through the texture coordinates in the order they will appear
    // on the final geometry.  For each one, get the texture coordinate name
    // from the associated stage.
    for (tc_index = 0; tc_index <= max_tc_index; ++tc_index) {
      si = ff_tc_index[tc_index];
      TextureStage *stage = _filtered_texture->get_on_ff_stage(si);
      InternalName *name = stage->get_texcoord_name();

      const GeomVertexColumn *texcoord_type = orig->get_column(name);

      if (texcoord_type != nullptr) {
        new_array_format->add_column
          (name, texcoord_type->get_num_values(), NT_float32, C_texcoord);
      } else {
        // We have to add something as a placeholder, even if the texture
        // coordinates aren't defined.
        new_array_format->add_column(name, 2, NT_float32, C_texcoord);
      }
      new_format->remove_column(name);
    }
  }

  // Now go through the remaining arrays and make sure they are tightly
  // packed.  If not, repack them.
  for (size_t i = 0; i < new_format->get_num_arrays(); ++i) {
    CPT(GeomVertexArrayFormat) orig_a = new_format->get_array(i);
    if (orig_a->count_unused_space() != 0) {
      PT(GeomVertexArrayFormat) new_a = new GeomVertexArrayFormat;
      for (int j = 0; j < orig_a->get_num_columns(); ++j) {
        const GeomVertexColumn *column = orig_a->get_column(j);
        new_a->add_column(column->get_name(), column->get_num_components(),
                          column->get_numeric_type(), column->get_contents());
      }
      new_format->set_array(i, new_a);
    }
  }

  // Make sure the FVF-style array we just built up is first in the list.
  new_format->insert_array(0, new_array_format);

  return GeomVertexFormat::register_format(new_format);
}

/**
 * Called to compare two GeomMungers who are known to be of the same type, for
 * an apples-to-apples comparison.  This will never be called on two pointers
 * of a different type.
 */
int DXGeomMunger9::
compare_to_impl(const GeomMunger *other) const {
  const DXGeomMunger9 *om = DCAST(DXGeomMunger9, other);
  if (_filtered_texture != om->_filtered_texture) {
    return _filtered_texture < om->_filtered_texture ? -1 : 1;
  }
  if (_tex_gen.owner_before(om->_tex_gen)) {
    return -1;
  }
  if (om->_tex_gen.owner_before(_tex_gen)) {
    return 1;
  }

  return StandardMunger::compare_to_impl(other);
}

/**
 * Called to compare two GeomMungers who are known to be of the same type, for
 * an apples-to-apples comparison.  This will never be called on two pointers
 * of a different type.
 */
int DXGeomMunger9::
geom_compare_to_impl(const GeomMunger *other) const {
  // Unlike GLGeomMunger, we do consider _filtered_texture and _tex_gen
  // important for this purpose, since they control the number and order of
  // texture coordinates we might put into the FVF.
  const DXGeomMunger9 *om = DCAST(DXGeomMunger9, other);
  if (_filtered_texture != om->_filtered_texture) {
    return _filtered_texture < om->_filtered_texture ? -1 : 1;
  }
  if (_tex_gen.owner_before(om->_tex_gen)) {
    return -1;
  }
  if (om->_tex_gen.owner_before(_tex_gen)) {
    return 1;
  }

  return StandardMunger::geom_compare_to_impl(other);
}
