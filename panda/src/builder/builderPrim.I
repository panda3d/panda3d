// Filename: builderPrim.I
// Created by:  drose (11Jan01)
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
//     Function: BuilderPrim::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrim::
BuilderPrim() {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrim::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrim::
BuilderPrim(const BuilderPrim &copy) :
  BuilderPrimTempl<BuilderVertex>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrim::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrim &BuilderPrim::
operator = (const BuilderPrim &copy) {
  BuilderPrimTempl<BuilderVertex>::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimI::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrimI::
BuilderPrimI() {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimI::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrimI::
BuilderPrimI(const BuilderPrimI &copy) :
  BuilderPrimTempl<BuilderVertexI>(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimI::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BuilderPrimI &BuilderPrimI::
operator = (const BuilderPrimI &copy) {
  BuilderPrimTempl<BuilderVertexI>::operator = (copy);
  return *this;
}
