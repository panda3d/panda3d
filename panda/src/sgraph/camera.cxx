// Filename: dCamera.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include "camera.h"
#include <projection.h>
#include <perspectiveProjection.h>
#include <frustum.h>
#include <geometricBoundingVolume.h>

#include <algorithm>


////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Camera::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: Camera::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Camera::
Camera(const string &name) : 
  ProjectionNode(name), 
  _active(true) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Camera::
Camera(const Camera &copy) : 
  ProjectionNode(copy), 
  _active(copy._active),
  _scene(copy._scene)
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void Camera::
operator = (const Camera &copy) {
  ProjectionNode::operator = (copy);
  _active = copy._active;
  _scene = copy._scene;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *Camera::
make_copy() const {
  return new Camera(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_transform() const {
  return false;
}
 

////////////////////////////////////////////////////////////////////
//     Function: Camera::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Camera::
~Camera(void) {
  // We don't have to destroy the display regions associated with the
  // camera; they're responsible for themselves.

  // We don't have to remove the camera from the global list, because
  // it must have already been removed--otherwise we couldn't be
  // deleting it!
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_num_drs
//       Access: Public
//  Description: Returns the number of display regions that share this
//               camera.
////////////////////////////////////////////////////////////////////
int Camera::
get_num_drs() const {
  return _display_regions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_dr
//       Access: Public
//  Description: Returns the nth display region that shares this
//               camera.
////////////////////////////////////////////////////////////////////
DisplayRegion *Camera::
get_dr(int index) const {
  nassertr(index >= 0 && index < get_num_drs(), NULL);
  return _display_regions[index];
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::is_in_view
//       Access: Public
//  Description: Returns true if the given point is within the bounds
//		 of the projection of the camera (i.e. if the camera
//		 can see the point).
////////////////////////////////////////////////////////////////////
bool Camera::
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
//     Function: Camera::get_fov_near_far
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Camera::
get_perspective_params(float &yfov, float &aspect, float &cnear, 
		       float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    proj->get_frustum().get_perspective_params(yfov, aspect, cnear, cfar);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_fov_near_far
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Camera::
get_perspective_params(float &xfov, float &yfov, float &aspect, float &cnear, 
		       float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    proj->get_frustum().get_perspective_params(xfov, yfov, aspect, cnear,
					       cfar);
  }
}

float Camera::
get_hfov(void) const {
  float xfov=0.0, yfov=0.0, aspect, cnear, cfar;
  get_perspective_params(xfov, yfov, aspect, cnear, cfar);
  return xfov;
}

float Camera::
get_vfov(void) const {
  float xfov=0.0, yfov=0.0, aspect, cnear, cfar;
  get_perspective_params(xfov, yfov, aspect, cnear, cfar);
  return yfov;
}

float Camera::
get_aspect(void) const {
  float yfov, aspect=0.0, cnear, cfar;
  get_perspective_params(yfov, aspect, cnear, cfar);
  return aspect;
}

void Camera::
get_near_far(float &cnear, float &cfar) const {
  if (_projection->get_type() == PerspectiveProjection::get_class_type()) {
    PerspectiveProjection *proj = DCAST(PerspectiveProjection, _projection);
    cnear = proj->get_frustum()._fnear;
    cfar = proj->get_frustum()._ffar;
  }
}

float Camera::
get_near(void) const {
  float cnear, cfar;
  get_near_far(cnear, cfar);
  return cnear;
}

float Camera::
get_far(void) const {
  float cnear, cfar;
  get_near_far(cnear, cfar);
  return cfar;
}

void Camera::
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

void Camera::
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

void Camera::
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

void Camera::
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

void Camera::
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

void Camera::
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

void Camera::
set_near(float cnear) {
  float cfar = get_far();
  set_near_far(cnear, cfar);
}

void Camera::
set_far(float cfar) {
  float cnear = get_near();
  set_near_far(cnear, cfar);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::add_display_region
//       Access: Private
//  Description: Adds the indicated DisplayRegion to the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
add_display_region(DisplayRegion *display_region) {
  _display_regions.push_back(display_region);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::remove_display_region
//       Access: Private
//  Description: Removes the indicated DisplayRegion from the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
remove_display_region(DisplayRegion *display_region) {
  DisplayRegions::iterator dri =
    find(_display_regions.begin(), _display_regions.end(), display_region);
  if (dri != _display_regions.end()) {
    _display_regions.erase(dri);
  }
}
