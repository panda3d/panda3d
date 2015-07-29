// Filename: shaderInput.I
// Created by: jyelon (01Sep05)
// Updated by: fperazzi, PandaSE (06Apr10)
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


////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
~ShaderInput() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, int priority) :
  _name(MOVE(name)),
  _type(M_invalid),
  _priority(priority)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, Texture *tex, int priority) :
  _name(MOVE(name)),
  _type(M_texture),
  _priority(priority),
  _value(tex)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, ParamValueBase *param, int priority) :
  _name(MOVE(name)),
  _type(M_param),
  _priority(priority),
  _value(param)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_float &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(LCAST(PN_stdfloat, vec))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2f &vec, int priority) :
  _name(MOVE(name)),
  _type(M_vector),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3f &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4f &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3f &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_double &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(LCAST(PN_stdfloat, vec))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), vec.get_z(), 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2d &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector(vec.get_x(), vec.get_y(), 0.0, 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix4d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LMatrix3d &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix4d &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LMatrix3d &mat, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_int &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase4i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase3i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const PTA_LVecBase2i &ptr, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(ptr)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase4i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), (int)vec.get_w())
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase3i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), (int)vec.get_z(), 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShaderInput::
ShaderInput(CPT_InternalName name, const LVecBase2i &vec, int priority) :
  _name(MOVE(name)),
  _type(M_numeric),
  _priority(priority),
  _stored_ptr(vec),
  _stored_vector((int)vec.get_x(), (int)vec.get_y(), 0.0, 0.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_name
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const InternalName *ShaderInput::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_value_type
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int ShaderInput::
get_value_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_priority
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int ShaderInput::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_vector
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4 &ShaderInput::
get_vector() const {
  return _stored_vector;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_ptr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Shader::ShaderPtrData &ShaderInput::
get_ptr() const {
  return _stored_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderInput::get_param
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ParamValueBase *ShaderInput::
get_param() const {
  return DCAST(ParamValueBase, _value);
}
