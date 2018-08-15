/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGeomMunger_src.cxx
 * @author drose
 * @date 2005-03-10
 */

#include "dcast.h"

TypeHandle CLP(GeomMunger)::_type_handle;

ALLOC_DELETED_CHAIN_DEF(CLP(GeomMunger));

/**
 *
 */
CLP(GeomMunger)::
CLP(GeomMunger)(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 4, NT_uint8, C_color),
  _texture(nullptr),
  _tex_gen(nullptr) {

  _flags = 0;

  if (gl_interleaved_arrays) {
    _flags |= F_interleaved_arrays;
  } else if (gl_parallel_arrays) {
    _flags |= F_parallel_arrays;
  }

  if ((_flags & F_parallel_arrays) == 0) {
    // Set a callback to unregister ourselves when either the Texture or the
    // TexGen object gets deleted.
    _texture = (const TextureAttrib *)state->get_attrib(TextureAttrib::get_class_slot());
    _tex_gen = (const TexGenAttrib *)state->get_attrib(TexGenAttrib::get_class_slot());
    _texture.add_callback(this);
    _tex_gen.add_callback(this);
  }
}

/**
 *
 */
CLP(GeomMunger)::
~CLP(GeomMunger)() {
  // We need to remove this pointer from all of the geom contexts that
  // reference this object.
  GeomContexts::iterator gci;
  for (gci = _geom_contexts.begin(); gci != _geom_contexts.end(); ++gci) {
    (*gci)->remove_munger(this);
  }
  _geom_contexts.clear();

  if ((_flags & F_parallel_arrays) == 0) {
    _texture.remove_callback(this);
    _tex_gen.remove_callback(this);
  }
}

/**
 * This callback is set to be made whenever the associated _texture or
 * _tex_gen attributes are destructed, in which case the GeomMunger is invalid
 * and should no longer be used.
 */
void CLP(GeomMunger)::
wp_callback(void *) {
  unregister_myself();
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) CLP(GeomMunger)::
munge_format_impl(const GeomVertexFormat *orig,
                  const GeomVertexAnimationSpec &animation) {
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
  new_format->set_animation(animation);

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, get_gsg(), nullptr);

#ifndef OPENGLES
  // OpenGL ES 1 does, but regular OpenGL doesn't support GL_BYTE vertices and
  // texture coordinates.
  const GeomVertexColumn *vertex_type = orig->get_vertex_column();
  if (vertex_type != nullptr &&
      (vertex_type->get_numeric_type() == NT_int8 ||
       vertex_type->get_numeric_type() == NT_uint8)) {
    int vertex_array = orig->get_array_with(InternalName::get_vertex());

    PT(GeomVertexArrayFormat) new_array_format = new_format->modify_array(vertex_array);

    // Replace the existing vertex format with the new format.
    new_array_format->add_column
      (InternalName::get_vertex(), 3, NT_int16,
       C_point, vertex_type->get_start(), vertex_type->get_column_alignment());
  }

  // Convert packed formats that OpenGL may not understand.
  for (size_t i = 0; i < orig->get_num_columns(); ++i) {
    const GeomVertexColumn *column = orig->get_column(i);
    int array = orig->get_array_with(column->get_name());

    if (column->get_numeric_type() == NT_packed_dabc &&
        !glgsg->_supports_packed_dabc) {
      // Unpack the packed ARGB color into its four byte components.
      PT(GeomVertexArrayFormat) array_format = new_format->modify_array(array);
      array_format->add_column(column->get_name(), 4, NT_uint8, C_color,
                               column->get_start(), column->get_column_alignment());

    } else if (column->get_numeric_type() == NT_packed_ufloat &&
               !glgsg->_supports_packed_ufloat) {
      // Unpack to three 32-bit floats.  (In future, should try 16-bit float)
      PT(GeomVertexArrayFormat) array_format = new_format->modify_array(array);
      array_format->add_column(column->get_name(), 3, NT_float32,
                               column->get_contents(), column->get_start(),
                               column->get_column_alignment());
    }
  }
