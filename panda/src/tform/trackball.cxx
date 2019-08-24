/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackball.cxx
 * @author drose
 * @date 2002-03-12
 */

#include "trackball.h"
#include "buttonEvent.h"
#include "buttonEventList.h"
#include "dataNodeTransmit.h"
#include "compose_matrix.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "linmath_events.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "config_tform.h"

TypeHandle Trackball::_type_handle;

/**
 *
 */
Trackball::
Trackball(const std::string &name) :
  MouseInterfaceNode(name)
{
  _transform_output = define_output("transform", TransformState::get_class_type());

  _transform = TransformState::make_identity();

  _rotscale = 0.3;
  _fwdscale = 0.3;

  _rotation = LMatrix4::ident_mat();
  _translation.set(0.0f, 0.0f, 0.0f);
  _mat = LMatrix4::ident_mat();
  _orig = LMatrix4::ident_mat();
  _invert = true;
  _cs = get_default_coordinate_system();
  _control_mode = CM_default;
}

/**
 * Reinitializes all transforms to identity.
 */
void Trackball::
reset() {
  _rotation = LMatrix4::ident_mat();
  _translation.set(0.0f, 0.0f, 0.0f);
  _orig = LMatrix4::ident_mat();
  _mat = LMatrix4::ident_mat();
}

/**
 * Returns the scale factor applied to forward and backward motion.  See
 * set_forward_scale().
 */
PN_stdfloat Trackball::
get_forward_scale() const {
  return _fwdscale;
}

/**
 * Changes the scale factor applied to forward and backward motion.  The
 * larger this number, the faster the model will move in response to dollying
 * in and out.
 */
void Trackball::
set_forward_scale(PN_stdfloat fwdscale) {
  _fwdscale = fwdscale;
}


/**
 * Return the offset from the center of rotation.
 */
const LPoint3 &Trackball::
get_pos() const {
  return _translation;
}

PN_stdfloat Trackball::
get_x() const {
  return _translation[0];
}

PN_stdfloat Trackball::
get_y() const {
  return _translation[1];
}

PN_stdfloat Trackball::
get_z() const {
  return _translation[2];
}


/**
 * Directly set the offset from the rotational origin.
 */
void Trackball::
set_pos(const LVecBase3 &vec) {
  _translation = vec;
  recompute();
}

void Trackball::
set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _translation.set(x, y, z);
  recompute();
}

void Trackball::
set_x(PN_stdfloat x) {
  _translation[0] = x;
  recompute();
}

void Trackball::
set_y(PN_stdfloat y) {
  _translation[1] = y;
  recompute();
}

void Trackball::
set_z(PN_stdfloat z) {
  _translation[2] = z;
  recompute();
}


/**
 * Return the trackball's orientation.
 */
LVecBase3 Trackball::
get_hpr() const {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  return hpr;
}

PN_stdfloat Trackball::
get_h() const {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  return hpr[0];
}

PN_stdfloat Trackball::
get_p() const {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  return hpr[1];
}

PN_stdfloat Trackball::
get_r() const {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  return hpr[2];
}


/**
 * Directly set the mover's orientation.
 */
void Trackball::
set_hpr(const LVecBase3 &hpr) {
  LVecBase3 scale, shear, old_hpr, translate;
  decompose_matrix(_rotation, scale, shear, old_hpr, translate);
  compose_matrix(_rotation, scale, shear, hpr, translate);
  recompute();
}

void Trackball::
set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  hpr.set(h, p, r);
  compose_matrix(_rotation, scale, shear, hpr, translate);
  recompute();
}

void Trackball::
set_h(PN_stdfloat h) {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  hpr[0] = h;
  compose_matrix(_rotation, scale, shear, hpr, translate);
  recompute();
}

void Trackball::
set_p(PN_stdfloat p) {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  hpr[1] = p;
  compose_matrix(_rotation, scale, shear, hpr, translate);
  recompute();
}

void Trackball::
set_r(PN_stdfloat r) {
  LVecBase3 scale, shear, hpr, translate;
  decompose_matrix(_rotation, scale, shear, hpr, translate);
  hpr[2] = r;
  compose_matrix(_rotation, scale, shear, hpr, translate);
  recompute();
}


/**
 * Reposition the center of rotation to coincide with the current translation
 * offset.  Future rotations will be about the current origin.
 */
void Trackball::
reset_origin_here() {
  recompute();
  _rotation = _orig;
  _translation.set(0.0f, 0.0f, 0.0f);
}


