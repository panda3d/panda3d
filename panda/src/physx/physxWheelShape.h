// Filename: physxWheelShape.h
// Created by:  enn0x (09Nov09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXWHEELSHAPE_H
#define PHYSXWHEELSHAPE_H

#include "pandabase.h"

#include "physxShape.h"
#include "physx_includes.h"

class PhysxWheelShapeDesc;
class PhysxSpringDesc;

////////////////////////////////////////////////////////////////////
//       Class : PhysxWheelShape
// Description : A special shape used for simulating a car wheel. 
//               The -Y axis should be directed toward the ground.
//
//               A ray is cast from the shape's origin along the -Y
//               axis. When the ray strikes something, and the
//               distance is:
//
//               - less than wheelRadius from the shape origin:
//                 a hard contact is created 
//               - between wheelRadius and (suspensionTravel +
//                 wheelRadius): a soft suspension contact is
//                 created 
//               - greater than (suspensionTravel + wheelRadius):
//                 no contact is created. 
//
//               Thus at the point of greatest possible suspension
//               compression the wheel axle will pass through at
//               the shape's origin. At the point greatest
//               suspension extension the wheel axle will be a
//               distance of suspensionTravel from the shape's
//               origin.
//
//               The suspension's targetValue is 0 for real cars,
//               which means that the suspension tries to extend
//               all the way. Otherwise one can specify values
//               [0,1] for suspensions which have a spring to
//               pull the wheel up when it is extended too far.
//               0.5 will then fall halfway along suspensionTravel.
//
//               The +Z axis is the 'forward' direction of travel
//               for the wheel. -Z is backwards. The wheel rolls
//               forward when rotating around the positive direction
//               around the X axis.
//
//               A positive wheel steering angle corresponds to a
//               positive rotation around the shape's Y axis.
//               (Castor angles are not modeled.)
//
//               The coordinate frame of the shape is rigidly fixed
//               on the car.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxWheelShape : public PhysxShape {

PUBLISHED:
  INLINE PhysxWheelShape();
  INLINE ~PhysxWheelShape();

  void save_to_desc(PhysxWheelShapeDesc &shapeDesc) const;

  void set_radius(float radius);
  void set_suspension_travel(float travel);
  void set_inverse_wheel_mass(float invMass);
  void set_motor_torque(float torque);
  void set_brake_torque(float torque);
  void set_steer_angle(float angle);
  void set_steer_angle_rad(float angle);
  void set_axle_speed(float speed);
  void set_wheel_flag(PhysxWheelShapeFlag flag, bool value);
  void set_suspension(const PhysxSpringDesc &spring);

  float get_radius() const;
  float get_suspension_travel() const;
  float get_inverse_wheel_mass() const;
  float get_motor_torque() const;
  float get_brake_torque() const;
  float get_steer_angle() const;
  float get_steer_angle_rad() const;
  float get_axle_speed() const;
  bool get_wheel_flag(PhysxWheelShapeFlag flag) const;

////////////////////////////////////////////////////////////////////
public:
  INLINE NxShape *ptr() const { return (NxShape *)_ptr; };

  void link(NxShape *shapePtr);
  void unlink();

private:
  NxWheelShape *_ptr;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxShape::init_type();
    register_type(_type_handle, "PhysxWheelShape", 
                  PhysxShape::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxWheelShape.I"

#endif // PHYSXWHEELSHAPE_H