#endif  // !OPENGLES

  const GeomVertexColumn *color_type = orig->get_color_column();
  if (color_type != nullptr &&
      color_type->get_numeric_type() == NT_packed_dabc &&
      !glgsg->_supports_packed_dabc) {
    // We need to convert the color format; OpenGL doesn't support the byte
    // order of DirectX's packed ARGB format.
    int color_array = orig->get_array_with(InternalName::get_color());

    PT(GeomVertexArrayFormat) new_array_format = new_format->modify_array(color_array);

    // Replace the existing color format with the new format.
    new_array_format->add_column
      (InternalName::get_color(), 4, NT_uint8, C_color,
       color_type->get_start(), color_type->get_column_alignment());
  }

  if (animation.get_animation_type() == AT_hardware) {
    // If we want hardware animation, we need to reserve space for the blend
    // weights.

    // Make sure the old weights and indices are removed, just in case.
    new_format->remove_column(InternalName::get_transform_weight());
    new_format->remove_column(InternalName::get_transform_index());

    // And we don't need the transform_blend table any more.
    new_format->remove_column(InternalName::get_transform_blend());

    if (animation.get_num_transforms() > 1) {
      PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;
      new_array_format->add_column
        (InternalName::get_transform_weight(), animation.get_num_transforms(),
         NT_stdfloat, C_other);

      if (animation.get_indexed_transforms()) {
        // Also, if we'll be indexing into the transform table, reserve space
        // for the index.

        // TODO: We should examine the maximum palette index so we can decide
        // whether we need 16-bit indices.  That implies saving the maximum
        // palette index, presumably in the AnimationSpec.  At the moment, I
        // don't think any existing hardware supports more than 255 indices
        // anyway.
        new_array_format->add_column
          (InternalName::get_transform_index(), animation.get_num_transforms(),
           NT_uint8, C_index);
      }

      new_format->add_array(new_array_format);
    }
  }

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(new_format);

  if ((_flags & F_parallel_arrays) != 0) {
    // Split out the interleaved array into n parallel arrays.
    new_format = new GeomVertexFormat;
    for (size_t i = 0; i < format->get_num_columns(); ++i) {
      const GeomVertexColumn *column = format->get_column(i);
      PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;
      new_array_format->add_column(column->get_name(), column->get_num_components(),
                                   column->get_numeric_type(), column->get_contents(),
                                   -1, column->get_column_alignment());
      new_format->add_array(new_array_format);
    }
    format = GeomVertexFormat::register_format(new_format);

  } else if ((_flags & F_interleaved_arrays) != 0) {
    // Combine the primary data columns into a single array.
    new_format = new GeomVertexFormat(*format);
    PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

    const GeomVertexColumn *column = format->get_vertex_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    column = format->get_normal_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    column = format->get_color_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    // Put only the used texture coordinates into the interleaved array.
    if (auto texture = _texture.lock()) {
      typedef pset<const InternalName *> UsedStages;
      UsedStages used_stages;

      int num_stages = texture->get_num_on_stages();
      for (int i = 0; i < num_stages; ++i) {
        TextureStage *stage = texture->get_on_stage(i);
        CPT(TexGenAttrib) tex_gen = _tex_gen.lock();
        if (tex_gen == nullptr || !tex_gen->has_stage(stage)) {
          InternalName *name = stage->get_texcoord_name();
          if (used_stages.insert(name).second) {
            // This is the first time we've encountered this texcoord name.
            const GeomVertexColumn *texcoord_type = format->get_column(name);

            if (texcoord_type != nullptr) {
              new_array_format->add_column
                (name, texcoord_type->get_num_values(), NT_stdfloat, C_texcoord,
                 -1, texcoord_type->get_column_alignment());
            } else {
              // We have to add something as a placeholder, even if the
              // texture coordinates aren't defined.
              new_array_format->add_column(name, 2, NT_stdfloat, C_texcoord);
            }
            new_format->remove_column(name);
          }
        }
      }
    }

    new_format->insert_array(0, new_array_format);
    format = GeomVertexFormat::register_format(new_format);
  }

  return format;
}

