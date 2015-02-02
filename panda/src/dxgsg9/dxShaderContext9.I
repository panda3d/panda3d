// Filename: dxShaderContext9.i
// Created by: aignacio (Jan06)
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


////////////////////////////////////////////////////////////////////
//     Function: DXShaderContext9::valid
//       Access: Public
//  Description: Returns true if the shader is "valid", ie, if the
//               compilation was successful.  The compilation could
//               fail if there is a syntax error in the shader, or
//               if the current video card isn't shader-capable,
//               or if no shader languages are compiled into panda.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
valid(GSG *gsg) {
#ifdef HAVE_CG
  return (_cg_program != 0);
#else
  return false;
#endif
}
