// Filename: trackball.cxx
// Created by:  drose (27Jan99)
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

#include "trackball.h"

#include "compose_matrix.h"
#include "mouse.h"
#include "mouseData.h"
#include "modifierButtons.h"
#include "buttonEventDataTransition.h"
#include "mouseButton.h"
#include "get_rel_pos.h"

TypeHandle Trackball::_type_handle;

TypeHandle Trackball::_pixel_xyz_type;
TypeHandle Trackball::_button_events_type;
TypeHandle Trackball::_transform_type;

#define B1_MASK 0x1
#define B2_MASK 0x2
#define B3_MASK 0x4

////////////////////////////////////////////////////////////////////
//     Function: Trackball::Constructor
//       Access: Public, Scheme
//  Description: Trackball uses the mouse input to spin around a DCS
//               similarly to Performer's trackball mode.
////////////////////////////////////////////////////////////////////
Trackball::
Trackball(const string &name) : DataNode(name) {
  _rotscale = 0.3;
  _fwdscale = 0.3;

  _lastx = _lasty = 0.5;

  _rotation = LMatrix4f::ident_mat();
  _translation.set(0.0, 0.0, 0.0);
  _mat = LMatrix4f::ident_mat();
  _orig = LMatrix4f::ident_mat();
  _invert = true;
  _rel_to = NULL;
  _cs = default_coordinate_system;

  _mods.add_button(MouseButton::one());
  _mods.add_button(MouseButton::two());
  _mods.add_button(MouseButton::three());

  _transform = new MatrixDataTransition;
  _transform->set_value(LMatrix4f::ident_mat());
  _attrib.set_transition(_transform_type, _transform);
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::Destructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
Trackball::
~Trackball() {
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::reset
//       Access: Public, Scheme
//  Description: Reinitializes all transforms to identity.
////////////////////////////////////////////////////////////////////
void Trackball::
reset() {
  _rotation = LMatrix4f::ident_mat();
  _translation.set(0.0, 0.0, 0.0);
  _orig = LMatrix4f::ident_mat();
  _mat = LMatrix4f::ident_mat();
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_pos
//       Access: Public, Scheme
//  Description: Return the offset from the center of rotation.
////////////////////////////////////////////////////////////////////
const LPoint3f &Trackball::
get_pos() const {
  return _translation;
}

float Trackball::
get_x() const {
  return _translation[0];
}

float Trackball::
get_y() const {
  return _translation[1];
}

float Trackball::
get_z() const {
  return _translation[2];
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_pos
//       Access: Public, Scheme
//  Description: Directly set the offset from the rotational origin.
////////////////////////////////////////////////////////////////////
void Trackball::
set_pos(const LVecBase3f &vec) {
  _translation = vec;
  recompute();
}

void Trackball::
set_pos(float x, float y, float z) {
  _translation.set(x, y, z);
  recompute();
}

void Trackball::
set_x(float x) {
  _translation[0] = x;
  recompute();
}

void Trackball::
set_y(float y) {
  _translation[1] = y;
  recompute();
}

void Trackball::
set_z(float z) {
  _translation[2] = z;
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_hpr
//       Access: Public, Scheme
//  Description: Return the trackball's orientation.
////////////////////////////////////////////////////////////////////
LVecBase3f Trackball::
get_hpr() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr;
}

float Trackball::
get_h() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[0];
}

float Trackball::
get_p() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[1];
}

float Trackball::
get_r() const {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  return hpr[2];
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_hpr
//       Access: Public, Scheme
//  Description: Directly set the mover's orientation.
////////////////////////////////////////////////////////////////////
void Trackball::
set_hpr(const LVecBase3f &hpr) {
  LVecBase3f scale, old_hpr, translate;
  decompose_matrix(_rotation, scale, old_hpr, translate);
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void Trackball::
set_hpr(float h, float p, float r) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr.set(h, p, r);
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void Trackball::
set_h(float h) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[0] = h;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void Trackball::
set_p(float p) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[1] = p;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}

void Trackball::
set_r(float r) {
  LVecBase3f scale, hpr, translate;
  decompose_matrix(_rotation, scale, hpr, translate);
  hpr[2] = r;
  compose_matrix(_rotation, scale, hpr, translate);
  recompute();
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::reset_origin_here
//       Access: Public, Scheme
//  Description: Reposition the center of rotation to coincide with
//               the current translation offset.  Future rotations
//               will be about the current origin.
////////////////////////////////////////////////////////////////////
void Trackball::
reset_origin_here() {
  recompute();
  _rotation = _orig;
  _translation.set(0.0, 0.0, 0.0);
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::move_origin
//       Access: Public, Scheme
//  Description: Moves the center of rotation by the given amount.
////////////////////////////////////////////////////////////////////
void Trackball::
move_origin(float x, float y, float z) {
  _rotation = LMatrix4f::translate_mat(LVecBase3f(x, y, z)) *  _rotation;
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_invert
//       Access: Public, Scheme
//  Description: Sets the invert flag.  When this is set, the inverse
//               matrix is generated, suitable for joining to a
//               camera, instead of parenting the scene under it.
////////////////////////////////////////////////////////////////////
void Trackball::
set_invert(bool flag) {
  _invert = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_invert
//       Access: Public, Scheme
//  Description: Returns the invert flag.  When this is set, the
//               inverse matrix is generated, suitable for joining to
//               a camera, instead of parenting the scene under it.
////////////////////////////////////////////////////////////////////
bool Trackball::
get_invert() const {
  return _invert;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_rel_to
//       Access: Public, Scheme
//  Description: Sets the node that all trackball manipulations are to
//               be assumed to be relative to.  For instance, set your
//               camera node here to make the trackball motion camera
//               relative.  The default is NULL, which means trackball
//               motion is in global space.
////////////////////////////////////////////////////////////////////
void Trackball::
set_rel_to(const Node *rel_to) {
  _rel_to = rel_to;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_rel_to
//       Access: Public, Scheme
//  Description: Returns the node that all trackball manipulations are
//               relative to, or NULL.
////////////////////////////////////////////////////////////////////
const Node *Trackball::
get_rel_to() const {
  return _rel_to;
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_coordinate_system
//       Access: Public, Scheme
//  Description: Sets the coordinate system of the Trackball.
//               Normally, this is the default coordinate system.
//               This changes the axes the Trackball manipulates so
//               that the user interface remains consistent across
//               different coordinate systems.
////////////////////////////////////////////////////////////////////
void Trackball::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_coordinate_system
//       Access: Public, Scheme
//  Description: Returns the coordinate system of the Trackball.
//               See set_coordinate_system().
////////////////////////////////////////////////////////////////////
CoordinateSystem Trackball::
get_coordinate_system() const {
  return _cs;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::set_mat
//       Access: Public, Scheme
//  Description: Stores the indicated transform in the trackball.
//               This is a transform in global space, regardless of
//               the rel_to node.
////////////////////////////////////////////////////////////////////
void Trackball::
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
//     Function: Trackball::get_mat
//       Access: Public, Scheme
//  Description: Returns the matrix represented by the trackball
//               rotation.
////////////////////////////////////////////////////////////////////
const LMatrix4f &Trackball::
get_mat() const {
  return _orig;
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::get_trans_mat
//       Access: Public, Scheme
//  Description: Returns the actual transform that will be applied to
//               the scene graph.  This is the same as get_mat(),
//               unless invert is in effect.
////////////////////////////////////////////////////////////////////
const LMatrix4f &Trackball::
get_trans_mat() const {
  return _mat;
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::apply
//       Access: Private
//  Description: Applies the operation indicated by the user's mouse
//               motion to the current state.  Returns the matrix
//               indicating the new state.
////////////////////////////////////////////////////////////////////
void Trackball::
apply(double x, double y, int button) {
  if (button && _rel_to != NULL) {
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
//     Function: Trackball::reextract
//       Access: Private
//  Description: Given a correctly computed _orig matrix, rederive the
//               translation and rotation elements.
////////////////////////////////////////////////////////////////////
void Trackball::
reextract() {
  LMatrix4f m = _orig;
  if (_rel_to != NULL) {
    LMatrix4f rel_mat;
    ::get_rel_mat(NULL, _rel_to, rel_mat);
    m = _orig * rel_mat;
  }

  m.get_row3(_translation,3);
  _rotation = m;
  _rotation.set_row(3, LVecBase3f(0.0, 0.0, 0.0));
}

////////////////////////////////////////////////////////////////////
//     Function: Trackball::recompute
//       Access: Private
//  Description: Rebuilds the matrix according to the stored rotation
//               and translation components.
////////////////////////////////////////////////////////////////////
void Trackball::
recompute() {
  _orig = _rotation * LMatrix4f::translate_mat(_translation);

  if (_rel_to != NULL) {
    LMatrix4f rel_mat;
    ::get_rel_mat(_rel_to, NULL, rel_mat);
    _orig = _orig * rel_mat;
  }

  if (_invert) {
    _mat = invert(_orig);
  } else {
    _mat = _orig;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::transmit_data
//       Access: Public
//  Description: Convert mouse data into a trackball matrix
////////////////////////////////////////////////////////////////////
void Trackball::
transmit_data(AllTransitionsWrapper &data) {
  // First, update our modifier buttons.
  const ButtonEventDataTransition *b;
  if (get_transition_into(b, data, _button_events_type)) {
    b->update_mods(_mods);
  }

  // Now, check for mouse motion.
  const NodeTransition *pixel_xyz = data.get_transition(_pixel_xyz_type);

  if (pixel_xyz != (NodeTransition *)NULL) {
    LVecBase3f p = DCAST(Vec3DataTransition, pixel_xyz)->get_value();
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
  data = _attrib;
}


////////////////////////////////////////////////////////////////////
//     Function: Trackball::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void Trackball::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "Trackball",
                DataNode::get_class_type());

  Vec3DataTransition::init_type();
  register_data_transition(_pixel_xyz_type, "PixelXYZ",
                           Vec3DataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
  MatrixDataTransition::init_type();
  register_data_transition(_transform_type, "Transform",
                           MatrixDataTransition::get_class_type());
}
