// Filename: directRenderTraverser.I
// Created by:  drose (02Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
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
//     Function: DirectRenderTraverser::set_view_frustum_cull
//       Access: Public
//  Description: Sets the flag that indicates whether view-frustum
//               culling should be performed during this traversal.
//               This is normally true.
////////////////////////////////////////////////////////////////////
INLINE void DirectRenderTraverser::
set_view_frustum_cull(bool flag) {
  _view_frustum_cull = flag;
}
