// Filename: boundedObject.I
// Created by:  drose (02Oct99)
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
//     Function: BoundedObject::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BoundedObject::CData::
CData() {
  _bound_type = BoundedObject::BVT_dynamic_sphere;
  _flags = F_bound_stale;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BoundedObject::CData::
CData(const BoundedObject::CData &copy) :
  _flags(copy._flags),
  _bound_type(copy._bound_type),
  _bound(copy._bound)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::CData::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BoundedObject::CData::
operator = (const BoundedObject::CData &copy) {
  _flags = copy._flags;
  _bound_type = copy._bound_type;
  _bound = copy._bound;
}


////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BoundedObject::
BoundedObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BoundedObject::
BoundedObject(const BoundedObject &copy) :
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void BoundedObject::
operator = (const BoundedObject &copy) {
  _cycler = copy._cycler;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_bound
//       Access: Published
//  Description: Sets the type of the bounding volume that will be
//               dynamically computed for this particular node.
//               Presently, this should only be BVT_dynamic_sphere.
////////////////////////////////////////////////////////////////////
INLINE void BoundedObject::
set_bound(BoundedObject::BoundingVolumeType type) {
  nassertv(type != BVT_static);
  mark_bound_stale();
  CDWriter cdata(_cycler);
  cdata->_bound_type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_bound
//       Access: Published
//  Description: Explicitly sets a new bounding volume on this node.
//               This will be a static bounding volume that will no
//               longer be recomputed automatically.
////////////////////////////////////////////////////////////////////
INLINE void BoundedObject::
set_bound(const BoundingVolume &bound) {
  mark_bound_stale();
  CDWriter cdata(_cycler);
  cdata->_bound_type = BVT_static;
  cdata->_flags &= ~F_bound_stale;  
  cdata->_bound = bound.make_copy();
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
INLINE bool BoundedObject::
mark_bound_stale() {
  if (is_bound_stale()) {
    return false;
  }
  {
    CDWriter cdata(_cycler);
    cdata->_flags |= F_bound_stale;
  }
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
INLINE void BoundedObject::
force_bound_stale() {
  {
    CDWriter cdata(_cycler);
    cdata->_flags |= F_bound_stale;
  }
  propagate_stale_bound();
}

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
INLINE bool BoundedObject::
is_bound_stale() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_bound_stale) != 0;
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
INLINE void BoundedObject::
set_final(bool flag) {
  CDWriter cdata(_cycler);
  if (flag) {
    cdata->_flags |= F_final;
  } else {
    cdata->_flags &= ~F_final;
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
INLINE bool BoundedObject::
is_final() const {
  CDReader cdata(_cycler);
  return (cdata->_flags & F_final) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::get_bound_ptr
//       Access: Protected
//  Description: Returns the state of the _bound pointer.  To be used
//               only internally by derived classes.
//
//               This returns a const pointer only; the bounding
//               volume should not be modified directly, because that
//               might interfere with pipelining.  Instead, create a
//               new copy with make_copy(), modify the copy, and
//               set_bound_ptr() with the copy.
//
//               Alternatively, if you have just called
//               recompute_bound(), which is guaranteed to reset the
//               pointer, just use the return value from that as a
//               non-const BoundingVolume pointer.
////////////////////////////////////////////////////////////////////
INLINE const BoundingVolume *BoundedObject::
get_bound_ptr() const {
  CDReader cdata(_cycler);
  return cdata->_bound;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::set_bound_ptr
//       Access: Protected
//  Description: Changes the _bound pointer.  To be used only
//               internally by derived classes, usually in
//               recompute_bound().  The return value is the same
//               pointer passed in, as a convenience (it will now be
//               reference counted).
////////////////////////////////////////////////////////////////////
INLINE BoundingVolume *BoundedObject::
set_bound_ptr(BoundingVolume *bound) {
  CDWriter cdata(_cycler);
  cdata->_bound = bound;
  return bound;
}
