// Filename: planarSlider.cxx
// Created by:  drose (10Jan00)
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

#include "planarSlider.h"

#include <compose_matrix.h>
#include <mouse.h>
#include <mouseData.h>
#include <modifierButtons.h>
#include <buttonEventDataTransition.h>
#include <buttonEventDataAttribute.h>
#include <mouseButton.h>
#include <get_rel_pos.h>

TypeHandle PlanarSlider::_type_handle;

TypeHandle PlanarSlider::_xyz_type;
TypeHandle PlanarSlider::_button_events_type;
TypeHandle PlanarSlider::_transform_type;


////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::Constructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
PlanarSlider::
PlanarSlider(const string &name) : DataNode(name) {
  _cs = default_coordinate_system;

  _mouse_pos.set(0.0, 0.0);
  _transform_mat = LMatrix4f::ident_mat();

  _transform = new MatrixDataAttribute;
  _attrib.set_attribute(_transform_type, _transform);

  _mods.add_button(MouseButton::one());
}



////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::Destructor
//       Access: Public, Scheme
//  Description:
////////////////////////////////////////////////////////////////////
PlanarSlider::
~PlanarSlider() {
}




////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::set_coordinate_system
//       Access: Public, Scheme
//  Description: Sets the coordinate system of the PlanarSlider.
//               Normally, this is the default coordinate system.
//               This changes the definition of the ground plane.
////////////////////////////////////////////////////////////////////
void PlanarSlider::
set_coordinate_system(CoordinateSystem cs) {
  _cs = cs;
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::get_coordinate_system
//       Access: Public, Scheme
//  Description: Returns the coordinate system of the PlanarSlider.
//               See set_coordinate_system().
////////////////////////////////////////////////////////////////////
CoordinateSystem PlanarSlider::
get_coordinate_system() const {
  return _cs;
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::set_transform
//       Access: Public, Scheme
//  Description: Defines the plane in which the TForm operates by
//               specifying a transform.  By default (with an identity
//               transform), the plane is the ground plane, at the
//               scale -1 to 1 around the origin.  This function may
//               specify a rotate, translate, or scale (or any
//               arbitrary transform) to reposition the plane.
////////////////////////////////////////////////////////////////////
void PlanarSlider::
set_transform(const LMatrix4f &trans) {
  _transform_mat = trans;
  _inverse_mat = invert(trans);
  _inverse_mat.set_row(3, LVector3f(0.0, 0.0, 0.0));
  set_mouse_pos(_mouse_pos);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::get_transform
//       Access: Public, Scheme
//  Description: Returns the matrix that transforms the plane in which
//               the TForm operates.  See set_transform().
////////////////////////////////////////////////////////////////////
LMatrix4f PlanarSlider::
get_transform() const {
  return _transform_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::set_mouse_pos
//       Access: Public, Scheme
//  Description: Directly sets the reported position of the mouse.  In
//               general, this function should not be used unless the
//               data graph is disconnected for some reason; normally,
//               the PlanarSlider reads the reported mouse position
//               directly from the data graph.  The mouse position
//               ranges from -1 to 1 in both axes.
////////////////////////////////////////////////////////////////////
void PlanarSlider::
set_mouse_pos(const LPoint2f &mouse_pos) {
  _mouse_pos.set(max(min(mouse_pos[0], 1.0f), -1.0f),
                 max(min(mouse_pos[1], 1.0f), -1.0f));

  LMatrix4f mat =
    _inverse_mat *
    LMatrix4f::translate_mat(LVector3f::rfu(_mouse_pos[0],
                                            _mouse_pos[1],
                                            0.0,
                                            _cs)) *
    _transform_mat;

  _transform->set_value(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::get_mouse_pos
//       Access: Public, Scheme
//  Description: Returns the last known position of the mouse, in the
//               range -1 to 1 in both axes.
////////////////////////////////////////////////////////////////////
LPoint2f PlanarSlider::
get_mouse_pos() const {
  return _mouse_pos;
}

////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::transmit_data
//       Access: Public
//  Description: Convert mouse data into a planarSlider matrix
////////////////////////////////////////////////////////////////////
void PlanarSlider::
transmit_data(NodeAttributes &data) {
  // First, update our modifier buttons.
  const ButtonEventDataAttribute *b;
  if (get_attribute_into(b, data, _button_events_type)) {
    b->update_mods(_mods);
  }

  // Now look for a mouse position.
  const NodeAttribute *xyz = data.get_attribute(_xyz_type);

  if (xyz != (NodeAttribute *)NULL) {
    if (_mods.is_any_down()) {
      LVecBase3f p = DCAST(Vec3DataAttribute, xyz)->get_value();
      set_mouse_pos(LPoint2f(p[0], p[1]));
    }
  }

  // Now send our matrix down the pipe.
  data = _attrib;
}


////////////////////////////////////////////////////////////////////
//     Function: PlanarSlider::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void PlanarSlider::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "PlanarSlider",
                DataNode::get_class_type());

  Vec3DataTransition::init_type();
  register_data_transition(_xyz_type, "XYZ",
                           Vec3DataTransition::get_class_type());
  MatrixDataTransition::init_type();
  register_data_transition(_transform_type, "Transform",
                           MatrixDataTransition::get_class_type());
  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}