/**
 * Given a source GeomVertexFormat, converts it if necessary to the
 * appropriate format for rendering.
 */
CPT(GeomVertexFormat) CLP(GeomMunger)::
premunge_format_impl(const GeomVertexFormat *orig) {
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);

  CLP(GraphicsStateGuardian) *glgsg;
  DCAST_INTO_R(glgsg, get_gsg(), nullptr);

#ifndef OPENGLES
  // OpenGL ES 1 does, but regular OpenGL doesn't support GL_BYTE vertices and
  // texture coordinates.
  const GeomVertexColumn *vertex_type = orig->get_vertex_column();
  if (vertex_type != nullptr &&
      (vertex_type->get_numeric_type() == NT_int8 ||
       vertex_type->get_numeric_type() == NT_uint8)) {
    int vertex_array = orig->get_array_with(InternalName::get_vertex());

    PT(GeomVertexArrayFormat) new_array_format = new_format->modify_array(vertex_array);

    // Replace the existing vertex format with the new format.
    new_array_format->add_column
      (InternalName::get_vertex(), 3, NT_int16,
       C_point, vertex_type->get_start(), vertex_type->get_column_alignment());
  }

  // Convert packed formats that OpenGL may not understand.
  for (size_t i = 0; i < orig->get_num_columns(); ++i) {
    const GeomVertexColumn *column = orig->get_column(i);
    int array = orig->get_array_with(column->get_name());

    if (column->get_numeric_type() == NT_packed_dabc &&
        !glgsg->_supports_packed_dabc) {
      // Unpack the packed ARGB color into its four byte components.
      PT(GeomVertexArrayFormat) array_format = new_format->modify_array(array);
      array_format->add_column(column->get_name(), 4, NT_uint8, C_color,
                               column->get_start(), column->get_column_alignment());

    } else if (column->get_numeric_type() == NT_packed_ufloat &&
               !glgsg->_supports_packed_ufloat) {
      // Unpack to three 32-bit floats.  (In future, should try 16-bit float)
      PT(GeomVertexArrayFormat) array_format = new_format->modify_array(array);
      array_format->add_column(column->get_name(), 3, NT_float32,
                               column->get_contents(), column->get_start(),
                               column->get_column_alignment());
    }
  }
