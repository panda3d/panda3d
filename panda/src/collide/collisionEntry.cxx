// Filename: collisionEntry.cxx
// Created by:  drose (24Apr00)
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
