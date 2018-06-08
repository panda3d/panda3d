/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxController.h
 * @author enn0x
 * @date 2009-09-24
 */

#ifndef PHYSXCONTROLLER_H
#define PHYSXCONTROLLER_H

#include "pandabase.h"
#include "pointerTo.h"
#include "luse.h"

#include "physxObject.h"
#include "physxEnums.h"
#include "physx_includes.h"

class PhysxActor;

/**
 * Abstract base class for character controllers.
 */
class EXPCL_PANDAPHYSX PhysxController : public PhysxObject, public PhysxEnums {

PUBLISHED:
  void release();

  PhysxActor *get_actor() const;

  void set_pos(const LPoint3f &pos);
  void set_sharpness(float sharpness);
  void set_collision(bool enable);
  void set_min_distance(float min_dist);
  void set_step_offset(float offset);

  LPoint3f get_pos() const;
  float get_sharpness() const;

  void set_global_speed(const LVector3f &speed);
  void set_local_speed(const LVector3f &speed);
  void set_omega(float omega);
  void set_h(float heading);
  float get_h() const;

  void report_scene_changed();
  void start_jump(float v0);
  void stop_jump();

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

public:
  void update_controller(float dt);

  static PhysxController *factory(NxControllerType shapeType);

  virtual NxController *ptr() const = 0;

  virtual void link(NxController *controllerPtr) = 0;
  virtual void unlink() = 0;

protected:
  INLINE PhysxController();

private:
  NxReal get_jump_height(float dt, NxVec3 &gravity);

  float _sharpness;
  float _min_dist;

  bool  _jumping;
  float _jump_time;
  float _jump_v0;

  float _omega;
  float _heading;
  NxVec3 _speed;

  NxVec3 _up_vector;
  NxQuat _up_quat;
  NxQuat _up_quat_inv;
  NxHeightFieldAxis _up_axis;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxController",
                  PhysxObject::get_class_type());
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

#include "physxController.I"

#endif // PHYSXCONTROLLER_H
