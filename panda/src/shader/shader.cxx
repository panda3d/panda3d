// Filename: shader.cxx
// Created by:  mike (09Jan97)
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
#include "shader.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Shader::_type_handle;
TypeHandle FrustumShader::_type_handle;

Shader::Visualize* Shader::_viz = (Shader::Visualize*)0L;

////////////////////////////////////////////////////////////////////
//     Function: Shader::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
Shader::
Shader() : Configurable(), _priority(0) {
  MemoryUsage::update_type(this, this);
}

Shader::Visualize::Visualize(void)
{
}

Shader::Visualize::~Visualize(void)
{
}

void Shader::Visualize::DisplayTexture(PT(Texture)&, Shader*)
{
}

void Shader::Visualize::DisplayPixelBuffer(PT(PixelBuffer)&, Shader*)
{
}

void Shader::Visualize::Flush(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::apply
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Shader::
apply(Node *, const AllAttributesWrapper &,
      const AllTransitionsWrapper &,
      GraphicsStateGuardian *)
{
  if (is_dirty()) config();
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::pre_apply
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Shader::
pre_apply(Node *, const AllAttributesWrapper &,
      const AllTransitionsWrapper &,
      GraphicsStateGuardian *)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::set_priority
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Shader::
set_priority(int priority)
{
  _priority = priority;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::get_priority
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Shader::
get_priority(void) const
{
  return _priority;
}


////////////////////////////////////////////////////////////////////
//     Function: Shader::set_multipass
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Shader::
set_multipass(bool on)
{
  _multipass_on = on;
}





