// Filename: cgShaderAttrib.I
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


////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::Constructor
//       Access: Private
//  Description: Use CgShaderAttrib::make() to construct a new
//               CgShaderAttrib object.
////////////////////////////////////////////////////////////////////
INLINE CgShaderAttrib::
CgShaderAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::is_off
//       Access: Published
//  Description: Returns true if the CgShaderAttrib is an 'off'
//               CgShaderAttrib, indicating that it should disable
//               the previous shader profiles
////////////////////////////////////////////////////////////////////
INLINE bool CgShaderAttrib::
is_off() const {
  return _cg_shader == (CgShader *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CgShaderAttrib::get_cg_shader
//       Access: Published
//  Description: If the CgShaderAttrib is not an 'off' CgShaderAttrib,
//               returns the shader that is associated.  Otherwise,
//               return NULL.
////////////////////////////////////////////////////////////////////
INLINE CgShader *CgShaderAttrib::
get_cg_shader() const {
  return _cg_shader;
}

