// Filename: texMatrixAttrib.I
// Created by:  drose (14Mar02)
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
//     Function: TexMatrixAttrib::Constructor
//       Access: Protected
//  Description: Use TexMatrixAttrib::make() to construct a new
//               TexMatrixAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexMatrixAttrib::
TexMatrixAttrib() {
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::Copy Constructor
//       Access: Protected
//  Description: Use TexMatrixAttrib::make() to construct a new
//               TexMatrixAttrib object.
////////////////////////////////////////////////////////////////////
INLINE TexMatrixAttrib::
TexMatrixAttrib(const TexMatrixAttrib &copy) :
  _stages(copy._stages)
{
}
