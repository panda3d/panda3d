/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderInput.I
 * @author jyelon
 * @date 2005-09-01
 * @author fperazzi, PandaSE
 * @date 2010-04-06
 */

/**
 *
 */
INLINE ShaderInput::
~ShaderInput() {
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, int priority) :
  _name(MOVE(name)),
  _type(M_invalid),
  _priority(priority)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, int priority) :
  _name(MOVE(name)),
  _type(M_texture),
  _priority(priority),
  _value(tex)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, ParamValueBase *param, int priority) :
  _name(MOVE(name)),
  _type(M_param),
  _priority(priority),
  _value(param)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, ShaderBuffer *buf, int priority) :
  _name(MOVE(name)),
  _type(M_buffer),
  _priority(priority),
  _value(buf)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_float &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(LCAST(PN_stdfloat, vec))
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4f &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3f &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_double &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(LCAST(PN_stdfloat, vec))
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4d &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3d &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_int &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), (int)vec.get_w())
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), 0.0)
{
}

/**
 *
 */
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), 0.0, 0.0)
{
}

/**
 *
 */
INLINE const InternalName *ShaderInput::
get_name() const {
  return _name;
}

/**
 *
 */
INLINE int ShaderInput::
get_value_type() const {
  return _type;
}

/**
 *
 */
INLINE int ShaderInput::
get_priority() const {
  return _priority;
}

/**
 *
 */
INLINE const LVecBase4 &ShaderInput::
get_vector() const {
  return _stored_vector;
}

/**
 *
 */
INLINE const Shader::ShaderPtrData &ShaderInput::
get_ptr() const {
  return _stored_ptr;
}

/**
 *
 */
INLINE ParamValueBase *ShaderInput::
get_param() const {
  return DCAST(ParamValueBase, _value);
}
