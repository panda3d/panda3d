/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxVehicle.h
 * @author enn0x
 * @date 2010-03-23
 */

#ifndef PHYSXVEHICLE_H
#define PHYSXVEHICLE_H

#include "pandabase.h"
#include "pointerToArray.h"

#include "physxObject.h"
#include "physx_includes.h"

class PhysxActor;
class PhysxWheel;
class PhysxVehicleDesc;
class PhysxScene;

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxVehicle : public PhysxObject {

PUBLISHED:
  INLINE PhysxVehicle();
  INLINE ~PhysxVehicle();

  //PhysxActor *get_actor() const;

  //unsigned int get_num_wheels() const;
  //PhysxWheel *get_wheel(unsigned int idx) const;
  //MAKE_SEQ(get_wheels, get_num_wheels, get_wheel);

  INLINE void ls() const;
  INLINE void ls(std::ostream &out, int indent_level=0) const;

private:

PUBLISHED:
  void release();

public:
  void create(PhysxScene *scene, PhysxVehicleDesc &desc);
  void update_vehicle(float dt);

private:
  PTA(PT(PhysxWheel)) _wheels;
  PT(PhysxActor) _actor;
  PT(PhysxScene) _scene;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PhysxObject::init_type();
    register_type(_type_handle, "PhysxVehicle",
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

#include "physxVehicle.I"

#endif // PHYSXVEHICLE_H
