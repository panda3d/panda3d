// Filename: projectionNode.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "projectionNode.h"
#include <frustum.h>
#include <perspectiveProjection.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ProjectionNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *ProjectionNode::
make_copy() const {
  return new ProjectionNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: set_projection 
//       Access: Public
//  Description: Sets up the ProjectionNode using a copy of the
//               indicated Projection.  If the original Projection is
//               changed or destroyed, this ProjectionNode is not
//               affected.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_projection(const Projection &projection) {
  _projection = projection.make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: share_projection 
//       Access: Public
//  Description: This is similar to set_projection(), but the
//               Projection is assigned by pointer.  If the original
//               Projection is changed, this ProjectionNode is
//               immediately affected.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
share_projection(Projection *projection) {
  _projection = projection;
}

////////////////////////////////////////////////////////////////////
//     Function: get_projection 
//       Access: Public
//  Description: Returns a pointer to particular Projection associated
//               with this ProjectionNode.
////////////////////////////////////////////////////////////////////
Projection *ProjectionNode::
get_projection() {
  if (_projection == (Projection *)NULL) {
    // If we have no projection yet, give us a default Projection.
    Frustumf f;
    _projection = new PerspectiveProjection(f);
  }

  return _projection;
}
