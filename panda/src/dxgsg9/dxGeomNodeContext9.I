// Filename: dxGeomNodeContext.I
// Created by:   masad (02Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DXGeomNodeContext9::Constructor
//       Access: Public                                 
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DXGeomNodeContext9::
DXGeomNodeContext9(GeomNode *node) : GeomNodeContext(node)
{
  _num_verts = 0;
/*
  _pVB = NULL;
  _pXformed_VB = NULL;
*/  
}
