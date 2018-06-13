/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiCharacter.cxx
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#include "aiCharacter.h"

AICharacter::AICharacter(std::string model_name, NodePath model_np, double mass, double movt_force, double max_force) {
  _name = model_name;
  _ai_char_np = model_np;

  _mass = mass;
  _max_force = max_force;
  _movt_force = movt_force;

  _velocity = LVecBase3(0.0, 0.0, 0.0);
  _steering_force = LVecBase3(0.0, 0.0, 0.0);

  _world = nullptr;

  _steering = new AIBehaviors();
  _steering->_ai_char = this;

  _pf_guide = false;
}

AICharacter::~AICharacter() {
  nassertv(_world == nullptr);
}

/**
 * Each character's update will update its AI and physics based on his
 * resultant steering force.  This also makes the character look in the
 * direction of the force.
 */
void AICharacter::
update() {
  if (!_steering->is_off(_steering->_none)) {
    LVecBase3 old_pos = _ai_char_np.get_pos();
    LVecBase3 steering_force = _steering->calculate_prioritized();
    LVecBase3 acceleration = steering_force / _mass;

    _velocity = acceleration;

    LVecBase3 direction = _steering->_steering_force;
    direction.normalize();

    _ai_char_np.set_pos(old_pos + _velocity) ;

    if (steering_force.length() > 0) {
      _ai_char_np.look_at(old_pos + (direction * 5));
      _ai_char_np.set_h(_ai_char_np.get_h() + 180);
      _ai_char_np.set_p(-_ai_char_np.get_p());
      _ai_char_np.set_r(-_ai_char_np.get_r());
    }
  } else {
    _steering->_steering_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_seek_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_flee_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_pursue_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_evade_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_arrival_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_flock_force = LVecBase3(0.0, 0.0, 0.0);
    _steering->_wander_force = LVecBase3(0.0, 0.0, 0.0);
  }
}

LVecBase3 AICharacter::get_velocity() {
  return _velocity;
}

void AICharacter::set_velocity(LVecBase3 velocity) {
  _velocity = velocity;
}

double AICharacter::get_mass() {
  return _mass;
}

void AICharacter::set_mass(double m) {
  _mass = m;
}

double AICharacter::get_max_force() {
  return _max_force;
}

void AICharacter::set_max_force(double max_force) {
  _max_force = max_force;
}

NodePath AICharacter::get_node_path() {
  return _ai_char_np;
}

void AICharacter::set_node_path(NodePath np) {
  _ai_char_np = np;
}

AIBehaviors * AICharacter::get_ai_behaviors() {
  return _steering;
}

void AICharacter::set_char_render(NodePath render) {
  _window_render = render;
}

NodePath AICharacter::get_char_render() {
  return _window_render;
}

void AICharacter::set_pf_guide(bool pf_guide) {
  _pf_guide = pf_guide;
}
