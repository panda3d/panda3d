// Filename: glShaderContext_src.h
// Created by: jyelon (01Sep05)
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
//     Function: GLShaderContext::valid
//       Access: Public
//  Description: Returns true if the shader is "valid", ie, if the
//               compilation was successful.  The compilation could
//               fail if there is a syntax error in the shader, or
//               if the current video card isn't shader-capable,
//               or if no shader languages are compiled into panda.
////////////////////////////////////////////////////////////////////
INLINE bool CLP(ShaderContext)::
valid() {
#ifdef HAVE_CGGL
  if (_cg_context) return true;
#endif
  return false;
}

