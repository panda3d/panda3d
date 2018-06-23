/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTransform.h
 * @author drose
 * @date 2002-06-21
 */

#ifndef EGGTRANSFORM_H
#define EGGTRANSFORM_H

#include "pandabase.h"
#include "luse.h"
#include "eggObject.h"

/**
 * This represents the <Transform> entry of a group or texture node: a list of
 * component transform operations, applied in order, that describe a net
 * transform matrix.
 *
 * This may be either a 3-d transform, and therefore described by a 4x4
 * matrix, or a 2-d transform, described by a 3x3 matrix.
 */
class EXPCL_PANDA_EGG EggTransform {
PUBLISHED:
  EggTransform();
  EggTransform(const EggTransform &copy);
  EggTransform &operator = (const EggTransform &copy);
  virtual ~EggTransform();

  INLINE void clear_transform();

  void add_translate2d(const LVector2d &translate);
  void add_translate3d(const LVector3d &translate);
  void add_rotate2d(double angle);
  void add_rotx(double angle);
  void add_roty(double angle);
  void add_rotz(double angle);
  void add_rotate3d(double angle, const LVector3d &axis);
  void add_rotate3d(const LQuaterniond &quat);
  void add_scale2d(const LVecBase2d &scale);
  void add_scale3d(const LVecBase3d &scale);
  void add_uniform_scale(double scale);
  INLINE void add_matrix3(const LMatrix3d &mat);
  INLINE void add_matrix4(const LMatrix4d &mat);

  INLINE bool has_transform() const;
  INLINE bool has_transform2d() const;
  INLINE void set_transform2d(const LMatrix3d &mat);
  INLINE bool has_transform3d() const;
  INLINE void set_transform3d(const LMatrix4d &mat);
  INLINE LMatrix3d get_transform2d() const;
  INLINE const LMatrix4d &get_transform3d() const;
  INLINE bool transform_is_identity() const;

  enum ComponentType {
    CT_invalid,
    CT_translate2d,
    CT_translate3d,
    CT_rotate2d,
    CT_rotx,
    CT_roty,
    CT_rotz,
    CT_rotate3d,
    CT_scale2d,
    CT_scale3d,
    CT_uniform_scale,
    CT_matrix3,
    CT_matrix4
  };

  INLINE int get_num_components() const;
  INLINE ComponentType get_component_type(int n) const;
  INLINE double get_component_number(int n) const;
  INLINE const LVecBase2d &get_component_vec2(int n) const;
  INLINE const LVecBase3d &get_component_vec3(int n) const;
  INLINE const LMatrix3d &get_component_mat3(int n) const;
  INLINE const LMatrix4d &get_component_mat4(int n) const;

  void write(std::ostream &out, int indent_level,
             const std::string &label) const;

protected:
  void internal_clear_transform();
  void internal_add_matrix(const LMatrix3d &mat);
  void internal_add_matrix(const LMatrix4d &mat);
  INLINE void internal_set_transform(const LMatrix3d &mat);
  INLINE void internal_set_transform(const LMatrix4d &mat);

  virtual void transform_changed();

private:
  class Component {
  public:
    INLINE Component(ComponentType type, double number = 0.0);
    INLINE Component(const Component &copy);
    INLINE void operator = (const Component &copy);
    INLINE ~Component();

    ComponentType _type;
    double _number;
    LVecBase2d *_vec2;
    LVecBase3d *_vec3;
    LMatrix3d *_mat3;
    LMatrix4d *_mat4;
  };

  bool _is_transform_2d;
  typedef pvector<Component> Components;
  Components _components;
  LMatrix4d _transform;
};

#include "eggTransform.I"

#endif
