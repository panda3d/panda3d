// Filename: casterShader.I
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
int CasterShader::add_caster(NamedNode* node)
{
  make_dirty();
  _casters.push_back(node);
  return (_casters.size());
}

////////////////////////////////////////////////////////////////////
//     Function: CasterShader::remove_caster
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int CasterShader::remove_caster(NamedNode* node)
{
  make_dirty();
  NamedNodeVector::iterator i;
  for (i = _casters.begin(); i != _casters.end(); ++i) {
    if ((*i) == node) {
      _casters.erase(i);
      return true;
    }
  }
  return false;
}
