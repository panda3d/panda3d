// Filename: glGeomNodeContext_src.I
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
//     Function: CLP(GeomNodeContext)::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CLP(GeomNodeContext)::
CLP(GeomNodeContext)(GeomNode *node) :
  GeomNodeContext(node)
{
  _index = 0;
  DO_PSTATS_STUFF(_num_verts = 0;)
}
