// Filename: projectionNode.cxx
// Created by:  mike (09Jan97)
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
#include "projectionNode.h"
#include <frustum.h>
#include <perspectiveProjection.h>
#include <geometricBoundingVolume.h>

#include <algorithm>

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
//  Description: Returns a pointer to the particular Projection
//               associated with this ProjectionNode.
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

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::is_in_view
//       Access: Public
//  Description: Returns true if the given point is within the bounds
//               of the projection of the ProjectionNode (i.e. if the
//               camera can see the point).
////////////////////////////////////////////////////////////////////
bool ProjectionNode::
is_in_view(const LPoint3f &pos) {
  BoundingVolume *bv = _projection->make_bounds();
  if (bv == NULL)
    return false;
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bv);
  int ret = gbv->contains(pos);
  delete bv;
  return (ret != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_perspective_params
//       Access: Public
//  Description: Gets the viewing parameters of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
get_perspective_params(float &yfov, float &aspect, float &cnear,
                       float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    proj->get_frustum().get_perspective_params(yfov, aspect, cnear, cfar);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_perspective_params
//       Access: Public
//  Description: Gets the viewing parameters of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
get_perspective_params(float &xfov, float &yfov, float &aspect, float &cnear,
                       float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    proj->get_frustum().get_perspective_params(xfov, yfov, aspect, cnear,
                                               cfar);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_hfov
//       Access: Public
//  Description: Gets the horizontal field of view of the
//               ProjectionNode, assuming it represents a perspective
//               frustum.
////////////////////////////////////////////////////////////////////
float ProjectionNode::
get_hfov(void) const {
  float xfov=0.0, yfov=0.0, aspect, cnear, cfar;
  get_perspective_params(xfov, yfov, aspect, cnear, cfar);
  return xfov;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_vfov
//       Access: Public
//  Description: Gets the vertical field of view of the
//               ProjectionNode, assuming it represents a perspective
//               frustum.
////////////////////////////////////////////////////////////////////
float ProjectionNode::
get_vfov(void) const {
  float xfov=0.0, yfov=0.0, aspect, cnear, cfar;
  get_perspective_params(xfov, yfov, aspect, cnear, cfar);
  return yfov;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_aspect
//       Access: Public
//  Description: Gets the aspect ratio of the horizontal field of view
//               to the vertical field of view, assuming we have a
//               perspective frustum.
////////////////////////////////////////////////////////////////////
float ProjectionNode::
get_aspect(void) const {
  float yfov, aspect=0.0, cnear, cfar;
  get_perspective_params(yfov, aspect, cnear, cfar);
  return aspect;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_near_far
//       Access: Public
//  Description: Gets the near and far clipping planes, assuming we
//               have a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
get_near_far(float &cnear, float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    cnear = proj->get_frustum()._fnear;
    cfar = proj->get_frustum()._ffar;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_near
//       Access: Public
//  Description: Gets the near clipping plane, assuming we have a
//               perspective frustum.
////////////////////////////////////////////////////////////////////
float ProjectionNode::
get_near(void) const {
  float cnear, cfar;
  get_near_far(cnear, cfar);
  return cnear;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::get_far
//       Access: Public
//  Description: Gets the far clipping plane, assuming we have a
//               perspective frustum.
////////////////////////////////////////////////////////////////////
float ProjectionNode::
get_far(void) const {
  float cnear, cfar;
  get_near_far(cnear, cfar);
  return cfar;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_fov
//       Access: Public
//  Description: Sets the field-of-view of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_fov(float hfov) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float tfov, ufov, aspect, cnear, cfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(tfov, ufov, aspect, cnear, cfar);
    frust.make_perspective_hfov(hfov, aspect, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_fov
//       Access: Public
//  Description: Sets the field-of-view of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_fov(float hfov, float vfov) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float tfov, ufov, aspect, cnear, cfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(tfov, ufov, aspect, cnear, cfar);
    frust.make_perspective(hfov, vfov, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_hfov
//       Access: Public
//  Description: Sets the horizontal field-of-view of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_hfov(float hfov) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float tfov, ufov, aspect, cnear, cfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(tfov, ufov, aspect, cnear, cfar);
    frust.make_perspective(hfov, ufov, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_vfov
//       Access: Public
//  Description: Sets the vertical field-of-view of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_vfov(float vfov) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float tfov, ufov, aspect, cnear, cfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(tfov, ufov, aspect, cnear, cfar);
    frust.make_perspective(tfov, vfov, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_aspect
//       Access: Public
//  Description: Sets the aspect ratio of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_aspect(float aspect) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float xfov, yfov, taspect, cnear, cfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(xfov, yfov, taspect, cnear, cfar);
    // I don't know what to do when the aspect changes.  I have arbitrarilly
    // decided to preserved the horizontal FoV.  The vertical will be
    // recomputed based on the new aspect and the horizontal.
    frust.make_perspective_hfov(xfov, aspect, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_near_far
//       Access: Public
//  Description: Sets the near and far planes of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_near_far(float cnear, float cfar) {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    float xfov, yfov, aspect, tnear, tfar;
    Frustumf frust = proj->get_frustum();
    frust.get_perspective_params(xfov, yfov, aspect, tnear, tfar);
    frust.make_perspective(xfov, yfov, cnear, cfar);
    proj->set_frustum(frust);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_near
//       Access: Public
//  Description: Sets the near clipping plane of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_near(float cnear) {
  float cfar = get_far();
  set_near_far(cnear, cfar);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionNode::set_far
//       Access: Public
//  Description: Sets the far clipping plane of the ProjectionNode,
//               assuming it represents a perspective frustum.
////////////////////////////////////////////////////////////////////
void ProjectionNode::
set_far(float cfar) {
  float cnear = get_near();
  set_near_far(cnear, cfar);
}
