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
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ShaderAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ShaderAttrib object suitable for
//               process the indicated geometry with shaders
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
make(ShaderMode *shader_mode) {
  ShaderAttrib *attrib = new ShaderAttrib;
  attrib->_shader_mode = shader_mode;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new ShaderAttrib object suitable for
//               rendering geometry with no shader interference
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShaderAttrib::
make_off() {
  ShaderAttrib *attrib = new ShaderAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void ShaderAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_shader(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ShaderAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ShaderAttrib
//               types to specify what the default property for a
//               TexGenAttrib of this type should be.
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
  const ShaderAttrib *cgsa;
  DCAST_INTO_R(cgsa, other, 0);
  
  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  if (_shader_mode != cgsa->_shader_mode) {
    return _shader_mode < cgsa->_shader_mode ? -1 : 1;
  }
  return 0;
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