/**
 * Moves the center of rotation by the given amount.
 */
void Trackball::
move_origin(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _rotation = LMatrix4::translate_mat(LVecBase3(x, y, z)) *  _rotation;
}

/**
 * Returns the current center of rotation.
 */
LPoint3 Trackball::
get_origin() const {
  return _rotation.get_row3(3);
}

/**
 * Directly sets the center of rotation.
 */
void Trackball::
set_origin(const LVecBase3 &origin) {
  _rotation.set_row(3, LVecBase3(0.0f, 0.0f, 0.0f));
  _rotation = LMatrix4::translate_mat(-origin) *  _rotation;
}


/**
 * Sets the invert flag.  When this is set, the inverse matrix is generated,
 * suitable for joining to a camera, instead of parenting the scene under it.
 */
void Trackball::
set_invert(bool flag) {
  _invert = flag;
}

/**
 * Returns the invert flag.  When this is set, the inverse matrix is
 * generated, suitable for joining to a camera, instead of parenting the scene
 * under it.
 */
bool Trackball::
get_invert() const {
  return _invert;
}

/**
 * Sets the control mode.  Normally this is CM_default, which means each mouse
 * button serves its normal function.  When it is CM_truck, CM_pan, CM_dolly,
 * or CM_roll, all of the mouse buttons serve the indicated function instead
 * of their normal function.  This can be used in conjunction with some
 * external way of changing modes.
 */
void Trackball::
set_control_mode(ControlMode control_mode) {
  _control_mode = control_mode;
}

/**
 * Returns the control mode.  See set_control_mode().
 */
Trackball::ControlMode Trackball::
get_control_mode() const {
  return _control_mode;
}

/**
 * Sets the NodePath that all trackball manipulations are to be assumed to be
 * relative to.  For instance, set your camera node here to make the trackball
 * motion camera relative.  The default is the empty path, which means
 * trackball motion is in global space.
 */
void Trackball::
set_rel_to(const NodePath &rel_to) {
  _rel_to = rel_to;
}

/**
 * Returns the NodePath that all trackball manipulations are relative to, or
 * the empty path.
 */
const NodePath &Trackball::
get_rel_to() const {
  return _rel_to;
}


/**
 * Sets the coordinate system of the Trackball.  Normally, this is the default
 * coordinate system.  This changes the axes the Trackball manipulates so that
 * the user interface remains consistent across different coordinate systems.
 */
void Trackball::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
}

/**
 * Returns the coordinate system of the Trackball.  See
 * set_coordinate_system().
 */
CoordinateSystem Trackball::
get_coordinate_system() const {
  return _cs;
}

/**
 * Stores the indicated transform in the trackball.  This is a transform in
 * global space, regardless of the rel_to node.
 */
void Trackball::
set_mat(const LMatrix4 &mat) {
  _orig = mat;
  if (_invert) {
    _mat = invert(_orig);
  } else {
    _mat = _orig;
  }

  reextract();
}


/**
 * Returns the matrix represented by the trackball rotation.
 */
const LMatrix4 &Trackball::
get_mat() const {
  return _orig;
}

/**
 * Returns the actual transform that will be applied to the scene graph.  This
 * is the same as get_mat(), unless invert is in effect.
 */
const LMatrix4 &Trackball::
get_trans_mat() const {
  return _mat;
}

/**
 * Given a correctly computed _orig matrix, rederive the translation and
 * rotation elements.
 */
void Trackball::
reextract() {
  LMatrix4 m = _orig;
  if (!_rel_to.is_empty()) {
    NodePath root;
    m = _orig * root.get_transform(_rel_to)->get_mat();
  }

  m.get_row3(_translation,3);
  _rotation = m;
  _rotation.set_row(3, LVecBase3(0.0f, 0.0f, 0.0f));
}

/**
 * Rebuilds the matrix according to the stored rotation and translation
 * components.
 */
void Trackball::
recompute() {
  _orig = _rotation * LMatrix4::translate_mat(_translation);

  if (!_rel_to.is_empty()) {
    NodePath root;
    _orig = _orig * _rel_to.get_transform(root)->get_mat();
  }

  if (_invert) {
    _mat = invert(_orig);
  } else {
    _mat = _orig;
  }
}


/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void Trackball::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &output) {
  // Send our matrix down the pipe.
  _transform = TransformState::make_mat(_mat);
  output.set_data(_transform_output, EventParameter(_transform));
}
