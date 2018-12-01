/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxController.cxx
 * @author enn0x
 * @date 2009-09-24
 */

#include "event.h"
#include "eventQueue.h"
#include "eventParameter.h"

#include "physxController.h"
#include "physxManager.h"
#include "physxActor.h"
#include "physxBoxController.h"
#include "physxCapsuleController.h"

TypeHandle PhysxController::_type_handle;

/**
 *
 */
void PhysxController::
release() {

  nassertv(_error_type == ET_ok);

  NxControllerManager *cm = get_actor()->get_scene()->cm();
  unlink();
  cm->releaseController(*ptr());
}

/**
 *
 */
PhysxController *PhysxController::
factory(NxControllerType controllerType) {

  switch (controllerType) {

  case NX_CONTROLLER_BOX:
    return new PhysxBoxController();

  case NX_CONTROLLER_CAPSULE:
    return new PhysxCapsuleController();

  default:
    break;
  }

  physx_cat.error() << "Unknown controller type.\n";
  return nullptr;
}


/**
 * Retrieves the actor which this controller is associated with.
 */
PhysxActor *PhysxController::
get_actor() const {

  nassertr(_error_type == ET_ok, nullptr);
  return (PhysxActor *)(ptr()->getActor()->userData);
}

/**
 * Sets the position of the controller is global space.  This can be used for
 * initial placement or for teleporting the character.
 */
void PhysxController::
set_pos(const LPoint3f &pos) {

  nassertv(_error_type == ET_ok);
  ptr()->setPosition(PhysxManager::point3_to_nxExtVec3(pos));
}

/**
 * Retruns the position of the controller is global space.
 */
LPoint3f PhysxController::
get_pos() const {

  nassertr(_error_type == ET_ok, LPoint3f::zero());
  return PhysxManager::nxExtVec3_to_point3(ptr()->getPosition());
}

/**
 * Sharpness is used to smooth motion with a feedback filter, having a value
 * between 0 (so smooth it doesn't move) and 1 (no smoothing = unfiltered
 * motion). Sharpness can ease the motion curve when the auto-step feature is
 * used with boxes.  Default value is 1.0.
 */
void PhysxController::
set_sharpness(float sharpness) {

  nassertv(_error_type == ET_ok);
  nassertv(sharpness > 0.0f);
  nassertv(sharpness <= 1.0f);

  _sharpness = sharpness;
}

/**
 * Returns the sharpness used to ease the motion curve when the auto-step
 * feature is used.  Default value is 1.0.
 */
float PhysxController::
get_sharpness() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _sharpness;
}

/**
 * Enable/Disable collisions for this controller and actor.
 */
void PhysxController::
set_collision(bool enable) {

  nassertv(_error_type == ET_ok);
  ptr()->setCollision(enable);
}

/**
 * Sets the the minimum travelled distance to consider when moving the
 * controller.  If travelled distance is smaller, the character doesn't move.
 * This is used to stop the recursive motion algorithm when remaining distance
 * to travel is small.  The default value is 0.0001.
 */
void PhysxController::
set_min_distance(float min_dist) {

  nassertv(_error_type == ET_ok);
  nassertv(min_dist > 0.0f);

  _min_dist = min_dist;
}

/**
 * Sets the step height/offset for the controller.
 */
void PhysxController::
set_step_offset(float offset) {

  nassertv(_error_type == ET_ok);
  nassertv(offset > 0.0f);

  ptr()->setStepOffset(offset);
}

/**
 * Sets the linear speed of the controller in global space.
 */
void PhysxController::
set_global_speed(const LVector3f &speed) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!speed.is_nan());

  _speed = NxVec3(speed.get_x(), speed.get_y(), speed.get_z());
}

/**
 * Sets the linear speed of the controller in local coordinates.
 */
