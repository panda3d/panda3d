/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStage.I
 * @author masad
 * @date 2004-07-15
 */

/**
 * Initialize the texture stage from other
 */
INLINE TextureStage::
TextureStage(const TextureStage &copy) {
  (*this) = copy;
}

/**
 * Returns the name of this texture stage
 */
INLINE const std::string &TextureStage::
get_name() const {
  return _name;
}

/**
 * Changes the name of this texture stage
 */
INLINE void TextureStage::
set_name(const std::string &name) {
  _name = name;
}

/**
 * Changes the order in which the texture associated with this stage is
 * rendered relative to the other texture stages.  When geometry is rendered
 * with multiple textures, the textures are rendered in order from the lowest
 * sort number to the highest sort number.
 *
 * Also see set_priority(), which is used to select the most important
 * textures for rendering when some must be omitted because of hardware
 * limitations.
 */
INLINE void TextureStage::
set_sort(int sort) {
  _sort = sort;

  // Update the global flag to indicate that all TextureAttribs in the world
  // must now re-sort their lists.
  _sort_seq++;

  if (_used_by_auto_shader) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * Returns the sort order of this texture stage.
 */
INLINE int TextureStage::
get_sort() const {
  return _sort;
}

/**
 * Changes the relative importance of the texture associated with this stage
 * relative to the other texture stages that are applied simultaneously.
 *
 * This is unrelated to set_sort(), which controls the order in which multiple
 * textures are applied.  The priority number is used to decide which of the
 * requested textures are to be selected for rendering when more textures are
 * requested than the hardware will support.  The highest-priority n textures
 * are selected for rendering, and then rendered in order by their sort
 * factor.
 */
INLINE void TextureStage::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all TextureAttribs in the world
  // must now re-sort their lists.
  _sort_seq++;

  if (_used_by_auto_shader) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

/**
 * Returns the priority associated with this stage.
 *
 * This is specially helpful for cards that do not support more than n stages
 * of multi-texturing.
 */
INLINE int TextureStage::
get_priority() const {
  return _priority;
}

/**
 * Indicate which set of UV's this texture stage will use.  Geometry may have
 * any number of associated UV sets, each of which must have a unique name.
 */
INLINE void TextureStage::
set_texcoord_name(InternalName *name) {
  if (name != _texcoord_name) {
    _texcoord_name = name;

    if (_used_by_auto_shader) {
      GraphicsStateGuardianBase::mark_rehash_generated_shaders();
    }
  }
}

/**
 * Indicate which set of UV's this texture stage will use.  Geometry may have
 * any number of associated UV sets, each of which must have a unique name.
 */
INLINE void TextureStage::
set_texcoord_name(const std::string &name) {
  set_texcoord_name(InternalName::get_texcoord_name(name));
}

/**
 * See set_texcoord_name.  The default is InternalName::get_texcoord().
 */
INLINE InternalName *TextureStage::
get_texcoord_name() const {
  return _texcoord_name;
}

/**
 * Returns the set of tangents this texture stage will use.  This is the same
 * as get_texcoord_name(), except that the first part is "tangent".
 */
INLINE InternalName *TextureStage::
get_tangent_name() const {
  if (_texcoord_name->get_parent() == nullptr) {
    return InternalName::get_tangent();
  } else {
    return InternalName::get_tangent_name(_texcoord_name->get_basename());
  }
}

/**
 * Returns the set of binormals this texture stage will use.  This is the same
 * as get_binormal_name(), except that the first part is "binormal".
 */
INLINE InternalName *TextureStage::
get_binormal_name() const {
  if (_texcoord_name->get_parent() == nullptr) {
    return InternalName::get_binormal();
  } else {
    return InternalName::get_binormal_name(_texcoord_name->get_basename());
  }
}

/**
 * Set the mode of this texture stage
 */
INLINE void TextureStage::
set_mode(TextureStage::Mode mode) {
  if (mode != _mode) {
    _mode = mode;

    if (_mode != M_combine) {
      _num_combine_rgb_operands = 0;
      _num_combine_alpha_operands = 0;
    }

    update_color_flags();
  }
}

/**
 * Return the mode of this stage
 */
INLINE TextureStage::Mode TextureStage::
get_mode() const {
  return _mode;
}

/**
 * Returns true if the TextureStage is relevant to the classic fixed function
 * pipeline.  This excludes texture stages such as normal mapping and the
 * like.
 */
INLINE bool TextureStage::
is_fixed_function() const {
  return (_mode < M_normal);
}

/**
 * Set the color for this stage
 */
INLINE void TextureStage::
set_color(const LColor &color) {
  _color = color;
}

/**
 * return the color for this stage
 */
INLINE LColor TextureStage::
get_color() const {
  return _color;
}

/**
 * Sets an additional factor that will scale all three r, g, b components
 * after the texture has been applied.  This is used only when the mode is
 * CM_combine.
 *
 * The only legal values are 1, 2, or 4.
 */
INLINE void TextureStage::
set_rgb_scale(int rgb_scale) {
  if (rgb_scale != _rgb_scale) {
    nassertv(rgb_scale == 1 || rgb_scale == 2 || rgb_scale == 4);
    _rgb_scale = rgb_scale;

    if (_used_by_auto_shader) {
      GraphicsStateGuardianBase::mark_rehash_generated_shaders();
    }
  }
}

/**
 * See set_rgb_scale().
 */
INLINE int TextureStage::
get_rgb_scale() const {
  return _rgb_scale;
}

/**
 * Sets an additional factor that will scale the alpha component after the
 * texture has been applied.  This is used only when the mode is CM_combine.
 *
 * The only legal values are 1, 2, or 4.
 */
INLINE void TextureStage::
set_alpha_scale(int alpha_scale) {
  if (alpha_scale != _alpha_scale) {
    nassertv(alpha_scale == 1 || alpha_scale == 2 || alpha_scale == 4);
    _alpha_scale = alpha_scale;

    if (_used_by_auto_shader) {
      GraphicsStateGuardianBase::mark_rehash_generated_shaders();
    }
  }
}

/**
 * See set_alpha_scale().
 */
INLINE int TextureStage::
get_alpha_scale() const {
  return _alpha_scale;
}

/**
 * Sets the saved_result flag.  When this is true, the output of this stage
 * will be supplied as the "last_saved_result" source for any future stages,
 * until the next TextureStage with a saved_result set true is encountered.
 *
 * This can be used to reuse the results of this texture stage as input to
 * more than one stage later in the pipeline.
 *
 * The last texture in the pipeline (the one with the highest sort value)
 * should not have this flag set.
 */
INLINE void TextureStage::
set_saved_result(bool saved_result) {
  if (saved_result != _saved_result) {
    _saved_result = saved_result;

    if (_used_by_auto_shader) {
      GraphicsStateGuardianBase::mark_rehash_generated_shaders();
    }
  }
}

/**
 * Returns the current setting of the saved_result flag.  See
 * set_saved_result().
 */
INLINE bool TextureStage::
get_saved_result() const {
  return _saved_result;
}

/**
 * Sets the tex_view_offset value.  This is used only when a special multiview
 * texture is bound to the TextureStage, and it selects the particular view of
 * the texture that is to be used.
 *
 * This value is added to the similar parameter on DisplayRegion to derive the
 * final texture view index that is selected for rendering.
 */
INLINE void TextureStage::
set_tex_view_offset(int tex_view_offset) {
  _tex_view_offset = tex_view_offset;
}

/**
 * Returns the current setting of the tex_view_offset.  See
 * set_tex_view_offset().
 */
INLINE int TextureStage::
get_tex_view_offset() const {
  return _tex_view_offset;
}

/**
 * Specifies any of the CombineMode values that represent a one-parameter
 * operation.  Specifically, this is CM_replace only.
 */
INLINE void TextureStage::
set_combine_rgb(CombineMode mode,
                CombineSource source0, CombineOperand operand0) {
  nassertv(get_expected_num_combine_operands(mode) == 1);
  nassertv(operand_valid_for_rgb(operand0));
  _mode = M_combine;
  _num_combine_rgb_operands = 1;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = CS_undefined;
  _combine_rgb_operand1 = CO_undefined;
  _combine_rgb_source2 = CS_undefined;
  _combine_rgb_operand2 = CO_undefined;

  update_color_flags();
}

/**
 * Specifies any of the CombineMode values that represent a two-parameter
 * operation.  Specifically, this is everything except for CM_replace and
 * CM_interpolate.
 */
INLINE void TextureStage::
set_combine_rgb(CombineMode mode,
                CombineSource source0, CombineOperand operand0,
                CombineSource source1, CombineOperand operand1) {
  nassertv(get_expected_num_combine_operands(mode) == 2);
  nassertv(operand_valid_for_rgb(operand0));
  nassertv(operand_valid_for_rgb(operand1));
  _mode = M_combine;
  _num_combine_rgb_operands = 2;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = source1;
  _combine_rgb_operand1 = operand1;
  _combine_rgb_source2 = CS_undefined;
  _combine_rgb_operand2 = CO_undefined;

  update_color_flags();
}

/**
 * Specifies any of the CombineMode values that represent a one-parameter
 * operation.  Specifically, this is CM_interpolate only.
 */
INLINE void TextureStage::
set_combine_rgb(CombineMode mode,
                CombineSource source0, CombineOperand operand0,
                CombineSource source1, CombineOperand operand1,
                CombineSource source2, CombineOperand operand2) {
  nassertv(get_expected_num_combine_operands(mode) == 3);
  nassertv(operand_valid_for_rgb(operand0));
  nassertv(operand_valid_for_rgb(operand1));
  nassertv(operand_valid_for_rgb(operand2));
  _mode = M_combine;
  _num_combine_rgb_operands = 3;
  _combine_rgb_mode = mode;
  _combine_rgb_source0 = source0;
  _combine_rgb_operand0 = operand0;
  _combine_rgb_source1 = source1;
  _combine_rgb_operand1 = operand1;
  _combine_rgb_source2 = source2;
  _combine_rgb_operand2 = operand2;

  update_color_flags();
}

/**
 * Get the combine_rgb_mode
 */
INLINE TextureStage::CombineMode TextureStage::
get_combine_rgb_mode() const {
  return _combine_rgb_mode;
}

/**
 * Returns the number of meaningful operands that may be retrieved via
 * get_combine_rgb_sourceN() and get_combine_rgb_operandN().
 */
INLINE int TextureStage::
get_num_combine_rgb_operands() const {
  return _num_combine_rgb_operands;
}

/**
 * Get source0 of combine_rgb_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source0() const {
  return _combine_rgb_source0;
}

/**
 * Get operand0 of combine_rgb_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand0() const {
  return _combine_rgb_operand0;
}

/**
 * Get source1 of combine_rgb_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source1() const {
  return _combine_rgb_source1;
}

/**
 * Get operand1 of combine_rgb_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand1() const {
  return _combine_rgb_operand1;
}

/**
 * Get source2 of combine_rgb_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_rgb_source2() const {
  return _combine_rgb_source2;
}

/**
 * Get operand2 of combine_rgb_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_rgb_operand2() const {
  return _combine_rgb_operand2;
}

/**
 * Specifies any of the CombineMode values that represent a one-parameter
 * operation.  Specifically, this is CM_replace only.
 */
INLINE void TextureStage::
set_combine_alpha(CombineMode mode,
                  CombineSource source0, CombineOperand operand0) {
  nassertv(get_expected_num_combine_operands(mode) == 1);
  nassertv(operand_valid_for_alpha(operand0));
  _mode = M_combine;
  _num_combine_alpha_operands = 1;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = CS_undefined;
  _combine_alpha_operand1 = CO_undefined;
  _combine_alpha_source2 = CS_undefined;
  _combine_alpha_operand2 = CO_undefined;

  update_color_flags();
}

/**
 * Specifies any of the CombineMode values that represent a two-parameter
 * operation.  Specifically, this is everything except for CM_replace and
 * CM_interpolate.
 */
INLINE void TextureStage::
set_combine_alpha(CombineMode mode,
                  CombineSource source0, CombineOperand operand0,
                  CombineSource source1, CombineOperand operand1) {
  nassertv(get_expected_num_combine_operands(mode) == 2);
  nassertv(operand_valid_for_alpha(operand0));
  nassertv(operand_valid_for_alpha(operand1));
  _mode = M_combine;
  _num_combine_alpha_operands = 2;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = source1;
  _combine_alpha_operand1 = operand1;
  _combine_alpha_source2 = CS_undefined;
  _combine_alpha_operand2 = CO_undefined;

  update_color_flags();
}

/**
 * Specifies any of the CombineMode values that represent a one-parameter
 * operation.  Specifically, this is CM_interpolate only.
 */
INLINE void TextureStage::
set_combine_alpha(CombineMode mode,
                  CombineSource source0, CombineOperand operand0,
                  CombineSource source1, CombineOperand operand1,
                  CombineSource source2, CombineOperand operand2) {
  nassertv(get_expected_num_combine_operands(mode) == 3);
  nassertv(operand_valid_for_alpha(operand0));
  nassertv(operand_valid_for_alpha(operand1));
  nassertv(operand_valid_for_alpha(operand2));
  _mode = M_combine;
  _num_combine_alpha_operands = 3;
  _combine_alpha_mode = mode;
  _combine_alpha_source0 = source0;
  _combine_alpha_operand0 = operand0;
  _combine_alpha_source1 = source1;
  _combine_alpha_operand1 = operand1;
  _combine_alpha_source2 = source2;
  _combine_alpha_operand2 = operand2;

  update_color_flags();
}

/**
 * Get combine_alpha_mode
 */
INLINE TextureStage::CombineMode TextureStage::
get_combine_alpha_mode() const {
  return _combine_alpha_mode;
}

/**
 * Returns the number of meaningful operands that may be retrieved via
 * get_combine_alpha_sourceN() and get_combine_alpha_operandN().
 */
INLINE int TextureStage::
get_num_combine_alpha_operands() const {
  return _num_combine_alpha_operands;
}

/**
 * Get source0 of combine_alpha_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source0() const {
  return _combine_alpha_source0;
}

/**
 * Get operand0 of combine_alpha_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand0() const {
  return _combine_alpha_operand0;
}

/**
 * Get source1 of combine_alpha_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source1() const {
  return _combine_alpha_source1;
}

/**
 * Get operand1 of combine_alpha_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand1() const {
  return _combine_alpha_operand1;
}

/**
 * Get source2 of combine_alpha_mode
 */
INLINE TextureStage::CombineSource TextureStage::
get_combine_alpha_source2() const {
  return _combine_alpha_source2;
}

/**
 * Get operand2 of combine_alpha_mode
 */
INLINE TextureStage::CombineOperand TextureStage::
get_combine_alpha_operand2() const {
  return _combine_alpha_operand2;
}

/**
 * Returns true if the TextureStage is affected by the setting of the current
 * ColorScaleAttrib, false otherwise.
 */
INLINE bool TextureStage::
involves_color_scale() const {
  return _involves_color_scale;
}

/**
 * Returns true if the TextureStage makes use of whatever color is specified
 * in set_color(), false otherwise.
 */
INLINE bool TextureStage::
uses_color() const {
  return _uses_color;
}

/**
 * Returns true if the TextureStage makes use of the CS_primary_color combine
 * source.
 */
INLINE bool TextureStage::
uses_primary_color() const {
  return _uses_primary_color;
}

/**
 * Returns true if the TextureStage makes use of the CS_primary_color combine
 * source.
 */
INLINE bool TextureStage::
uses_last_saved_result() const {
  return _uses_last_saved_result;
}

/**
 *
 */
INLINE bool TextureStage::
operator == (const TextureStage &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool TextureStage::
operator != (const TextureStage &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE bool TextureStage::
operator < (const TextureStage &other) const {
  return compare_to(other) < 0;
}

/**
 * Returns the default TextureStage that will be used for all texturing that
 * does not name a particular stage.  This generally handles the normal
 * single-texture case.
 */
INLINE TextureStage *TextureStage::
get_default() {
  if (_default_stage == nullptr) {
    _default_stage = new TextureStage("default");
  }
  return _default_stage;
}

/**
 * Returns a global sequence number that is incremented any time any
 * TextureStage in the world changes sort or priority.  This is used by
 * TextureAttrib to determine when it is necessary to re-sort its internal
 * array of stages.
 */
INLINE UpdateSeq TextureStage::
get_sort_seq() {
  return _sort_seq;
}

/**
 * Marks this TextureStage as having been used by the auto shader.
 */
INLINE void TextureStage::
mark_used_by_auto_shader() const {
  _used_by_auto_shader = true;
}

/**
 * Updates _uses_color, _involves_color_scale, _uses_primary_color and
 * _uses_last_saved_result appropriately.
 */
INLINE void TextureStage::
update_color_flags() {
  _involves_color_scale =
    (_mode == M_blend_color_scale ||
     (_mode == M_combine &&
      (_combine_rgb_source0 == CS_constant_color_scale ||
       _combine_rgb_source1 == CS_constant_color_scale ||
       _combine_rgb_source2 == CS_constant_color_scale ||
       _combine_alpha_source0 == CS_constant_color_scale ||
       _combine_alpha_source1 == CS_constant_color_scale ||
       _combine_alpha_source2 == CS_constant_color_scale)));

  _uses_color =
    (_mode == M_blend || _mode == M_blend_color_scale ||
     (_mode == M_combine &&
      (_combine_rgb_source0 == CS_constant ||
       _combine_rgb_source1 == CS_constant ||
       _combine_rgb_source2 == CS_constant ||
       _combine_alpha_source0 == CS_constant ||
       _combine_alpha_source1 == CS_constant ||
       _combine_alpha_source2 == CS_constant)));

  _uses_primary_color =
     (_mode == M_combine &&
      (_combine_rgb_source0 == CS_primary_color ||
       _combine_rgb_source1 == CS_primary_color ||
       _combine_rgb_source2 == CS_primary_color ||
       _combine_alpha_source0 == CS_primary_color ||
       _combine_alpha_source1 == CS_primary_color ||
       _combine_alpha_source2 == CS_primary_color));

  _uses_last_saved_result =
     (_mode == M_combine &&
      (_combine_rgb_source0 == CS_last_saved_result ||
       _combine_rgb_source1 == CS_last_saved_result ||
       _combine_rgb_source2 == CS_last_saved_result ||
       _combine_alpha_source0 == CS_last_saved_result ||
       _combine_alpha_source1 == CS_last_saved_result ||
       _combine_alpha_source2 == CS_last_saved_result));

  if (_used_by_auto_shader) {
    GraphicsStateGuardianBase::mark_rehash_generated_shaders();
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const TextureStage &ts) {
  ts.output(out);
  return out;
}
