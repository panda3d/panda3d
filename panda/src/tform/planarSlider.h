// Filename: planarSlider.h
// Created by:  drose (10Jan00)
//
////////////////////////////////////////////////////////////////////

#ifndef PLANARSLIDER_H
#define PLANARSLIDER_H

#include <pandabase.h>

#include <dataNode.h>
#include <vec3DataTransition.h>
#include <vec3DataAttribute.h>
#include <matrixDataTransition.h>
#include <matrixDataAttribute.h>
#include <nodeAttributes.h>
#include <luse.h>
#include <lmatrix.h>


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

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(NodeAttributes &data);

  NodeAttributes _attrib;
  PT(MatrixDataAttribute) _transform;

  // inputs
  static TypeHandle _mods_type;
  static TypeHandle _xyz_type;

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
