// Filename: boundedObject.I
// Created by:  drose (02Oct99)
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

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::is_bound_stale
//       Access: Published
//  Description: Returns true if the bound is currently marked stale
//               and will be recomputed the next time get_bound() is
//               called.
//
//               This function is defined up at the top of this file,
//               because several of the inline functions below
//               reference it.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool BoundedObject::
is_bound_stale() const {
  return (_flags & F_bound_stale) != 0;
}



////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_GRAPH BoundedObject::
BoundedObject() {
  _bound_type = BVT_dynamic_sphere;
  _flags = F_bound_stale;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_bound
//       Access: Published
//  Description: Sets the type of the bounding volume that will be
//               dynamically computed for this particular node.
//               Presently, this should only be BVT_dynamic_sphere.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void BoundedObject::
set_bound(BoundedObject::BoundingVolumeType type) {
  nassertv(type != BVT_static);
  mark_bound_stale();
  _bound_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_bound
//       Access: Published
//  Description: Explicitly sets a new bounding volume on this node.
//               This will be a static bounding volume that will no
//               longer be recomputed automatically.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void BoundedObject::
set_bound(const BoundingVolume &bound) {
  mark_bound_stale();
  _bound_type = BVT_static;
  _flags &= ~F_bound_stale;  
  _bound = bound.make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::get_bound
//       Access: Published
//  Description: Returns the current bounding volume on this node,
//               possibly forcing a recompute.  A node's bounding
//               volume encloses only the node itself, irrespective of
//               the nodes above or below it in the graph.  This is
//               different from the bounding volumes on the arcs,
//               which enclose all geometry below them.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH const BoundingVolume &BoundedObject::
get_bound() const {
  if (_bound_type == BVT_static) {
    ((BoundedObject *)this)->_flags &= ~F_bound_stale;
  } else if (is_bound_stale() || _bound == (BoundingVolume *)NULL) {
    ((BoundedObject *)this)->recompute_bound();
  }
  return *_bound;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::mark_bound_stale
//       Access: Published
//  Description: Marks the current bounding volume as stale, so that
//               it will be recomputed later.  This may have a
//               cascading effect up to the root of all graphs of
//               which the node is a part.  Returns true if the
//               setting was changed, or false if it was already
//               marked stale (or if it is a static bounding volume).
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool BoundedObject::
mark_bound_stale() {
  if (is_bound_stale()) {
    return false;
  }
  _flags |= F_bound_stale;
  propagate_stale_bound();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::force_bound_stale
//       Access: Published
//  Description: Marks the current volume as stale and propagates the
//               effect at least one level, even if it had already
//               been marked stale.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void BoundedObject::
force_bound_stale() {
  _flags |= F_bound_stale;
  propagate_stale_bound();
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_final
//       Access: Published
//  Description: Sets the "final" flag on this BoundedObject.  If
//               this is true, than no bounding volume need be tested
//               below it; a positive intersection with this bounding
//               volume is deemed to be a positive intersection with
//               all geometry inside.
//
//               This is useful to quickly force a larger bounding
//               volume around a node when the GeomNodes themselves
//               are inaccurate for some reason, without forcing a
//               recompute of every nested bounding volume.  It's also
//               helpful when the bounding volume is tricked by some
//               special properties, like billboards, that may move
//               geometry out of its bounding volume otherwise.
////////////////////////////////////////////////////////////////////
INLINE_GRAPH void BoundedObject::
set_final(bool flag) {
  if (flag) {
    _flags |= F_final;
  } else {
    _flags &= ~F_final;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::is_final
//       Access: Published
//  Description: Returns the current state of the "final" flag.
//               Initially, this flag is off (false), but it may be
//               changed by an explicit call to set_final().  See
//               set_final().
////////////////////////////////////////////////////////////////////
INLINE_GRAPH bool BoundedObject::
is_final() const {
  return (_flags & F_final) != 0;
}
