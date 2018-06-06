/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file driveInterface.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef DRIVEINTERFACE_H
#define DRIVEINTERFACE_H

#include "pandabase.h"

#include "mouseInterfaceNode.h"
#include "modifierButtons.h"
#include "luse.h"
#include "linmath_events.h"
#include "transformState.h"


/**
 * This is a TFormer, similar to Trackball, that moves around a transform
 * matrix in response to mouse input.  The basic motion is on a horizontal
 * plane, as if driving a vehicle.
 */
class EXPCL_PANDA_TFORM DriveInterface : public MouseInterfaceNode {
PUBLISHED:
  explicit DriveInterface(const std::string &name = "");
  ~DriveInterface();

  INLINE void set_forward_speed(PN_stdfloat speed);
  INLINE PN_stdfloat get_forward_speed() const;
  INLINE void set_reverse_speed(PN_stdfloat speed);
  INLINE PN_stdfloat get_reverse_speed() const;
  INLINE void set_rotate_speed(PN_stdfloat speed);
  INLINE PN_stdfloat get_rotate_speed() const;
  INLINE void set_vertical_dead_zone(PN_stdfloat zone);
  INLINE PN_stdfloat get_vertical_dead_zone() const;
  INLINE void set_horizontal_dead_zone(PN_stdfloat zone);
  INLINE PN_stdfloat get_horizontal_dead_zone() const;

  INLINE void set_vertical_ramp_up_time(PN_stdfloat ramp_up_time);
  INLINE PN_stdfloat get_vertical_ramp_up_time() const;
  INLINE void set_vertical_ramp_down_time(PN_stdfloat ramp_down_time);
  INLINE PN_stdfloat get_vertical_ramp_down_time() const;
  INLINE void set_horizontal_ramp_up_time(PN_stdfloat ramp_up_time);
  INLINE PN_stdfloat get_horizontal_ramp_up_time() const;
  INLINE void set_horizontal_ramp_down_time(PN_stdfloat ramp_down_time);
  INLINE PN_stdfloat get_horizontal_ramp_down_time() const;

  INLINE PN_stdfloat get_speed() const;
  INLINE PN_stdfloat get_rot_speed() const;

  void reset();

  // **** Translation ****

  INLINE const LPoint3 &get_pos() const;
  INLINE PN_stdfloat get_x() const;
  INLINE PN_stdfloat get_y() const;
  INLINE PN_stdfloat get_z() const;
  INLINE void set_pos(const LVecBase3 &vec);
  INLINE void set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE void set_x(PN_stdfloat x);
  INLINE void set_y(PN_stdfloat y);
  INLINE void set_z(PN_stdfloat z);

  // **** Rotation ****

  INLINE const LVecBase3 &get_hpr() const;
  INLINE PN_stdfloat get_h() const;
  INLINE PN_stdfloat get_p() const;
  INLINE PN_stdfloat get_r() const;
  INLINE void set_hpr(const LVecBase3 &hpr);
  INLINE void set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  INLINE void set_h(PN_stdfloat h);
  INLINE void set_p(PN_stdfloat p);
  INLINE void set_r(PN_stdfloat r);

  void set_force_roll(PN_stdfloat force_roll);

  INLINE void set_ignore_mouse(bool ignore_mouse);
  INLINE bool get_ignore_mouse() const;

  INLINE void set_force_mouse(bool force_mouse);
  INLINE bool get_force_mouse() const;

  INLINE void set_stop_this_frame(bool stop_this_frame);
  INLINE bool get_stop_this_frame() const;

  void set_mat(const LMatrix4 &mat);
  const LMatrix4 &get_mat();

  void force_dgraph();

private:
  void apply(double x, double y, bool any_button);

  PN_stdfloat _forward_speed;  // units / sec, mouse all the way up
  PN_stdfloat _reverse_speed;  // units / sec, mouse all the way down
  PN_stdfloat _rotate_speed;   // degrees / sec, mouse all the way over
  PN_stdfloat _vertical_dead_zone;    // fraction of window size
  PN_stdfloat _horizontal_dead_zone;  // fraction of window size
  PN_stdfloat _vertical_center;    // window units, 0 = center, -1 = bottom, 1 = top
  PN_stdfloat _horizontal_center;  // window units, 0 = center, -1 = left, 1 = right

  // The time it takes to ramp up to full speed from a stop (or return to a
  // stop from full speed) when using the keyboard.
  PN_stdfloat _vertical_ramp_up_time;
  PN_stdfloat _vertical_ramp_down_time;
  PN_stdfloat _horizontal_ramp_up_time;
  PN_stdfloat _horizontal_ramp_down_time;

  PN_stdfloat _speed; // instantaneous units / sec
  PN_stdfloat _rot_speed; // instantaneous rotational units / sec

  LPoint3 _xyz;
  LVecBase3 _hpr;
  LVector3 _vel;
  bool _ignore_mouse;
  bool _force_mouse;
  bool _stop_this_frame;

  // This is only used to return a temporary value in get_mat().
  LMatrix4 _mat;

  // Remember which arrow keys are being held down and which aren't, and at
  // what point they last changed state.
  class KeyHeld {
  public:
    KeyHeld();
    PN_stdfloat get_effect(PN_stdfloat ramp_up_time, PN_stdfloat ramp_down_time);
    void set_key(bool down);
    void clear();
    bool operator < (const KeyHeld &other) const;

    PN_stdfloat _effect;
    bool _down;
    double _changed_time;
    PN_stdfloat _effect_at_change;
  };
  KeyHeld _up_arrow, _down_arrow;
  KeyHeld _left_arrow, _right_arrow;


protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _xy_input;
  int _button_events_input;

  // outputs
  int _transform_output;
  int _velocity_output;

  CPT(TransformState) _transform;
  PT(EventStoreVec3) _velocity;

  // This is the smallest meaningful value we can set on the hpr via the
  // public set_hpr() interface.  It's intended to filter out small
  // meaningless perturbations of hpr that may get introduced due to numerical
  // inaccuracy as we compute relative orientations in the show.
  static const PN_stdfloat _hpr_quantize;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseInterfaceNode::init_type();
    register_type(_type_handle, "DriveInterface",
                  MouseInterfaceNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "driveInterface.I"

#endif
