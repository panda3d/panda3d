/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trackball.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef TRACKBALL_H
#define TRACKBALL_H

#include "pandabase.h"

#include "mouseInterfaceNode.h"
#include "nodePath.h"
#include "modifierButtons.h"
#include "luse.h"
#include "transformState.h"


/**
 * Trackball acts like Performer in trackball mode.  It can either spin around
 * a piece of geometry directly, or it can spin around a camera with the
 * inverse transform to make it appear that the whole world is spinning.
 *
 * The Trackball object actually just places a transform in the data graph;
 * parent a Transform2SG node under it to actually transform objects (or
 * cameras) in the world.
 */
class EXPCL_PANDA_TFORM Trackball : public MouseInterfaceNode {
PUBLISHED:
  explicit Trackball(const std::string &name);
  ~Trackball();

  void reset();

  PN_stdfloat get_forward_scale() const;
  void set_forward_scale(PN_stdfloat fwdscale);

  // **** Translation ****

  const LPoint3 &get_pos() const;
  PN_stdfloat get_x() const;
  PN_stdfloat get_y() const;
  PN_stdfloat get_z() const;
  void set_pos(const LVecBase3 &vec);
  void set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  void set_x(PN_stdfloat x);
  void set_y(PN_stdfloat y);
  void set_z(PN_stdfloat z);

  // **** Rotation ****

  LVecBase3 get_hpr() const;
  PN_stdfloat get_h() const;
  PN_stdfloat get_p() const;
  PN_stdfloat get_r() const;
  void set_hpr(const LVecBase3 &hpr);
  void set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  void set_h(PN_stdfloat h);
  void set_p(PN_stdfloat p);
  void set_r(PN_stdfloat r);

  // **** Origin of Rotation ****

  void reset_origin_here();
  void move_origin(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);

  LPoint3 get_origin() const;
  void set_origin(const LVecBase3 &origin);

  // **** Misc ****

  enum ControlMode {
    CM_default,
    CM_truck,  // Normally mouse 1
    CM_pan,    // Normally mouse 2
    CM_dolly,  // Normally mouse 3
    CM_roll,   // Normally mouse 2 + 3
  };

  void set_control_mode(ControlMode control_mode);
  ControlMode get_control_mode() const;

  void set_invert(bool flag);
  bool get_invert() const;

  void set_rel_to(const NodePath &rel_to);
  const NodePath &get_rel_to() const;

  void set_coordinate_system(CoordinateSystem cs);
  CoordinateSystem get_coordinate_system() const;

  void set_mat(const LMatrix4 &mat);
  const LMatrix4 &get_mat() const;
  const LMatrix4 &get_trans_mat() const;


private:
  void apply(double x, double y, int button);

  void reextract();
  void recompute();

  int _last_button;
  PN_stdfloat _lastx, _lasty;

  PN_stdfloat _rotscale;
  PN_stdfloat _fwdscale;

  LMatrix4 _rotation;
  LPoint3 _translation;
  LMatrix4 _mat, _orig;
  bool _invert;
  NodePath _rel_to;
  CoordinateSystem _cs;
  ControlMode _control_mode;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _pixel_xy_input;

  // outputs
  int _transform_output;

  CPT(TransformState) _transform;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseInterfaceNode::init_type();
    register_type(_type_handle, "Trackball",
                  MouseInterfaceNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
