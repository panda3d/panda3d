// Filename: driveInterface.h
// Created by:  drose (17Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef DRIVEINTERFACE_H
#define DRIVEINTERFACE_H

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

  void set_forward_speed(float speed);
  float get_forward_speed() const;
  void set_reverse_speed(float speed);
  float get_reverse_speed() const;
  void set_rotate_speed(float speed);
  float get_rotate_speed() const;
  void set_vertical_dead_zone(float zone);
  float get_vertical_dead_zone() const;
  void set_horizontal_dead_zone(float zone);
  float get_horizontal_dead_zone() const;

  void set_vertical_ramp_up_time(float ramp_up_time);
  float get_vertical_ramp_up_time() const;
  void set_vertical_ramp_down_time(float ramp_down_time);
  float get_vertical_ramp_down_time() const;
  void set_horizontal_ramp_up_time(float ramp_up_time);
  float get_horizontal_ramp_up_time() const;
  void set_horizontal_ramp_down_time(float ramp_down_time);
  float get_horizontal_ramp_down_time() const;

  float get_speed() const;
  float get_rot_speed() const;

  void reset();

  /// **** Translation ****

  const LPoint3f &get_pos() const;
  float get_x() const;
  float get_y() const;
  float get_z() const;
  void set_pos(const LVecBase3f &vec);
  void set_pos(float x, float y, float z);
  void set_x(float x);
  void set_y(float y);
  void set_z(float z);
 
  /// **** Rotation ****
 
  const LVecBase3f &get_hpr() const;
  float get_h() const;
  float get_p() const;
  float get_r() const;
  void set_hpr(const LVecBase3f &hpr);
  void set_hpr(float h, float p, float r);
  void set_h(float h);
  void set_p(float p);
  void set_r(float r);

  void set_coordinate_system(CoordinateSystem cs);
  CoordinateSystem get_coordinate_system() const;

  void set_mat(const LMatrix4f &mat);
  const LMatrix4f &get_mat() const;


private:
  void apply(double x, double y, bool any_button);

  void reextract();
  void recompute();

  float _forward_speed;  // units / sec, mouse all the way up
  float _reverse_speed;  // units / sec, mouse all the way down
  float _rotate_speed;   // degrees / sec, mouse all the way over
  float _vertical_dead_zone;    // fraction of window size
  float _horizontal_dead_zone;  // fraction of window size

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
  LMatrix4f _mat;
  CoordinateSystem _cs;

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
