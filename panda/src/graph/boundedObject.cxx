// Filename: boundedObject.cxx
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

#include "boundedObject.h"
#include "config_graph.h"

#include <boundingSphere.h>

TypeHandle BoundedObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BoundedObject::CData::
CData(const BoundedObject::CData &copy) :
  _flags(copy._flags),
  _bound_type(copy._bound_type),
  _bound(copy._bound)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundedObject::
~BoundedObject() {
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
  case BVT_dynamic_sphere:
    cdata->_bound = new BoundingSphere;
    break;

  default:
    graph_cat.error()
      << "Unexpected _bound_type: " << (int)cdata->_bound_type
      << " in BoundedObject::recompute_bound()\n";
    cdata->_bound = new BoundingSphere;
  }

  cdata->_flags &= ~F_bound_stale;

  // Now the _bound is new and empty.
  return cdata->_bound;
}
