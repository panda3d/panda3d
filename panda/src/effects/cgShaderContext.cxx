// Filename: cgShaderContext.cxx
// Created by:  sshodhan (20Jul04)
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

#ifdef HAVE_CG

#include "cgShaderContext.h"
TypeHandle CgShaderContext::_type_handle;

  
////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::init_cg_shader_context
//       Access: Published
//  Description: To be overridden by derived class to do appropriate
//               initialization
////////////////////////////////////////////////////////////////////
bool CgShaderContext::
init_cg_shader_context() {
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::bind
//       Access: Published
//  Description: Enable the shaders and actually send parameters
////////////////////////////////////////////////////////////////////
void CgShaderContext::
bind(GraphicsStateGuardianBase *gsg){
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::unbind
//       Access: Published
//  Description: Disable the shaders and texture parameters if any
////////////////////////////////////////////////////////////////////
void CgShaderContext::
un_bind() {
}


////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set Matrix parameters
////////////////////////////////////////////////////////////////////
void CgShaderContext::
set_param(const string &pname, CgShader::Matrix_Type m, CgShader::Transform_Type t,
  bool vert_or_frag) {
}

#endif  // HAVE_CG
