// Filename: collisionEntry.cxx
// Created by:  drose (16Mar02)
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

#include "collisionEntry.h"

TypeHandle CollisionEntry::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionEntry::
CollisionEntry(const CollisionEntry &copy) :
  _from(copy._from),
  _into(copy._into),
  _from_node(copy._from_node),
  _into_node(copy._into_node),
  _into_node_path(copy._into_node_path),
  _from_space(copy._from_space),
  _into_space(copy._into_space),
  _wrt_space(copy._wrt_space),
  _inv_wrt_space(copy._inv_wrt_space),
  _flags(copy._flags),
  _into_intersection_point(copy._into_intersection_point),
  _into_surface_normal(copy._into_surface_normal),
  _into_depth(copy._into_depth)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionEntry::
operator = (const CollisionEntry &copy) {
  _from = copy._from;
  _into = copy._into;
  _from_node = copy._from_node;
  _into_node = copy._into_node;
  _into_node_path = copy._into_node_path;
  _from_space = copy._from_space;
  _into_space = copy._into_space;
  _wrt_space = copy._wrt_space;
  _inv_wrt_space = copy._inv_wrt_space;
  _flags = copy._flags;
  _into_intersection_point = copy._into_intersection_point;
  _into_surface_normal = copy._into_surface_normal;
  _into_depth = copy._into_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::compute_from_surface_normal
//       Access: Private
//  Description: Computes the "from" surface normal by converting the
//               "into" surface normal into the colliding object's
//               space.
////////////////////////////////////////////////////////////////////
void CollisionEntry::
compute_from_surface_normal() {
  _from_surface_normal = get_into_surface_normal() * get_inv_wrt_space();
  _flags |= F_has_from_surface_normal;
}
