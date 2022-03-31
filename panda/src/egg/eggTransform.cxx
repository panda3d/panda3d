/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTransform.cxx
 * @author drose
 * @date 2002-06-21
 */

#include "eggTransform.h"


/**
 *
 */
EggTransform::
EggTransform() :
  _is_transform_2d(true),
  _transform(LMatrix4d::ident_mat())
{
}

/**
 *
 */
EggTransform::
EggTransform(const EggTransform &copy) :
  _is_transform_2d(copy._is_transform_2d),
  _components(copy._components),
  _transform(copy._transform)
{
}

/**
 *
 */
EggTransform &EggTransform::
operator = (const EggTransform &copy) {
  _is_transform_2d = copy._is_transform_2d;
  _components = copy._components;
  _transform = copy._transform;
  return *this;
}

/**
 *
 */
EggTransform::
~EggTransform() {
}

/**
 * Appends a 2-d translation operation to the current transform.
 */
void EggTransform::
add_translate2d(const LVector2d &translate) {
  _components.push_back(Component(CT_translate2d));
  _components.back()._vec2 = new LVecBase2d(translate);
  _transform *= LMatrix4d::translate_mat(LVector3d(translate[0], translate[1], 0.0));
  transform_changed();
}

/**
 * Appends a 3-d translation operation to the current transform.
 */
void EggTransform::
add_translate3d(const LVector3d &translate) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_translate3d));
  _components.back()._vec3 = new LVecBase3d(translate);
  _transform *= LMatrix4d::translate_mat(translate);
  transform_changed();
}

/**
 * Appends a 2-d rotation to the current transform.  The rotation angle is
 * specified in degrees counterclockwise about the origin.
 */
void EggTransform::
add_rotate2d(double angle) {
  _components.push_back(Component(CT_rotate2d, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(0.0, 0.0, 1.0));
  transform_changed();
}

/**
 * Appends a rotation about the X axis to the current transform.  The rotation
 * angle is specified in degrees counterclockwise about the axis.
 */
void EggTransform::
add_rotx(double angle) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_rotx, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(1.0, 0.0, 0.0));
  transform_changed();
}

/**
 * Appends a rotation about the Y axis to the current transform.  The rotation
 * angle is specified in degrees counterclockwise about the axis.
 */
void EggTransform::
add_roty(double angle) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_roty, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(0.0, 1.0, 0.0));
  transform_changed();
}

/**
 * Appends a rotation about the Z axis to the current transform.  The rotation
 * angle is specified in degrees counterclockwise about the axis.
 */
void EggTransform::
add_rotz(double angle) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_rotz, angle));
  _transform *= LMatrix4d::rotate_mat_normaxis(angle, LVector3d(0.0, 0.0, 1.0));
  transform_changed();
}

/**
 * Appends a 3-d rotation about an arbitrary axis to the current transform.
 * The rotation angle is specified in degrees counterclockwise about the axis.
 */
void EggTransform::
add_rotate3d(double angle, const LVector3d &axis) {
  _is_transform_2d = false;
  LVector3d normaxis = normalize(axis);
  _components.push_back(Component(CT_rotate3d, angle));
  _components.back()._vec3 = new LVecBase3d(normaxis);
  _transform *= LMatrix4d::rotate_mat(angle, normaxis);
  transform_changed();
}

/**
 * Appends an arbitrary 3-d rotation to the current transform, expressed as a
 * quaternion.  This is converted to axis-angle notation for the egg file.
 */
void EggTransform::
add_rotate3d(const LQuaterniond &quat) {
  _is_transform_2d = false;
  add_rotate3d(quat.get_angle(), quat.get_axis());
  transform_changed();
}

/**
 * Appends a possibly non-uniform scale to the current transform.
 */
void EggTransform::
add_scale2d(const LVecBase2d &scale) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_scale2d));
  _components.back()._vec2 = new LVecBase2d(scale);
  _transform *= LMatrix4d::scale_mat(LVecBase3d(scale[0], scale[1], 1.0));
  transform_changed();
}

