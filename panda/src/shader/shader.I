// Filename: shader.I
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
INLINE Shader::
Visualize* Shader::get_viz(void) {
  return _viz;
}

////////////////////////////////////////////////////////////////////
//     Function: Shader::set_viz
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Shader::
set_viz(Shader::Visualize* v) {
  _viz = v;
}

////////////////////////////////////////////////////////////////////
//     Function: FrustumShader::add_frustum
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int FrustumShader::
add_frustum(LensNode* frust) {
  make_dirty();
  _frusta.push_back(frust);
  return (_frusta.size());
}

////////////////////////////////////////////////////////////////////
//     Function: FrustumShader::remove_frustum
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int FrustumShader::
remove_frustum(LensNode* frust) {
  make_dirty();
  LensVector::iterator i;
  for (i = _frusta.begin(); i != _frusta.end(); ++i) {
    if ((*i) == frust) {
      _frusta.erase(i);
      return true;
    }
  }
  return false;
}
