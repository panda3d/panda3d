// Filename: eggTransform3d.h
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

#ifndef EGGTRANSFORM3D_H
#define EGGTRANSFORM3D_H

#include "pandabase.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : EggTransform3d
// Description : This represents the <Transform> entry of a group
//               node: a list of component transform operations,
//               applied in order, that describe a net transform
//               matrix.  This is a 3-d transform, and therefore
//               computes a 4x4 matrix.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggTransform3d {
PUBLISHED:
  EggTransform3d();
  EggTransform3d(const EggTransform3d &copy);
  EggTransform3d &operator = (const EggTransform3d &copy);
  virtual ~EggTransform3d();

  INLINE void clear_transform();

  void add_translate(const LVector3d &translate);
  void add_rotx(double angle); 
  void add_roty(double angle); 
  void add_rotz(double angle); 
  void add_rotate(double angle, const LVector3d &axis);
  void add_rotate(const LQuaterniond &quat);
  void add_scale(const LVecBase3d &scale);
  void add_uniform_scale(double scale);
  INLINE void add_matrix(const LMatrix4d &mat);

  INLINE bool has_transform() const;
  INLINE void set_transform(const LMatrix4d &mat);
  INLINE const LMatrix4d &get_transform() const;
  INLINE bool transform_is_identity() const;

  enum ComponentType {
    CT_invalid,
    CT_translate,
    CT_rotx,
    CT_roty,
    CT_rotz,
    CT_rotate,
    CT_scale,
    CT_uniform_scale,
    CT_matrix
  };

  INLINE int get_num_components() const;
  INLINE ComponentType get_component_type(int n) const;
  INLINE double get_component_number(int n) const;
  INLINE const LVector3d &get_component_vector(int n) const;
  INLINE const LMatrix4d &get_component_matrix(int n) const;

  void write(ostream &out, int indent_level) const;

protected:
  void internal_clear_transform();
  void internal_add_matrix(const LMatrix4d &mat);
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
    LVector3d *_vector;
    LMatrix4d *_matrix;
  };

  typedef pvector<Component> Components;
  Components _components;
  LMatrix4d _transform;
};

#include "eggTransform3d.I"

#endif
