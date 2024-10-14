/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInputBinding_impls.I
 * @author rdb
 * @date 2024-09-22
 */

/**
 *
 */
INLINE ShaderMatrixBinding::
ShaderMatrixBinding(ShaderEnums::StateMatrix input, CPT_InternalName arg,
                    bool transpose, size_t offset, size_t num_rows, size_t num_cols) :
  _input(input),
  _arg(std::move(arg)),
  _transpose(transpose),
  _offset(offset),
  _num_rows(num_rows),
  _num_cols(num_cols) {
}

/**
 *
 */
INLINE ShaderMatrixComposeBinding::
ShaderMatrixComposeBinding(ShaderEnums::StateMatrix input0, CPT_InternalName arg0,
                           ShaderEnums::StateMatrix input1, CPT_InternalName arg1,
                           bool transpose, size_t offset, size_t num_rows, size_t num_cols) :
  _input0(input0),
  _input1(input1),
  _arg0(std::move(arg0)),
  _arg1(std::move(arg1)),
  _transpose(transpose),
  _offset(offset),
  _num_rows(num_rows),
  _num_cols(num_cols) {
}

/**
 *
 */
INLINE ShaderPackedLightBinding::
ShaderPackedLightBinding(int index) :
  _index(index) {
}

/**
 *
 */
INLINE ShaderLegacyLightBinding::
ShaderLegacyLightBinding(CPT_InternalName input, ShaderEnums::StateMatrix matrix,
                         CPT_InternalName arg) :
  _input(std::move(input)),
  _matrix(matrix),
  _arg(std::move(arg)) {
}

/**
 *
 */
INLINE ShaderTextureStagesBinding::
ShaderTextureStagesBinding(Texture::TextureType desired_type, size_t count,
                           Texture *default_texture, unsigned int mode_mask) :
  _count(count),
  _default_texture(default_texture),
  _desired_type(desired_type),
  _mode_mask(mode_mask) {
}

/**
 *
 */
INLINE ShaderTextureBinding::
ShaderTextureBinding(CPT(InternalName) input, Texture::TextureType desired_type) :
  _input(std::move(input)),
  _desired_type(desired_type) {
}

/**
 *
 */
INLINE ShaderDataBinding::
ShaderDataBinding(CPT_InternalName input,
                  size_t num_elements, size_t num_rows, size_t num_cols) :
  _input(std::move(input)),
  _num_elements(num_elements),
  _num_rows(num_rows),
  _num_cols(num_cols) {
}

/**
 *
 */
INLINE ShaderAggregateBinding::
ShaderAggregateBinding(CPT_InternalName name, const ShaderType *type) {
  r_collect_members(name, type);
}
