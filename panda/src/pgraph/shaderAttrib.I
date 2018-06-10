/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderAttrib.I
 * @author sshodhan
 * @date 2004-07-10
 * @author weifengh, PandaSE
 * @date 2010-04-15
 */

/**
 *
 */
INLINE ShaderAttrib::
ShaderAttrib() :
  _shader(nullptr),
  _shader_priority(0),
  _auto_shader(false),
  _has_shader(false),
  _flags(0),
  _has_flags(0),
  _instance_count(0),
  _auto_normal_on(false),
  _auto_glow_on(false),
  _auto_gloss_on(false),
  _auto_ramp_on(false),
  _auto_shadow_on(false)
{
}

/**
 *
 */
INLINE ShaderAttrib::
ShaderAttrib(const ShaderAttrib &copy) :
  _shader(copy._shader),
  _shader_priority(copy._shader_priority),
  _auto_shader(copy._auto_shader),
  _has_shader(copy._has_shader),
  _flags(copy._flags),
  _has_flags(copy._has_flags),
  _instance_count(copy._instance_count),
  _auto_normal_on(copy._auto_normal_on),
  _auto_glow_on(copy._auto_glow_on),
  _auto_gloss_on(copy._auto_gloss_on),
  _auto_ramp_on(copy._auto_ramp_on),
  _auto_shadow_on(copy._auto_shadow_on),
  _inputs(copy._inputs)
{
}

/**
 * If true, the shader field of this attribute overrides the shader field of
 * the parent attribute.
 */
INLINE bool ShaderAttrib::
has_shader() const {
  return _has_shader;
}

/**
 * If true, then this ShaderAttrib does not contain an explicit shader -
 * instead, it requests the automatic generation of a shader.
 */
INLINE bool ShaderAttrib::
auto_shader() const {
  return _auto_shader;
}

/**
 *
 */
INLINE int ShaderAttrib::
get_shader_priority() const {
  return _shader_priority;
}

/**
 * Returns the number of geometry instances.  A value of 0 means not to use
 * instancing at all.
 */
INLINE int ShaderAttrib::
get_instance_count() const {
  return _instance_count;
}

/**
 *
 */
INLINE bool ShaderAttrib::
get_flag(int index) const {
  return (_flags & (1<<index)) ? true:false;
}

/**
 * Returns true if there is a ShaderInput of the given name.
 */
bool ShaderAttrib::
has_shader_input(CPT_InternalName id) const {
  return (_inputs.find(id) != _inputs.end());
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_float &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_double &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}


/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LVecBase2 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LVecBase2 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LMatrix4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const PTA_LMatrix3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LMatrix4 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const LMatrix3 &v, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), v, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, Texture *tex, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), tex, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, const NodePath &np, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), np, priority));
}

/**
 *
 */
INLINE CPT(RenderAttrib) ShaderAttrib::
set_shader_input(CPT_InternalName id, double n1, double n2, double n3, double n4, int priority) const {
  return set_shader_input(ShaderInput(std::move(id), LVecBase4((PN_stdfloat)n1, (PN_stdfloat)n2, (PN_stdfloat)n3, (PN_stdfloat)n4), priority));
}

INLINE bool ShaderAttrib::
auto_normal_on() const {
  return _auto_normal_on;
}

/**
 *
 */
INLINE bool ShaderAttrib::
auto_glow_on() const {
  return _auto_glow_on;
}

/**
 *
 */
INLINE bool ShaderAttrib::
auto_gloss_on() const {
  return _auto_gloss_on;
}

/**
 *
 */
INLINE bool ShaderAttrib::
auto_ramp_on() const {
  return _auto_ramp_on;
}

/**
 *
 */
INLINE bool ShaderAttrib::
auto_shadow_on() const {
  return _auto_shadow_on;
}
