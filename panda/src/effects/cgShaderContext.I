// Filename: cgShaderContext.I
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



////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::Constructor
//       Access: Published
//  Description: Use CgShaderContext() to construct a new
//               CgShaderContext object.
////////////////////////////////////////////////////////////////////
INLINE CgShaderContext::
CgShaderContext(PT(CgShader) cg_shader) :
  _cg_shader(cg_shader) {
}


////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 1f parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const float value, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 1d parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const double value, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 2f parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const float value1, const float value2,
  bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 2d parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const double value1, const double value2,
  bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 3f parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const float value1, const float value2,
  const float value3, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 3d parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const double value1, const double value2, 
  const double value3, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 4f parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const float value1, const float value2, 
  const float value3, const float value4, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set 4d parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, const double value1, const double value2,
  const double value3, const double value4, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::set_param
//       Access: Published
//  Description: Overloaded version to set Texture parameters
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
set_param(const string &pname, Texture *t , bool vert_or_frag, GraphicsStateGuardianBase *gsg) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::enable_texture_param
//       Access: Published
//  Description: Enable a Texture parameter
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
enable_texture_param(const string &pname, bool vert_or_frag) {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderContext::enable_texture_param
//       Access: Published
//  Description: Disable a Texture parameter
////////////////////////////////////////////////////////////////////
INLINE void CgShaderContext::
disable_texture_param(const string &pname, bool vert_or_frag) {
}