void PhysxController::
set_local_speed(const LVector3f &speed) {

  nassertv(_error_type == ET_ok);
  nassertv_always(!speed.is_nan());

  NodePath np = get_actor()->get_node_path();
  nassertv(!np.is_empty());

  NxQuat q = ptr()->getActor()->getGlobalOrientationQuat();
  NxVec3 s = NxVec3(speed.get_x(), speed.get_y(), speed.get_z());
  _speed = (q * _up_quat_inv).rot(s);
}

/**
 * Sets the angular velocity (degrees per second) of the controller.  The
 * angular velocity is used to compute the new heading when updating the
 * controller.
 */
void PhysxController::
set_omega(float omega) {

  nassertv(_error_type == ET_ok);
  _omega = omega;
}

/**
 * Sets the heading of the controller is global space.  Note: only heading is
 * supported.  Pitch and roll are constrained by PhysX in order to alyways
 * keep the character upright.
 */
void PhysxController::
set_h(float heading) {

  nassertv(_error_type == ET_ok);

  _heading = heading;
  NxQuat q(_heading, _up_vector);
  ptr()->getActor()->moveGlobalOrientationQuat(_up_quat * q);
}

/**
 * Returns the heading of the controller in global space.
 */
float PhysxController::
get_h() const {

  nassertr(_error_type == ET_ok, 0.0f);
  return _heading;
}

/**
 * The character controller uses caching in order to speed up collision
 * testing, this caching can not detect when static objects have changed in
 * the scene.  You need to call this method when such changes have been made.
 */
void PhysxController::
report_scene_changed() {

  nassertv(_error_type == ET_ok);
  ptr()->reportSceneChanged();
}

/**
 *
 */
void PhysxController::
update_controller(float dt) {

  nassertv(_error_type == ET_ok);

  // Speed
  NxU32 mask = 0xFFFFFFFF;
  NxU32 collision_flags;
  NxVec3 gravity;

  ptr()->getActor()->getScene().getGravity(gravity);

  NxVec3 d = (_speed + gravity) * dt;

  NxReal heightDelta = get_jump_height(dt, gravity);
  if (heightDelta != 0.0f) {
    ((_up_axis == NX_Z) ? d.z : d.y) += heightDelta;
  }

  ptr()->move(d, mask, _min_dist, collision_flags, _sharpness);

  if (collision_flags & NXCC_COLLISION_DOWN) {
    stop_jump();
  }

  // Omega
  if (_omega != 0.0f) {
    NxReal delta = _omega * dt;
    _heading += delta;
    NxQuat q(_heading, _up_vector);
    ptr()->getActor()->moveGlobalOrientationQuat(_up_quat * q);
  }

  // Reset speed and omega
  _speed.zero();
  _omega = 0.0f;
}

/**
 *
 */
NxReal PhysxController::
get_jump_height(float dt, NxVec3 &gravity) {

  if (_jumping == false) {
    return 0.0f;
  }

  _jump_time += dt;

  float G = (_up_axis == NX_Z) ? gravity.z : gravity.y;
  float h = 2.0f * G * _jump_time * _jump_time + _jump_v0 * _jump_time;
  return (h - G) * dt;
}

/**
 * Enters the jump mode.  The parameter is the intial upward velocity of the
 * character.
 */
void PhysxController::
start_jump(float v0) {

  nassertv(_error_type == ET_ok);

  if (_jumping == true) {
     return;
  }

  _jumping = true;
  _jump_time = 0.0f;
  _jump_v0 = v0;
}

/**
 * Leaves the jump mode.  This method is automatically called if a ground
 * collision is detected.  Usually users need not call this method.
 */
void PhysxController::
stop_jump() {

  nassertv(_error_type == ET_ok);

  if (_jumping == false) {
    return;
  }

  _jumping = false;

  // NxVec3 v = ptr()->getActor()->getLinearVelocity(); double velocity =
  // (_up_axis == NX_Z) ? v.z : v.y;

  Event *event = new Event("physx-controller-down");
  event->add_parameter(EventParameter(this));
  // event->add_parameter(EventParameter(velocity));
  EventQueue::get_global_event_queue()->queue_event(event);
}