/**
 * Appends a possibly non-uniform scale to the current transform.
 */
void EggTransform::
add_scale3d(const LVecBase3d &scale) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_scale3d));
  _components.back()._vec3 = new LVecBase3d(scale);
  _transform *= LMatrix4d::scale_mat(scale);
  transform_changed();
}

/**
 * Appends a uniform scale to the current transform.
 */
void EggTransform::
add_uniform_scale(double scale) {
  _components.push_back(Component(CT_uniform_scale, scale));
  _transform *= LMatrix4d::scale_mat(scale);
  transform_changed();
}

/**
 * Writes the transform to the indicated stream in Egg format.
 */
void EggTransform::
write(std::ostream &out, int indent_level, const std::string &label) const {
  indent(out, indent_level) << label << " {\n";

  int num_components = get_num_components();
  for (int i = 0; i < num_components; i++) {
    switch (get_component_type(i)) {
    case CT_translate2d:
      indent(out, indent_level + 2)
        << "<Translate> { " << get_component_vec2(i) << " }\n";
      break;

    case CT_translate3d:
      indent(out, indent_level + 2)
        << "<Translate> { " << get_component_vec3(i) << " }\n";
      break;

    case CT_rotate2d:
      indent(out, indent_level + 2)
        << "<Rotate> { " << get_component_number(i) << " }\n";
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

    case CT_rotate3d:
      indent(out, indent_level + 2)
        << "<Rotate> { " << get_component_number(i) << " "
        << get_component_vec3(i) << " }\n";
      break;

    case CT_scale2d:
      indent(out, indent_level + 2)
        << "<Scale> { " << get_component_vec2(i) << " }\n";
      break;

    case CT_scale3d:
      indent(out, indent_level + 2)
        << "<Scale> { " << get_component_vec3(i) << " }\n";
      break;

    case CT_uniform_scale:
      indent(out, indent_level + 2)
        << "<Scale> { " << get_component_number(i) << " }\n";
      break;

    case CT_matrix3:
      indent(out, indent_level + 2) << "<Matrix3> {\n";
      get_component_mat3(i).write(out, indent_level + 4);
      indent(out, indent_level + 2) << "}\n";
      break;

    case CT_matrix4:
      indent(out, indent_level + 2) << "<Matrix4> {\n";
      get_component_mat4(i).write(out, indent_level + 4);
      indent(out, indent_level + 2) << "}\n";
      break;

    case CT_invalid:
      nassertv(false);
      break;
    }
  }

  indent(out, indent_level) << "}\n";
}

/**
 * Resets the transform to empty without calling transform_changed().
 */
void EggTransform::
internal_clear_transform() {
  _is_transform_2d = true;
  _components.clear();
  _transform = LMatrix4d::ident_mat();
}

/**
 * Appends an arbitrary 4x4 matrix to the current transform, without calling
 * transform_changed().
 */
void EggTransform::
internal_add_matrix(const LMatrix3d &mat) {
  _components.push_back(Component(CT_matrix3));
  _components.back()._mat3 = new LMatrix3d(mat);
  LMatrix4d mat4(mat(0, 0), mat(0, 1), 0.0, mat(0, 2),
                 mat(1, 0), mat(1, 1), 0.0, mat(1, 2),
                 0.0, 0.0, 1.0, 0.0,
                 mat(2, 0), mat(2, 1), 0.0, mat(2, 2));
  _transform *= mat4;
}

/**
 * Appends an arbitrary 4x4 matrix to the current transform, without calling
 * transform_changed().
 */
void EggTransform::
internal_add_matrix(const LMatrix4d &mat) {
  _is_transform_2d = false;
  _components.push_back(Component(CT_matrix4));
  _components.back()._mat4 = new LMatrix4d(mat);
  _transform *= mat;
}

/**
 * This virtual method is called whenever the transform is changed; it is
 * intended to provide a hook for derived classes (e.g.  EggGroup) to update
 * their internal cache appropriately.
 */
void EggTransform::
transform_changed() {
}
