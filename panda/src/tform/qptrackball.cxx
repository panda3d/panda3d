// Filename: qptrackball.cxx
// Created by:  drose (12Mar02)
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

#include "qptrackball.h"
#include "buttonEvent.h"
#include "buttonEventList.h"
#include "dataNodeTransmit.h"
#include "compose_matrix.h"
#include "mouseData.h"
#include "modifierButtons.h"

TypeHandle qpTrackball::_type_handle;

// These are used internally.
#define B1_MASK 0x1
#define B2_MASK 0x2
#define B3_MASK 0x4

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpTrackball::
qpTrackball(const string &name) :
  qpDataNode(name)
{
  _pixel_xy_input = define_input("PixelXY", EventStoreVec2::get_class_type());
  _button_events_input = define_input("ButtonEvents", ButtonEventList::get_class_type());

  _transform_output = define_output("Transform", EventStoreMat4::get_class_type());

  _transform = new EventStoreMat4(LMatrix4f::ident_mat());

  _rotscale = 0.3f;
  _fwdscale = 0.3f;

  _lastx = _lasty = 0.5f;

  _rotation = LMatrix4f::ident_mat();
  _translation.set(0.0f, 0.0f, 0.0f);
  _mat = LMatrix4f::ident_mat();
  _orig = LMatrix4f::ident_mat();
  _invert = true;
  _cs = default_coordinate_system;

  _mods.add_button(MouseButton::one());
  _mods.add_button(MouseButton::two());
  _mods.add_button(MouseButton::three());
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::Destructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
qpTrackball::
~qpTrackball() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::reset
//       Access: Public, Scheme
//  Description: Reinitializes all transforms to identity.
////////////////////////////////////////////////////////////////////
void qpTrackball::
reset() {
  _rotation = LMatrix4f::ident_mat();
  _translation.set(0.0f, 0.0f, 0.0f);
  _orig = LMatrix4f::ident_mat();
  _mat = LMatrix4f::ident_mat();
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_pos
//       Access: Public, Scheme
//  Description: Return the offset from the center of rotation.
////////////////////////////////////////////////////////////////////
const LPoint3f &qpTrackball::
get_pos() const {
  return _translation;
}

float qpTrackball::
get_x() const {
  return _translation[0];
}

float qpTrackball::
get_y() const {
  return _translation[1];
}

float qpTrackball::
get_z() const {
  return _translation[2];
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_pos
//       Access: Public, Scheme
//  Description: Directly set the offset from the rotational origin.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_pos(const LVecBase3f &vec) {
  _translation = vec;
  recompute();
}

void qpTrackball::
set_pos(float x, float y, float z) {
  _translation.set(x, y, z);
  recompute();
}

void qpTrackball::
set_x(float x) {
  _translation[0] = x;
  recompute();
}

void qpTrackball::
set_y(float y) {
  _translation[1] = y;
  recompute();
}

void qpTrackball::
set_z(float z) {
  _translation[2] = z;
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_hpr
//       Access: Public, Scheme
//  Description: Return the trackball's orientation.
////////////////////////////////////////////////////////////////////
LVecBase3f qpTrackball::
get_hpr() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr;
}

float qpTrackball::
get_h() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[0];
}

float qpTrackball::
get_p() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[1];
}