#endif  // !OPENGLES

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(new_format);

  if ((_flags & F_parallel_arrays) != 0) {
    // Split out the interleaved array into n parallel arrays.
    new_format = new GeomVertexFormat;
    for (size_t i = 0; i < format->get_num_columns(); ++i) {
      const GeomVertexColumn *column = format->get_column(i);
      PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;
      new_array_format->add_column(column->get_name(), column->get_num_components(),
                                   column->get_numeric_type(), column->get_contents(),
                                   -1, column->get_column_alignment());
      new_format->add_array(new_array_format);
    }
    format = GeomVertexFormat::register_format(new_format);

  } else {
    // Combine the primary data columns into a single array.  Unlike the munge
    // case, above, in the premunge case, we do this even if
    // F_interleaved_arrays is not set (unless F_parallel_arrays is set),
    // since the presumption is that you're more willing to pay the overhead
    // of doing this step at load time than you might be at run time.
    new_format = new GeomVertexFormat(*format);
    PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

    const GeomVertexColumn *column = format->get_vertex_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    column = format->get_normal_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    column = format->get_color_column();
    if (column != nullptr) {
      new_array_format->add_column
        (column->get_name(), column->get_num_components(),
         column->get_numeric_type(), column->get_contents(),
         -1, column->get_column_alignment());
      new_format->remove_column(column->get_name());
    }

    // Put only the used texture coordinates into the interleaved array.  The
    // others will be kept around, but in a parallel array.
    if (auto texture = _texture.lock()) {
      typedef pset<const InternalName *> UsedStages;
      UsedStages used_stages;

      int num_stages = texture->get_num_on_stages();
      for (int i = 0; i < num_stages; ++i) {
        TextureStage *stage = texture->get_on_stage(i);
        CPT(TexGenAttrib) tex_gen = _tex_gen.lock();
        if (tex_gen == nullptr || !tex_gen->has_stage(stage)) {
          InternalName *name = stage->get_texcoord_name();
          if (used_stages.insert(name).second) {
            // This is the first time we've encountered this texcoord name.
            const GeomVertexColumn *texcoord_type = format->get_column(name);

            if (texcoord_type != nullptr) {
              new_array_format->add_column
                (name, texcoord_type->get_num_values(), NT_stdfloat, C_texcoord,
                 -1, texcoord_type->get_column_alignment());
            } else {
              // We have to add something as a placeholder, even if the
              // texture coordinates aren't defined.
              new_array_format->add_column(name, 2, NT_stdfloat, C_texcoord);
            }
            new_format->remove_column(name);
          }
        }
      }
    }

    // Now go through the remaining arrays and make sure they are tightly
    // packed (with the column alignment restrictions).  If not, repack them.
    for (size_t i = 0; i < new_format->get_num_arrays(); ++i) {
      CPT(GeomVertexArrayFormat) orig_a = new_format->get_array(i);
      if (orig_a->count_unused_space() != 0) {
        PT(GeomVertexArrayFormat) new_a = new GeomVertexArrayFormat;
        for (int j = 0; j < orig_a->get_num_columns(); ++j) {
          const GeomVertexColumn *column = orig_a->get_column(j);
          new_a->add_column(column->get_name(), column->get_num_components(),
                            column->get_numeric_type(), column->get_contents(),
                            -1, column->get_column_alignment());
        }
        new_format->set_array(i, new_a);
      }
    }

    // Finally, insert the interleaved array first in the format.
    new_format->insert_array(0, new_array_format);
    format = GeomVertexFormat::register_format(new_format);
  }

  return format;
}

/**
 * Called to compare two GeomMungers who are known to be of the same type, for
 * an apples-to-apples comparison.  This will never be called on two pointers
 * of a different type.
 */
int CLP(GeomMunger)::
compare_to_impl(const GeomMunger *other) const {
  const CLP(GeomMunger) *om = (CLP(GeomMunger) *)other;
  if (_texture.owner_before(om->_texture)) {
    return -1;
  }
  if (om->_texture.owner_before(_texture)) {
    return 1;
  }
  if (_tex_gen.owner_before(om->_tex_gen)) {
    return -1;
  }
  if (om->_tex_gen.owner_before(_tex_gen)) {
    return 1;
  }
  if (_flags != om->_flags) {
    return _flags < om->_flags ? -1 : 1;
  }

  return StandardMunger::compare_to_impl(other);
}

/**
 * Called to compare two GeomMungers who are known to be of the same type, for
 * an apples-to-apples comparison.  This will never be called on two pointers
 * of a different type.
 */
int CLP(GeomMunger)::
geom_compare_to_impl(const GeomMunger *other) const {
  const CLP(GeomMunger) *om = (CLP(GeomMunger) *)other;
  if (_texture.owner_before(om->_texture)) {
    return -1;
  }
  if (om->_texture.owner_before(_texture)) {
    return 1;
  }
  if (_tex_gen.owner_before(om->_tex_gen)) {
    return -1;
  }
  if (om->_tex_gen.owner_before(_tex_gen)) {
    return 1;
  }
  if (_flags != om->_flags) {
    return _flags < om->_flags ? -1 : 1;
  }

  return StandardMunger::compare_to_impl(other);
}
