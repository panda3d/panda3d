// Filename: boundedObject.cxx
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

#include "boundedObject.h"
#include "config_gobj.h"

#include "boundingSphere.h"

TypeHandle BoundedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundedObject::
~BoundedObject() {
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
const BoundingVolume &BoundedObject::
get_bound() const {
  {
    CDReader cdata(_cycler);
    if (cdata->_bound_type == BVT_static) {
      CDWriter cdata_w(((BoundedObject *)this)->_cycler, cdata);
      cdata_w->_flags &= ~F_bound_stale;
      return *cdata_w->_bound;
    }
    
    if (!is_bound_stale() && cdata->_bound != (BoundingVolume *)NULL) {
      return *cdata->_bound;
    }

    // We need to recompute the bounding volume.  First, we need to
    // release the old CDReader, so we can make a CDWriter in
    // recompute_bound.
  }

  // Now it's safe to recompute the bounds.
  ((BoundedObject *)this)->recompute_bound();
  CDReader cdata(_cycler);
  return *cdata->_bound;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *BoundedObject::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void BoundedObject::
propagate_stale_bound() {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this
//               object.  The default behavior is the compute an empty
//               bounding volume; this may be overridden to extend it
//               to create a nonempty bounding volume.  However, after
//               calling this function, it is guaranteed that the
//               _bound pointer will not be shared with any other
//               stage of the pipeline, and this new pointer is
//               returned.
////////////////////////////////////////////////////////////////////
BoundingVolume *BoundedObject::
recompute_bound() {
  CDWriter cdata(_cycler);
  switch (cdata->_bound_type) {
  case BVT_static:
    // Don't change it if it's a static volume.
    break;

  case BVT_dynamic_sphere:
    cdata->_bound = new BoundingSphere;
    break;

  default:
    gobj_cat.error()
      << "Unexpected _bound_type: " << (int)cdata->_bound_type
      << " in BoundedObject::recompute_bound()\n";
    cdata->_bound = new BoundingSphere;
  }

  cdata->_flags &= ~F_bound_stale;

  // Now the _bound is new and empty.
  return cdata->_bound;
}