float qpTrackball::
get_r() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[2];
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_hpr
//       Access: Public, Scheme
//  Description: Directly set the mover's orientation.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_hpr(const LVecBase3f &hpr) {
  LVecBase3f scale, old_hpr, translate;
  decompose_matrix(_rotation, scale, old_hpr, translate);
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void qpTrackball::
set_hpr(float h, float p, float r) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr.set(h, p, r);
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void qpTrackball::
set_h(float h) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[0] = h;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void qpTrackball::
set_p(float p) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[1] = p;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void qpTrackball::
set_r(float r) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[2] = r;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::reset_origin_here
//       Access: Public, Scheme
//  Description: Reposition the center of rotation to coincide with
//               the current translation offset.  Future rotations
//               will be about the current origin.
////////////////////////////////////////////////////////////////////
void qpTrackball::
reset_origin_here() {
  recompute();
  _rotation = _orig;
  _translation.set(0.0f, 0.0f, 0.0f);
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::move_origin
//       Access: Public, Scheme
//  Description: Moves the center of rotation by the given amount.
////////////////////////////////////////////////////////////////////
void qpTrackball::
move_origin(float x, float y, float z) {
  _rotation = LMatrix4f::translate_mat(LVecBase3f(x, y, z)) *  _rotation;
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_invert
//       Access: Public, Scheme
//  Description: Sets the invert flag.  When this is set, the inverse
//               matrix is generated, suitable for joining to a
//               camera, instead of parenting the scene under it.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_invert(bool flag) {
  _invert = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_invert
//       Access: Public, Scheme
//  Description: Returns the invert flag.  When this is set, the
//               inverse matrix is generated, suitable for joining to
//               a camera, instead of parenting the scene under it.
////////////////////////////////////////////////////////////////////
bool qpTrackball::
get_invert() const {
  return _invert;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_rel_to
//       Access: Public, Scheme
//  Description: Sets the NodePath that all trackball manipulations
//               are to be assumed to be relative to.  For instance,
//               set your camera node here to make the trackball
//               motion camera relative.  The default is the empty
//               path, which means trackball motion is in global
//               space.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_rel_to(const qpNodePath &rel_to) {
  _rel_to = rel_to;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_rel_to
//       Access: Public, Scheme
//  Description: Returns the NodePath that all trackball manipulations
//               are relative to, or the empty path.
////////////////////////////////////////////////////////////////////
const qpNodePath &qpTrackball::
get_rel_to() const {
  return _rel_to;
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_coordinate_system
//       Access: Public, Scheme
//  Description: Sets the coordinate system of the Trackball.
//               Normally, this is the default coordinate system.
//               This changes the axes the Trackball manipulates so
//               that the user interface remains consistent across
//               different coordinate systems.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_coordinate_system
//       Access: Public, Scheme
//  Description: Returns the coordinate system of the Trackball.
//               See set_coordinate_system().
////////////////////////////////////////////////////////////////////
CoordinateSystem qpTrackball::
get_coordinate_system() const {
  return _cs;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::set_mat
//       Access: Public, Scheme
//  Description: Stores the indicated transform in the trackball.
//               This is a transform in global space, regardless of
//               the rel_to node.
////////////////////////////////////////////////////////////////////
void qpTrackball::
set_mat(const LMatrix4f &mat) {
  _orig = mat;
  if (_invert) {
    _mat = invert(_orig);
  } else {
    _mat = _orig;
  }

  reextract();
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_mat
//       Access: Public, Scheme
//  Description: Returns the matrix represented by the trackball
//               rotation.
////////////////////////////////////////////////////////////////////
const LMatrix4f &qpTrackball::
get_mat() const {
  return _orig;
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::get_trans_mat
//       Access: Public, Scheme
//  Description: Returns the actual transform that will be applied to
//               the scene graph.  This is the same as get_mat(),
//               unless invert is in effect.
////////////////////////////////////////////////////////////////////
const LMatrix4f &qpTrackball::
get_trans_mat() const {
  return _mat;
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::apply
//       Access: Private
//  Description: Applies the operation indicated by the user's mouse
//               motion to the current state.  Returns the matrix
//               indicating the new state.
////////////////////////////////////////////////////////////////////
void qpTrackball::
apply(double x, double y, int button) {
  if (button && !_rel_to.is_empty()) {
    // If we have a rel_to node, we must first adjust our rotation and
    // translation to be in those local coordinates.
    reextract();
  }
  if (button == B1_MASK) {
    // Button 1: translate in plane parallel to screen.

    _translation +=
      x * _fwdscale * LVector3f::right(_cs) +
      y * _fwdscale * LVector3f::down(_cs);

  } else if (button == (B2_MASK | B3_MASK)) {
    // Buttons 2 + 3: rotate about the vector perpendicular to the
    // screen.

    _rotation *=
      LMatrix4f::rotate_mat_normaxis((x - y) * _rotscale,
                            LVector3f::forward(_cs), _cs);

  } else if ((button == B2_MASK) || (button == (B1_MASK | B3_MASK))) {
    // Button 2, or buttons 1 + 3: rotate about the right and up
    // vectors.  (We alternately define this as buttons 1 + 3, to
    // support two-button mice.)

    _rotation *=
      LMatrix4f::rotate_mat_normaxis(x * _rotscale, LVector3f::up(_cs), _cs) *
      LMatrix4f::rotate_mat_normaxis(y * _rotscale, LVector3f::right(_cs), _cs);

  } else if ((button == B3_MASK) || (button == (B1_MASK | B2_MASK))) {
    // Button 3, or buttons 1 + 2: dolly in and out along the forward
    // vector.  (We alternately define this as buttons 1 + 2, to
    // support two-button mice.)
    _translation -= y * _fwdscale * LVector3f::forward(_cs);
  }

  if (button) {
    recompute();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::reextract
//       Access: Private
//  Description: Given a correctly computed _orig matrix, rederive the
//               translation and rotation elements.
////////////////////////////////////////////////////////////////////
void qpTrackball::
reextract() {
  LMatrix4f m = _orig;
  if (!_rel_to.is_empty()) {
    qpNodePath root;
    const LMatrix4f &rel_mat = root.get_mat(_rel_to);
    m = _orig * rel_mat;
  }

  m.get_row3(_translation,3);
  _rotation = m;
  _rotation.set_row(3, LVecBase3f(0.0f, 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::recompute
//       Access: Private
//  Description: Rebuilds the matrix according to the stored rotation
//               and translation components.
////////////////////////////////////////////////////////////////////
void qpTrackball::
recompute() {
  _orig = _rotation * LMatrix4f::translate_mat(_translation);

  if (!_rel_to.is_empty()) {
    qpNodePath root;
    const LMatrix4f &rel_mat = _rel_to.get_mat(root);
    _orig = _orig * rel_mat;
  }

  if (_invert) {
    _mat = invert(_orig);
  } else {
    _mat = _orig;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: qpTrackball::do_transmit_data
//       Access: Protected, Virtual
//  Description: The virtual implementation of transmit_data().  This
//               function receives an array of input parameters and
//               should generate an array of output parameters.  The
//               input parameters may be accessed with the index
//               numbers returned by the define_input() calls that
//               were made earlier (presumably in the constructor);
//               likewise, the output parameters should be set with
//               the index numbers returned by the define_output()
//               calls.
////////////////////////////////////////////////////////////////////
void qpTrackball::
do_transmit_data(const DataNodeTransmit &input, DataNodeTransmit &output) {
  // First, update our modifier buttons.
  if (input.has_data(_button_events_input)) {
    const ButtonEventList *button_events;
    DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());
    button_events->update_mods(_mods);
  }

  // Now, check for mouse motion.
  if (input.has_data(_pixel_xy_input)) {
    const EventStoreVec2 *pixel_xy;
    DCAST_INTO_V(pixel_xy, input.get_data(_pixel_xy_input).get_ptr());
    const LVecBase2f &p = pixel_xy->get_value();
    float this_x = p[0];
    float this_y = p[1];
    int this_button = 0;

    if (_mods.is_down(MouseButton::one())) {
      this_button |= B1_MASK;
    }
    if (_mods.is_down(MouseButton::two())) {
      this_button |= B2_MASK;
    }
    if (_mods.is_down(MouseButton::three())) {
      this_button |= B3_MASK;
    }

    float x = this_x - _lastx;
    float y = this_y - _lasty;

    apply(x, y, this_button);

    _lastx = this_x;
    _lasty = this_y;
  }

  // Now send our matrix down the pipe.
  _transform->set_value(_mat);
  output.set_data(_transform_output, EventParameter(_transform));
}
