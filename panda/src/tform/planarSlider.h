// Filename: planarSlider.h
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

#ifndef PLANARSLIDER_H
#define PLANARSLIDER_H

#include <pandabase.h>

#include <dataNode.h>
#include <vec3DataTransition.h>
#include <matrixDataTransition.h>
#include <allTransitionsWrapper.h>
#include <luse.h>
#include <lmatrix.h>
#include <modifierButtons.h>


////////////////////////////////////////////////////////////////////
//       Class : PlanarSlider
// Description : A PlanarSlider TFormer can be connected to a DCS.  It
//               accepts mouse input from the data graph and uses it
//               to slide the DCS around within a specified plane.  By
//               default, this is the ground plane in the range -1,1.
//               This can be scaled or rotated using the
//               set_transform() method.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PlanarSlider : public DataNode {
public:

  PlanarSlider(const string &name = "");
  ~PlanarSlider();

  void set_coordinate_system(CoordinateSystem cs);
  CoordinateSystem get_coordinate_system() const;

  void set_transform(const LMatrix4f &trans);
  LMatrix4f get_transform() const;

  void set_mouse_pos(const LPoint2f &mouse_pos);
  LPoint2f get_mouse_pos() const;

private:
  CoordinateSystem _cs;
  LMatrix4f _transform_mat, _inverse_mat;
  LPoint2f _mouse_pos;
  ModifierButtons _mods;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(AllTransitionsWrapper &data);

  AllTransitionsWrapper _attrib;
  PT(MatrixDataTransition) _transform;

  // inputs
  static TypeHandle _xyz_type;
  static TypeHandle _button_events_type;

  // outputs
  static TypeHandle _transform_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif
