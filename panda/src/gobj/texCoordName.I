// Filename: texCoordName.I
// Created by:  masad (15Jul04)
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
//     Function: TexCoordName::get_name
//       Access: Published
//  Description: Return the name string of this texcoordname
////////////////////////////////////////////////////////////////////
INLINE const string &TexCoordName::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: TexCoordName::get_default
//       Access: Published, Static
//  Description: Returns the default TexCoordName that will be used
//               for all geometry that does not give a particular name
//               for its UV's
////////////////////////////////////////////////////////////////////
INLINE const TexCoordName *TexCoordName::
get_default() {
  if (_default_name == (TexCoordName *)NULL) {
    _default_name = TexCoordName::make("default");
  }
  return _default_name;
}

INLINE ostream &
operator << (ostream &out, const TexCoordName &tcn) {
  tcn.output(out);
  return out;
}
