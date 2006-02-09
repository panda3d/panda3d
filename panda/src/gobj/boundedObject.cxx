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
const BoundingVolume *BoundedObject::
get_bound(int pipeline_stage) const {
  CDStageReader cdata(_cycler, pipeline_stage);
  if (cdata->_bound_type == BVT_static) {
    CDStageWriter cdataw(((BoundedObject *)this)->_cycler, pipeline_stage, cdata);
    cdataw->_flags &= ~F_bound_stale;
    return cdataw->_bound;
  }

  if ((cdata->_flags & F_bound_stale) == 0 &&
      cdata->_bound != (BoundingVolume *)NULL) {
    return cdata->_bound;
  }

  // We need to recompute the bounding volume.  We do this on the
  // current pipeline stage as well as on all upstream stages, so we
  // don't lose the cache value.
  CDStageWriter cdataw(((BoundedObject *)this)->_cycler, pipeline_stage, cdata);

#ifdef DO_PIPELINING
  ((BoundedObject *)this)->_cycler.lock();
  for (int i = 0; i < pipeline_stage; ++i) {
    CDStageReader stage_cdata(_cycler, i);
    if ((cdataw->_flags & F_bound_stale) != 0) {
      CDStageWriter stage_cdataw(((BoundedObject *)this)->_cycler, i, stage_cdata);
      ((BoundedObject *)this)->recompute_bound(i);
    }
  }
  ((BoundedObject *)this)->_cycler.release();
#endif

  ((BoundedObject *)this)->recompute_bound(pipeline_stage);
      
  return cdataw->_bound;
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
bool BoundedObject::
mark_bound_stale() {
  // With no pipeline stage specified, we must mark the bound stale on
  // all upstream stages.
  bool any_changed = false;

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler) {
    if (mark_bound_stale(pipeline_stage)) {
      any_changed = true;
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::force_bound_stale
//       Access: Published
//  Description: Marks the current volume as stale and propagates the
//               effect at least one level, even if it had already
//               been marked stale.
////////////////////////////////////////////////////////////////////
void BoundedObject::
force_bound_stale() {
  // With no pipeline stage specified, we must force the bound stale on
  // all upstream stages.
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler) {
    force_bound_stale(pipeline_stage);
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
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
propagate_stale_bound(int pipeline_stage) {
}

////////////////////////////////////////////////////////////////////
//     Function: BoundedObject::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this
//               object.  The default behavior is the compute an empty
//               bounding volume; this may be overridden to extend it
//               to create a nonempty bounding volume.  However, after
//               calling this function, it is guaranteed that the
//               _bound pointer will not be shared with any downstream
//               stage of the pipeline, and this new pointer is
//               returned.
////////////////////////////////////////////////////////////////////
BoundingVolume *BoundedObject::
recompute_bound(int pipeline_stage) {
  CDStageWriter cdata(_cycler, pipeline_stage);
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
