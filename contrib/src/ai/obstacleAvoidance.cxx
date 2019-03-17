/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file obstacleAvoidance.cxx
 * @author Deepak, John, Navin
 * @date 2009-11-10
 */

#include "obstacleAvoidance.h"

#include "aiWorld.h"

ObstacleAvoidance::
ObstacleAvoidance(AICharacter *ai_char, float feeler_length) {
  _ai_char = ai_char;
  _feeler = feeler_length;
}

ObstacleAvoidance::
~ObstacleAvoidance() {
}

/**
 * This function checks if an obstacle is near to the AICharacter and if an
 * obstacle is detected returns true
 */
bool ObstacleAvoidance::
obstacle_detection() {
  // Calculate the volume of the AICharacter with respect to render
  PT(BoundingVolume) np_bounds = _ai_char->get_node_path().get_bounds();
  CPT(BoundingSphere) np_sphere = np_bounds->as_bounding_sphere();
  LVecBase3 avoidance(0.0, 0.0, 0.0);
  double distance = 0x7fff ;
  double expanded_radius = 0;
  LVecBase3 to_obstacle;
  for(unsigned int i = 0; i < _ai_char->_world->_obstacles.size(); ++i) {
    PT(BoundingVolume) bounds = _ai_char->_world->_obstacles[i].get_bounds();
    CPT(BoundingSphere) bsphere = bounds->as_bounding_sphere();
    LVecBase3 near_obstacle = _ai_char->_world->_obstacles[i].get_pos() - _ai_char->get_node_path().get_pos();
    // Check if it's the nearest obstacle, If so initialize as the nearest
    // obstacle
    if((near_obstacle.length() < distance) && (_ai_char->_world->_obstacles[i].get_pos() != _ai_char->get_node_path().get_pos())) {
      _nearest_obstacle = _ai_char->_world->_obstacles[i];
      distance = near_obstacle.length();
      expanded_radius = bsphere->get_radius() + np_sphere->get_radius();
    }
  }

  LVecBase3 feeler = _feeler * _ai_char->get_char_render().get_relative_vector(_ai_char->get_node_path(), LVector3::forward());
  feeler.normalize();
  feeler *= (expanded_radius + np_sphere->get_radius()) ;
  to_obstacle = _nearest_obstacle.get_pos() - _ai_char->get_node_path().get_pos();
  LVector3 line_vector = _ai_char->get_char_render().get_relative_vector(_ai_char->get_node_path(), LVector3::forward());
  LVecBase3 project = (to_obstacle.dot(line_vector) * line_vector) / line_vector.length_squared();
  LVecBase3 perp = project - to_obstacle;

  // If the nearest obstacle will collide with our AICharacter then send
  // obstacle detection as true
  if (_nearest_obstacle && (perp.length() < expanded_radius - np_sphere->get_radius()) && (project.length() < feeler.length())) {
    return true;
  }
  return false;
}

/**
 * This function activates obstacle_avoidance if a obstacle is detected
 */
void ObstacleAvoidance::
obstacle_avoidance_activate() {
  if (obstacle_detection()) {
    _ai_char->_steering->turn_off("obstacle_avoidance_activate");
    _ai_char->_steering->turn_on("obstacle_avoidance");
  }
}

/**
 * This function returns the force necessary by the AICharacter to avoid the
 * nearest obstacle detected by obstacle_detection function NOTE : This
 * assumes the obstacles are spherical
 */
LVecBase3 ObstacleAvoidance::
do_obstacle_avoidance() {
  LVecBase3 offset = _ai_char->get_node_path().get_pos() - _nearest_obstacle.get_pos();
  PT(BoundingVolume) bounds =_nearest_obstacle.get_bounds();
  CPT(BoundingSphere) bsphere = bounds->as_bounding_sphere();
  PT(BoundingVolume) np_bounds = _ai_char->get_node_path().get_bounds();
  CPT(BoundingSphere) np_sphere = np_bounds->as_bounding_sphere();

  if (obstacle_detection()) {
    LVecBase3 direction = _ai_char->get_char_render().get_relative_vector(_ai_char->get_node_path(), LVector3::forward());
    direction.normalize();
    float forward_component = offset.dot(direction);
    LVecBase3 projection = forward_component * direction;
    LVecBase3 perpendicular_component = offset - projection;
    double p = perpendicular_component.length();
    perpendicular_component.normalize();
    LVecBase3   avoidance = perpendicular_component;
    // The more closer the obstacle, the more force it generates
    avoidance = (avoidance * _ai_char->get_max_force() * _ai_char->_movt_force) / (p + 0.01);
    return avoidance;
  }
  _ai_char->_steering->turn_on("obstacle_avoidance_activate");
  _ai_char->_steering->turn_off("obstacle_avoidance");
  return LVecBase3(0, 0, 0);
}
