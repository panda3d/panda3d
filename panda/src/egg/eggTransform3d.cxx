// Filename: eggTransform3d.cxx
// Created by:  drose (21Jun02)
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

#include "eggTransform3d.h"


////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTransform3d::
EggTransform3d() :
  _transform(LMatrix4d::ident_mat())
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTransform3d::
EggTransform3d(const EggTransform3d &copy) :
  _components(copy._components),
  _transform(copy._transform)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTransform3d &EggTransform3d::
operator = (const EggTransform3d &copy) {
  _components = copy._components;
  _transform = copy._transform;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTransform3d::
~EggTransform3d() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::clear_transform
//       Access: Public
//  Description: Resets the transform to empty, identity.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
clear_transform() {
  _components.clear();
  _transform = LMatrix4d::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_translate
//       Access: Public
//  Description: Appends a translation operation to the current
//               transform.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_translate(const LVector3d &translate) {
  _components.push_back(Component(CT_translate));
  _components.back()._vector = new LVector3d(translate);
  _transform *= LMatrix4d::translate_mat(translate);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_rotx
//       Access: Public
//  Description: Appends a rotation about the X axis to the current
//               transform.  The rotation angle is specified in
//               degrees counterclockwise about the axis.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_rotx(double angle) {
  _components.push_back(Component(CT_rotx, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(1.0, 0.0, 0.0));
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_roty
//       Access: Public
//  Description: Appends a rotation about the Y axis to the current
//               transform.  The rotation angle is specified in
//               degrees counterclockwise about the axis.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_roty(double angle) {
  _components.push_back(Component(CT_roty, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(0.0, 1.0, 0.0));
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_rotz
//       Access: Public
//  Description: Appends a rotation about the Z axis to the current
//               transform.  The rotation angle is specified in
//               degrees counterclockwise about the axis.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_rotz(double angle) {
  _components.push_back(Component(CT_rotz, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(0.0, 0.0, 1.0));
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_rotate
//       Access: Public
//  Description: Appends a rotation about an arbitrary axis to the
//               current transform.  The rotation angle is specified
//               in degrees counterclockwise about the axis.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_rotate(double angle, const LVector3d &axis) {
  _components.push_back(Component(CT_rotate, angle));
  _components.back()._vector = new LVector3d(axis);
  _transform *= LMatrix4d::rotate_mat(angle, axis);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_scale
//       Access: Public
//  Description: Appends a possibly non-uniform scale to the current
//               transform.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_scale(const LVecBase3d &scale) {
  _components.push_back(Component(CT_scale));
  _components.back()._vector = new LVector3d(scale);
  _transform *= LMatrix4d::scale_mat(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_uniform_scale
//       Access: Public
//  Description: Appends a uniform scale to the current transform.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_uniform_scale(double scale) {
  _components.push_back(Component(CT_uniform_scale, scale));
  _transform *= LMatrix4d::scale_mat(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::add_matrix
//       Access: Public
//  Description: Appends an arbitrary 4x4 matrix to the current
//               transform.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
add_matrix(const LMatrix4d &mat) {
  _components.push_back(Component(CT_matrix));
  _components.back()._matrix = new LMatrix4d(mat);
  _transform *= mat;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTransform3d::write
//       Access: Public
//  Description: Writes the transform to the indicated stream in Egg
//               format.
////////////////////////////////////////////////////////////////////
void EggTransform3d::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << "<Transform> {\n";

  int num_components = get_num_components();
  for (int i = 0; i < num_components; i++) {
    switch (get_component_type(i)) {
    case CT_translate:
      indent(out, indent_level + 2)
        << "<Translate> { " << get_component_vector(i) << " }\n";
      break;

    case CT_rotx:
      indent(out, indent_level + 2)
        << "<RotX> { " << get_component_number(i) << " }\n";
      break;

    case CT_roty:
      indent(out, indent_level + 2)
        << "<RotY> { " << get_component_number(i) << " }\n";
      break;

    case CT_rotz:
      indent(out, indent_level + 2)
        << "<RotZ> { " << get_component_number(i) << " }\n";
      break;

    case CT_rotate:
      indent(out, indent_level + 2)
        << "<Rotate> { " << get_component_number(i) << " " 
        << get_component_vector(i) << " }\n";
      break;

    case CT_scale:
      indent(out, indent_level + 2)
        << "<Scale> { " << get_component_vector(i) << " }\n";
      break;

    case CT_uniform_scale:
      indent(out, indent_level + 2)
        << "<Scale> { " << get_component_number(i) << " }\n";
      break;

    case CT_matrix:
      indent(out, indent_level + 2) << "<Matrix4> {\n";
      get_component_matrix(i).write(out, indent_level + 4);
      indent(out, indent_level + 2) << "}\n";
      break;

    case CT_invalid:
      nassertv(false);
      break;
    }
  }

  indent(out, indent_level) << "}\n";
}
