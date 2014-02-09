// Filename: meshDrawer.I
// Created by:  treeform (19dec08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "lpoint2.h"

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::Constructor
//       Access: Published
//  Description: Creates the MeshDrawer low level system.
////////////////////////////////////////////////////////////////////
INLINE MeshDrawer::
MeshDrawer() {
  _root = NodePath("MeshDrawer");
  _at_start = 0;
  _bv = NULL;
  _vertex = NULL;
  _normal = NULL;
  _uv = NULL;
  _color = NULL;
  _budget = 5000;
  _vdata = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::Destructor
//       Access: Published
//  Description: Destroys the MeshDrawer low level system.
////////////////////////////////////////////////////////////////////
INLINE MeshDrawer::
~MeshDrawer() {
  _root.remove_node();
  if (_vertex != NULL) delete _vertex;
  if (_normal != NULL) delete _normal;
  if (_uv != NULL)     delete _uv;
  if (_color != NULL)  delete _color;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::get_root
//       Access: Published
//  Description: Returns the root NodePath.  You should use this node
//               to reparent mesh drawer onto the scene
//               might also want to disable depth draw or enable
//               transparency.
////////////////////////////////////////////////////////////////////
INLINE NodePath MeshDrawer::
get_root() {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::set_budget
//       Access: Published
//  Description: Sets the total triangle budget of the drawer.
//               This will not be exceeded.  Don't set some thing too 
//               large because it will be slow
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer::
set_budget(int total_budget) {
  _budget = total_budget;
  generator(_budget);
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::get_budget()
//       Access: Published
//  Description: Gets the total triangle budget of the drawer
////////////////////////////////////////////////////////////////////
INLINE int MeshDrawer::
get_budget() {
  return _budget;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::tri
//       Access: Published
//  Description: Draws a triangle with the given parameters.
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer::tri(const LVector3 &v1, const LVector4 &c1, const LVector2 &uv1,
                            const LVector3 &v2, const LVector4 &c2, const LVector2 &uv2,
                            const LVector3 &v3, const LVector4 &c3, const LVector2 &uv3) {

  if( _clear_index > _end_clear_index) return;

  _vertex->add_data3(v1);
  _color->add_data4(c1);
  _uv->add_data2(uv1);

  _vertex->add_data3(v2);
  _color->add_data4(c2);
  _uv->add_data2(uv2);

  _vertex->add_data3(v3);
  _color->add_data4(c3);
  _uv->add_data2(uv3);

  _clear_index += 1;
}
