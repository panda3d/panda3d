/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderModule.I
 * @author rdb
 * @date 2019-03-21
 */

/**
 * Returns the stage of the shading pipeline that this shader module is
 * intended to fill.
 */
INLINE ShaderModule::Stage ShaderModule::
get_stage() const {
  return _stage;
}

/**
 * Returns the set of capabilities used by this shader.
 */
INLINE int ShaderModule::
get_used_capabilities() const {
  return _used_caps;
}

/**
 * Returns the original filename that the module was compiled from.  This may
 * be empty.
 */
INLINE const Filename &ShaderModule::
get_source_filename() const {
  return _source_filename;
}

/**
 * Sets the source filename.
 */
INLINE void ShaderModule::
set_source_filename(const Filename &filename) {
  _source_filename = filename;
}

/**
 * Returns the number of input variables in this shader stage.
 */
INLINE size_t ShaderModule::
get_num_inputs() const {
  return _inputs.size();
}

/**
 * Returns the nth input variable in this shader stage.
 */
INLINE const ShaderModule::Variable &ShaderModule::
get_input(size_t i) const {
  return _inputs[i];
}

/**
 * Returns the index of the input variable with the given name.
 */
INLINE int ShaderModule::
find_input(CPT_InternalName name) const {
  for (size_t i = 0; i < _inputs.size(); ++i) {
    if (_inputs[i].name == name) {
      return (int)i;
    }
  }
  return -1;
}

/**
 * Returns the number of output variables in this shader stage.
 */
INLINE size_t ShaderModule::
get_num_outputs() const {
  return _outputs.size();
}

/**
 * Returns the nth output variable in this shader stage.
 */
INLINE const ShaderModule::Variable &ShaderModule::
get_output(size_t i) const {
  return _outputs[i];
}

/**
 * Returns the index of the output variable with the given name.
 */
INLINE int ShaderModule::
find_output(CPT_InternalName name) const {
  for (size_t i = 0; i < _outputs.size(); ++i) {
    if (_outputs[i].name == name) {
      return (int)i;
    }
  }
  return -1;
}

/**
 * Returns the number of parameter variables in this shader stage.
 */
INLINE size_t ShaderModule::
get_num_parameters() const {
  return _parameters.size();
}

/**
 * Returns the nth parameter variable in this shader stage.
 */
INLINE const ShaderModule::Variable &ShaderModule::
get_parameter(size_t i) const {
  return _parameters[i];
}

/**
 * Returns the index of the parameter variable with the given name.
 */
INLINE int ShaderModule::
find_parameter(CPT_InternalName name) const {
  for (size_t i = 0; i < _parameters.size(); ++i) {
    if (_parameters[i].name == name) {
      return (int)i;
    }
  }
  return -1;
}
