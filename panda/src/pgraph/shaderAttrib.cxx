// Filename: shaderAttrib.cxx
// Created by:  sshodhan (10Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "shaderAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ShaderAttrib::_type_handle;

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
    attrib->_shader = (Shader*)NULL;
    attrib->_shader_priority = 0;
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
make() {
  static CPT(RenderAttrib) _null_attrib;
  if (_null_attrib == 0) {
    ShaderAttrib *attrib = new ShaderAttrib;
    attrib->_shader = (Shader*)NULL;
    attrib->_shader_priority = 0;
    attrib->_has_shader = false;
    _null_attrib = return_new(attrib);
  }
  return _null_attrib;
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
  result->_has_shader = true;
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
  result->_has_shader = false;
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::add_input
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
add_input(const ShaderInput *input) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  Inputs::iterator i = result->_inputs.find(input->get_name());
  if (i == result->_inputs.end()) {
    result->_inputs.insert(Inputs::value_type(input->get_name(),input));
  } else {
    i->second = input;
  }
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::clear_input
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
clear_input(const InternalName *id) const {
  ShaderAttrib *result = new ShaderAttrib(*this);
  result->_inputs.erase(id);
  return return_new(result);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ShaderAttrib
//               types to specify what the default property for a
//               ShaderAttrib of this type should be.
//
//               This should return a newly-allocated ShaderAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ShaderAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ShaderAttrib::
make_default_impl() const {
  return new ShaderAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void ShaderAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_shader = this;
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
  if (this->_has_shader != that->_has_shader) {
    return (this->_has_shader < that->_has_shader) ? -1 : 1;
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
  if (i1 != that->_inputs.end()) {
    return -1;
  }
  
  return 0;
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
      attr->_has_shader = over->_has_shader;
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
  return return_new(attr);
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

