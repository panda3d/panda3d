// Filename: driveInterface.h
// Created by:  drose (12Mar02)
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

#ifndef DRIVEINTERFACE_H
#define DRIVEINTERFACE_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"
#include "luse.h"
#include "linmath_events.h"
#include "transformState.h"


////////////////////////////////////////////////////////////////////
//       Class : DriveInterface
// Description : This is a TFormer, similar to Trackball, that moves
//               around a transform matrix in response to mouse input.
//               The basic motion is on a horizontal plane, as if
//               driving a vehicle.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DriveInterface : public DataNode {
PUBLISHED:
  DriveInterface(const string &name = "");
  ~DriveInterface();

  INLINE void set_forward_speed(float speed);
  INLINE float get_forward_speed() const;
  INLINE void set_reverse_speed(float speed);
  INLINE float get_reverse_speed() const;
  INLINE void set_rotate_speed(float speed);
  INLINE float get_rotate_speed() const;
  INLINE void set_vertical_dead_zone(float zone);
  INLINE float get_vertical_dead_zone() const;
  INLINE void set_horizontal_dead_zone(float zone);
  INLINE float get_horizontal_dead_zone() const;

  INLINE void set_vertical_ramp_up_time(float ramp_up_time);
  INLINE float get_vertical_ramp_up_time() const;
  INLINE void set_vertical_ramp_down_time(float ramp_down_time);
  INLINE float get_vertical_ramp_down_time() const;
  INLINE void set_horizontal_ramp_up_time(float ramp_up_time);
  INLINE float get_horizontal_ramp_up_time() const;
  INLINE void set_horizontal_ramp_down_time(float ramp_down_time);
  INLINE float get_horizontal_ramp_down_time() const;

  INLINE float get_speed() const;
  INLINE float get_rot_speed() const;

  void reset();

  /// **** Translation ****

  INLINE const LPoint3f &get_pos() const;
  INLINE float get_x() const;
  INLINE float get_y() const;
  INLINE float get_z() const;
  INLINE void set_pos(const LVecBase3f &vec);
  INLINE void set_pos(float x, float y, float z);
  INLINE void set_x(float x);
  INLINE void set_y(float y);
  INLINE void set_z(float z);

  /// **** Rotation ****

  INLINE const LVecBase3f &get_hpr() const;
  INLINE float get_h() const;
  INLINE float get_p() const;
  INLINE float get_r() const;
  INLINE void set_hpr(const LVecBase3f &hpr);
  INLINE void set_hpr(float h, float p, float r);
  INLINE void set_h(float h);
  INLINE void set_p(float p);
  INLINE void set_r(float r);

  void set_force_roll(float force_roll);

  INLINE void set_ignore_mouse(bool ignore_mouse);
  INLINE bool get_ignore_mouse() const;

  INLINE void set_force_mouse(bool force_mouse);
  INLINE bool get_force_mouse() const;

  INLINE void set_stop_this_frame(bool stop_this_frame);
  INLINE bool get_stop_this_frame() const;

  void set_mat(const LMatrix4f &mat);
  const LMatrix4f &get_mat();

  void force_dgraph();

private:
  void apply(double x, double y, bool any_button);

  float _forward_speed;  // units / sec, mouse all the way up
  float _reverse_speed;  // units / sec, mouse all the way down
  float _rotate_speed;   // degrees / sec, mouse all the way over
  float _vertical_dead_zone;    // fraction of window size
  float _horizontal_dead_zone;  // fraction of window size
  float _vertical_center;    // window units, 0 = center, -1 = bottom, 1 = top
  float _horizontal_center;  // window units, 0 = center, -1 = left, 1 = right

  // The time it takes to ramp up to full speed from a stop (or return
  // to a stop from full speed) when using the keyboard.
  float _vertical_ramp_up_time;
  float _vertical_ramp_down_time;
  float _horizontal_ramp_up_time;
  float _horizontal_ramp_down_time;

  float _speed; // instantaneous units / sec
  float _rot_speed; // instantaneous rotational units / sec

  LPoint3f _xyz;
  LVecBase3f _hpr;
  LVector3f _vel;
  bool _ignore_mouse;
  bool _force_mouse;
  bool _stop_this_frame;

  // This is only used to return a temporary value in get_mat().
  LMatrix4f _mat;

  // Remember which mouse buttons are being held down.
  ModifierButtons _mods;

  // Remember which arrow keys are being held down and which aren't,
  // and at what point they last changed state.
  class KeyHeld {
  public:
    KeyHeld();
    float get_effect(float ramp_up_time, float ramp_down_time);
    void set_key(bool down);
    void clear();
    bool operator < (const KeyHeld &other) const;

    float _effect;
    bool _down;
    double _changed_time;
    float _effect_at_change;
  };
  KeyHeld _up_arrow, _down_arrow;
  KeyHeld _left_arrow, _right_arrow;


protected:
  // Inherited from DataNode
  virtual void do_transmit_data(const DataNodeTransmit &input,
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

  // This is the smallest meaningful value we can set on the hpr via
  // the public set_hpr() interface.  It's intended to filter out
  // small meaningless perturbations of hpr that may get introduced
  // due to numerical inaccuracy as we compute relative orientations
  // in the show.
  static const float _hpr_quantize;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "DriveInterface",
                  DataNode::get_class_type());
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
