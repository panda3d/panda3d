// Filename: shaderAttrib.cxx
// Created by:  sshodhan (10Jul04)
// Updated by:  fperazzi, PandaSE (06Apr10) (added more overloads
//   for set_shader_input)
// Updated by: weifengh, PandaSE(15Apr10) (added overload for
//   set_shader_auto)
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

#include "pandabase.h"
#include "shaderAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "nodePath.h"

TypeHandle ShaderAttrib::_type_handle;
int ShaderAttrib::_attrib_slot;

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new ShaderAttrib object that disables
//               the use of shaders (it does not clear out all shader
//               data, however.)
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
make_off() {
  static CPT(RenderAttrib) _off_attrib;
  if (_off_attrib == 0) {
    ShaderAttrib *attrib = new ShaderAttrib;
    attrib->_has_shader = true;
    _off_attrib = return_new(attrib);
  }
  return _off_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ShaderAttrib object with nothing
//               set.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
make(const Shader *shader) {
  static CPT(RenderAttrib) _null_attrib;
  if (_null_attrib == 0) {
    ShaderAttrib *attrib = new ShaderAttrib;
    _null_attrib = return_new(attrib);
  }

  if (shader == NULL) {
    return _null_attrib;
  } else {
    return DCAST(ShaderAttrib, _null_attrib)->set_shader(shader);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
make_default() {
  return return_new(new ShaderAttrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_shader
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_shader(const Shader *s, int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = s;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_has_shader = true;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_shader_off
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_shader_off(int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = NULL;
  result->_shader_priority = priority;
  result->_auto_shader = false;
  result->_auto_normal_on = false;
  result->_auto_glow_on = false;
  result->_auto_gloss_on = false;
  result->_auto_ramp_on = false;
  result->_auto_shadow_on = false;

  result->_has_shader = true;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_shader_auto
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_shader_auto(int priority) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = NULL;
  result->_shader_priority = priority;
  result->_auto_shader = true;
  result->_has_shader = true;
  result->_auto_normal_on = true;
  result->_auto_glow_on = true;
  result->_auto_gloss_on = true;
  result->_auto_ramp_on = true;
  result->_auto_shadow_on = true;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_shader_auto
//       Access: Published
//  Description: Set auto shader with bitmask to customize use,
//  e.g., to keep normal, glow, etc., on or off
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_shader_auto(BitMask32 shader_switch, int priority) const {

  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = NULL;
  result->_shader_priority = priority;
  result->_auto_shader = true;
  result->_has_shader = true;
  result->_auto_normal_on = shader_switch.get_bit(Shader::bit_AutoShaderNormal);
  result->_auto_glow_on = shader_switch.get_bit(Shader::bit_AutoShaderGlow);
  result->_auto_gloss_on = shader_switch.get_bit(Shader::bit_AutoShaderGloss);
  result->_auto_ramp_on = shader_switch.get_bit(Shader::bit_AutoShaderRamp);
  result->_auto_shadow_on = shader_switch.get_bit(Shader::bit_AutoShaderShadow);

  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_shader
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_shader() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_shader = NULL;
  result->_shader_priority = 0;
  result->_auto_shader = false;
  result->_has_shader = false;
  result->_auto_normal_on = false;
  result->_auto_glow_on = false;
  result->_auto_gloss_on = false;
  result->_auto_ramp_on = false;
  result->_auto_shadow_on = false;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_flag
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_flag(int flag, bool value) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  int bit = 1<<flag;
  if (value) {
    result->_flags |= bit;
  } else {
    result->_flags &= ~bit;
  }
  result->_has_flags |= bit;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_flag
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_flag(int flag) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  int bit = 1<<flag;
  result->_flags &= ~bit;
  result->_has_flags &= ~bit;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_shader_input
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_shader_input(const ShaderInput *input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  Inputs::iterator i = result->_inputs.find(input->get_name());
  if (i == result->_inputs.end()) {
    result->_inputs.insert(Inputs::value_type(input->get_name(), input));
  } else {
    i->second = input;
  }
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::set_instance_count
//       Access: Published
//  Description: Sets the geometry instance count. Do not confuse
//               this with instanceTo, which is used for animation
//               instancing, and has nothing to do with this.
//               A value of 0 means not to use instancing at all.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
set_instance_count(int instance_count) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_instance_count = instance_count;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_shader_input
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const InternalName *id) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.erase(id);
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_shader_input
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_shader_input(const string &id) const {
  return clear_shader_input(InternalName::make(id));
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_all_shader_inputs
//       Access: Published
//  Description: Clears all the shader inputs on the attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_all_shader_inputs() const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.clear();
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input
//       Access: Published
//  Description: Returns the ShaderInput of the given name.  If
//               no such name is found, this function does not return
//               NULL --- it returns the "blank" ShaderInput.
////////////////////////////////////////////////////////////////////
const ShaderInput *ShaderAttrib::
get_shader_input(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    return ShaderInput::get_blank();
  } else {
    return (*i).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input
//       Access: Published
//  Description: Returns the ShaderInput of the given name.  If
//               no such name is found, this function does not return
//               NULL --- it returns the "blank" ShaderInput.
////////////////////////////////////////////////////////////////////
const ShaderInput *ShaderAttrib::
get_shader_input(const string &id) const {
  return get_shader_input(InternalName::make(id));
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input_nodepath
//       Access: Published
//  Description: Returns the ShaderInput as a nodepath.  Assertion
//               fails if there is none, or if it is not a nodepath.
////////////////////////////////////////////////////////////////////
const NodePath &ShaderAttrib::
get_shader_input_nodepath(const InternalName *id) const {
  static NodePath resfail;
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return resfail;
  } else {
    const ShaderInput *p = (*i).second;
    if (p->get_value_type() != ShaderInput::M_nodepath) {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a nodepath.\n";
      nassert_raise(strm.str());
      return resfail;
    }
    return p->get_nodepath();
  }

  // Satisfy compiler.
  return resfail;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input_vector
//       Access: Published
//  Description: Returns the ShaderInput as a vector.  Assertion
//               fails if there is none, or if it is not a vector.
////////////////////////////////////////////////////////////////////
LVecBase4 ShaderAttrib::
get_shader_input_vector(InternalName *id) const {
  static LVecBase4 resfail(0,0,0,0);
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return resfail;
  } else {
    const ShaderInput *p = (*i).second;

    if (p->get_value_type() == ShaderInput::M_vector) {
      return p->get_vector();

    } else if (p->get_value_type() == ShaderInput::M_numeric && p->get_ptr()._size <= 4) {
      const Shader::ShaderPtrData &ptr = p->get_ptr();

      switch (ptr._type) {
      case Shader::SPT_float:
        {
          LVector4f vectorf;
          memcpy(&vectorf[0], ptr._ptr, sizeof(float) * ptr._size);
          return LCAST(PN_stdfloat, vectorf);
        }
      case Shader::SPT_double:
        {
          LVector4d vectord;
          memcpy(&vectord[0], ptr._ptr, sizeof(double) * ptr._size);
          return LCAST(PN_stdfloat, vectord);
        }
      default:
       {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain floating-point data.\n";
          nassert_raise(strm.str());
          return resfail;
        }
      }

    } else if (p->get_value_type() == ShaderInput::M_param) {
      // Temporary solution until the new param system
      ParamValueBase *param = p->get_param();
      if (param != NULL && param->is_of_type(ParamVecBase4::get_class_type())) {
        return ((const ParamVecBase4 *) param)->get_value();
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a vector.\n";
    nassert_raise(strm.str());
    return resfail;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input_ptr
//       Access: Published
//  Description: Returns the ShaderInput as a ShaderPtrData struct.
//               Assertion fails if there is none. or if it is not
//               a PTA(double/float)
////////////////////////////////////////////////////////////////////
const Shader::ShaderPtrData *ShaderAttrib::
get_shader_input_ptr(const InternalName *id) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return NULL;
  } else {
    const ShaderInput *p = (*i).second;
    if (p->get_value_type() != ShaderInput::M_numeric &&
        p->get_value_type() != ShaderInput::M_vector) {
      ostringstream strm;
      strm << "Shader input " << id->get_name() << " is not a PTA(float/double) type.\n";
      nassert_raise(strm.str());
      return NULL;
    }
    return &(p->get_ptr());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input_texture
//       Access: Published
//  Description: Returns the ShaderInput as a texture.  Assertion
//               fails if there is none, or if it is not a texture.
//
//               If sampler is not NULL, the sampler state to use
//               for this texture is assigned to it.
////////////////////////////////////////////////////////////////////
Texture *ShaderAttrib::
get_shader_input_texture(const InternalName *id, SamplerState *sampler) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return NULL;
  } else {
    const ShaderInput *p = (*i).second;
    if (p->get_value_type() != ShaderInput::M_texture &&
        p->get_value_type() != ShaderInput::M_texture_sampler) {
      ostringstream strm;
      strm <<  "Shader input " << id->get_name() << " is not a texture.\n";
      nassert_raise(strm.str());
      return NULL;
    }
    if (sampler != NULL) {
      *sampler = p->get_sampler();
    }
    return p->get_texture();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader_input_matrix
//       Access: Published
//  Description: Returns the ShaderInput as a matrix.  Assertion
//               fails if there is none, or if it is not a matrix
//               or NodePath.
////////////////////////////////////////////////////////////////////
const LMatrix4 &ShaderAttrib::
get_shader_input_matrix(const InternalName *id, LMatrix4 &matrix) const {
  Inputs::const_iterator i = _inputs.find(id);
  if (i == _inputs.end()) {
    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not present.\n";
    nassert_raise(strm.str());
    return LMatrix4::ident_mat();
  } else {
    const ShaderInput *p = (*i).second;

    if (p->get_value_type() == ShaderInput::M_nodepath) {
      const NodePath &np = p->get_nodepath();
      nassertr(!np.is_empty(), LMatrix4::ident_mat());
      return np.get_transform()->get_mat();

    } else if (p->get_value_type() == ShaderInput::M_numeric && p->get_ptr()._size == 16) {
      const Shader::ShaderPtrData &ptr = p->get_ptr();

      switch (ptr._type) {
        case Shader::SPT_float: {
          LMatrix4f matrixf;
          memcpy(&matrixf(0, 0), ptr._ptr, sizeof(float) * 16);
          matrix = LCAST(PN_stdfloat, matrixf);
          return matrix;
        }
        case Shader::SPT_double: {
          LMatrix4d matrixd;
          memcpy(&matrixd(0, 0), ptr._ptr, sizeof(double) * 16);
          matrix = LCAST(PN_stdfloat, matrixd);
          return matrix;
        }
        default: {
          ostringstream strm;
          strm << "Shader input " << id->get_name() << " does not contain floating-point data.\n";
          nassert_raise(strm.str());
          return LMatrix4::ident_mat();
        }
      }
    }

    ostringstream strm;
    strm << "Shader input " << id->get_name() << " is not a NodePath or LMatrix4.\n";
    nassert_raise(strm.str());
    return LMatrix4::ident_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_shader
//       Access: Published
//  Description: Returns the shader object associated with the node.
//               If get_override returns true, but get_shader
//               returns NULL, that means that this attribute should
//               disable the shader.
////////////////////////////////////////////////////////////////////
const Shader *ShaderAttrib::
get_shader() const {
  return _shader;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ShaderAttrib
//               types to return a unique number indicating whether
//               this ShaderAttrib is equivalent to the other one.
//
//               This should return 0 if the two ShaderAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ShaderAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ShaderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShaderAttrib *that;
  DCAST_INTO_R(that, other, 0);

  if (this->_shader != that->_shader) {
    return (this->_shader < that->_shader) ? -1 : 1;
  }
  if (this->_shader_priority != that->_shader_priority) {
    return (this->_shader_priority < that->_shader_priority) ? -1 : 1;
  }
  if (this->_auto_shader != that->_auto_shader) {
    return (this->_auto_shader < that->_auto_shader) ? -1 : 1;
  }
  if (this->_has_shader != that->_has_shader) {
    return (this->_has_shader < that->_has_shader) ? -1 : 1;
  }
  if (this->_flags != that->_flags) {
    return (this->_flags < that->_flags) ? -1 : 1;
  }
  if (this->_has_flags != that->_has_flags) {
    return (this->_has_flags < that->_has_flags) ? -1 : 1;
  }
  if (this->_instance_count != that->_instance_count) {
    return (this->_instance_count < that->_instance_count) ? -1 : 1;
  }
  if (this->_auto_normal_on != that->_auto_normal_on) {
    return (this->_auto_normal_on < that->_auto_normal_on) ? -1 : 1;
  }
  if (this->_auto_glow_on != that->_auto_glow_on) {
    return (this->_auto_glow_on < that->_auto_glow_on) ? -1 : 1;
  }
  if (this->_auto_gloss_on != that->_auto_gloss_on) {
    return (this->_auto_gloss_on < that->_auto_gloss_on) ? -1 : 1;
  }
  if (this->_auto_ramp_on != that->_auto_ramp_on) {
    return (this->_auto_ramp_on < that->_auto_ramp_on) ? -1 : 1;
  }
  if (this->_auto_shadow_on != that->_auto_shadow_on) {
    return (this->_auto_shadow_on < that->_auto_shadow_on) ? -1 : 1;
  }

  Inputs::const_iterator i1 = this->_inputs.begin();
  Inputs::const_iterator i2 = that->_inputs.begin();
  while ((i1 != this->_inputs.end()) && (i2 != that->_inputs.end())) {
    if (i1->second != i2->second) {
      return (i1->second < i2->second) ? -1 : 1;
    }
    ++i1;
    ++i2;
  }
  if (i1 != this->_inputs.end()) {
    return 1;
  }
  if (i2 != that->_inputs.end()) {
    return -1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t ShaderAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = pointer_hash::add_hash(hash, _shader);
  hash = int_hash::add_hash(hash, _shader_priority);
  hash = int_hash::add_hash(hash, (int)_auto_shader);
  hash = int_hash::add_hash(hash, (int)_has_shader);
  hash = int_hash::add_hash(hash, _flags);
  hash = int_hash::add_hash(hash, _has_flags);
  hash = int_hash::add_hash(hash, _instance_count);
  hash = int_hash::add_hash(hash, (int)_auto_normal_on);
  hash = int_hash::add_hash(hash, (int)_auto_glow_on);
  hash = int_hash::add_hash(hash, (int)_auto_gloss_on);
  hash = int_hash::add_hash(hash, (int)_auto_shadow_on);

  Inputs::const_iterator ii;
  for (ii = _inputs.begin(); ii != _inputs.end(); ++ii) {
    hash = pointer_hash::add_hash(hash, (*ii).first);
    hash = pointer_hash::add_hash(hash, (*ii).second);
  }

  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::compose_impl
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
compose_impl(const RenderAttrib *other) const {
  ShaderAttrib *attr = new ShaderAttrib(*this);
  const ShaderAttrib *over;
  DCAST_INTO_R(over, other, 0);
  // Update the shader portion.
  if (over->_has_shader) {
    if ((attr->_has_shader == false) ||
        (over->_shader_priority >= attr->_shader_priority)) {
      attr->_shader = over->_shader;
      attr->_shader_priority = over->_shader_priority;
      attr->_auto_shader = over->_auto_shader;
      attr->_has_shader = over->_has_shader;
      attr->_auto_normal_on = over->_auto_normal_on;
      attr->_auto_glow_on = over->_auto_glow_on;
      attr->_auto_gloss_on = over->_auto_gloss_on;
      attr->_auto_ramp_on = over->_auto_ramp_on;
      attr->_auto_shadow_on = over->_auto_shadow_on;
    }
  }
  // Update the shader-data portion.
  Inputs::const_iterator iover;
  for (iover=over->_inputs.begin(); iover!=over->_inputs.end(); ++iover) {
    const InternalName *id = (*iover).first;
    const ShaderInput *dover = (*iover).second;
    Inputs::iterator iattr = attr->_inputs.find(id);
    if (iattr == attr->_inputs.end()) {
      attr->_inputs.insert(Inputs::value_type(id,dover));
    } else {
      const ShaderInput *dattr = (*iattr).second;
      if (dattr->get_priority() <= dover->get_priority()) {
        iattr->second = iover->second;
      }
    }
  }
  // Just copy the instance count.
  attr->_instance_count = over->_instance_count;
  // Update the flags.
  attr->_flags &= ~(over->_has_flags);
  attr->_flags |= over->_flags;
  attr->_has_flags |= (over->_has_flags);
  return return_new(attr);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::get_auto_shader_attrib_impl
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
get_auto_shader_attrib_impl(const RenderState *state) const {
  // For a ShaderAttrib, we only need to preserve the auto-shader
  // flags.  Custom shaders, and custom shader inputs, aren't relevant
  // to the shader generator.
  ShaderAttrib *attrib = new ShaderAttrib;
  attrib->_auto_shader = _auto_shader;
  attrib->_has_shader = _has_shader;
  attrib->_auto_normal_on = _auto_normal_on;
  attrib->_auto_glow_on = _auto_glow_on;
  attrib->_auto_gloss_on = _auto_gloss_on;
  attrib->_auto_ramp_on = _auto_ramp_on;
  attrib->_auto_shadow_on = _auto_shadow_on;
  attrib->_flags = _flags;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a Shader object
////////////////////////////////////////////////////////////////////
void ShaderAttrib::
register_with_read_factory() {
  // IMPLEMENT ME
}

