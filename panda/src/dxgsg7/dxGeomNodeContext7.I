// Filename: dxGeomNodeContext7.I
// Created by:  drose (12Jun01)
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
//     Function: DXGeomNodeContext7::Constructor
//       Access: Public                                 
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DXGeomNodeContext7::
DXGeomNodeContext7(GeomNode *node) : GeomNodeContext(node)
{
  _num_verts = 0;
  _pVB = NULL;
  _pXformed_VB = NULL;
}
