// Filename: boundedObject.cxx
// Created by:  drose (02Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "boundedObject.h"
#include "config_graph.h"

#include <boundingSphere.h>

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
//               to create a nonempty bounding volume.
////////////////////////////////////////////////////////////////////
void BoundedObject::
recompute_bound() {
  switch (_bound_type) {
  case BVT_dynamic_sphere:
    _bound = new BoundingSphere;
    break;

  default:
    graph_cat.error()
      << "Unexpected _bound_type: " << (int)_bound_type 
      << " in BoundedObject::recompute_bound()\n";
    _bound = NULL;
  }

  _bound_stale = false;
  // By default, the bound is empty.
}
