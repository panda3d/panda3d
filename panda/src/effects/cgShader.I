// Filename: cgShader.I
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
//  Function: CgShader::get_name
//  Access: Public
//  Description: Every CgShader object has a name
////////////////////////////////////////////////////////////////////
INLINE string CgShader::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param texture
//  Access: Public
//  Description: Store texture in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, Texture *t) {
  _cg_textures[pname] = t;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param matrix
//  Access: Public
//  Description: Store Matrix type and Transform in the map to associate 
//               with param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, Matrix_Type m, Transform_Type t) {
  CGMATRIXDEF cgm;
  cgm.matrix = m;
  cgm.transform = t;
  _cg_matrices[pname] = cgm;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 1f
//  Access: Public
//  Description: Store 1f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, float p1f) {
  _cg_params1f[pname] = p1f;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 2f
//  Access: Public
//  Description: Store 2f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector2f p2f) {
  _cg_params2f[pname] = p2f;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 3f
//  Access: Public
//  Description: Store 3f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector3f p3f) {
  _cg_params3f[pname] = p3f;
}


////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 4f
//  Access: Public
//  Description: Store 4f in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector4f p4f) {
  _cg_params4f[pname] = p4f;
}


////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 1d
//  Access: Public
//  Description: Store 1d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, double p1d) {
  _cg_params1d[pname] = p1d;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 2d
//  Access: Public
//  Description: Store 2d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector2d p2d) {
  _cg_params2d[pname] = p2d;
}

////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 3d
//  Access: Public
//  Description: Store 3d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector3d p3d) {
  _cg_params3d[pname] = p3d;
}


////////////////////////////////////////////////////////////////////
//  Function: CgShader::set_param 4d
//  Access: Public
//  Description: Store 4d in the map to associate with 
//               param name
////////////////////////////////////////////////////////////////////
INLINE void CgShader::
set_param(const string &pname, LVector4d p4d) {
  _cg_params4d[pname] = p4d;
}

