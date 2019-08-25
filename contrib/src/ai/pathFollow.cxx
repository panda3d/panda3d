/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pathFind.cxx
 * @author Deepak, John, Navin
 * @date 2009-10-24
 */

#include "pathFollow.h"

#include "pathFind.h"

PathFollow::PathFollow(AICharacter *ai_ch, float follow_wt) {
    _follow_weight = follow_wt;
  _curr_path_waypoint = -1;
  _start = false;
  _ai_char = ai_ch;
  _myClock = ClockObject::get_global_clock();
}

PathFollow::~PathFollow() {
}

/**
 * This function adds the positions generated from a pathfind or a simple path
 * follow behavior to the _path list.
 */
void PathFollow::add_to_path(LVecBase3 pos) {
    _path.push_back(pos);
}

/**
 * This function initiates the path follow behavior.
 */
void PathFollow::start(std::string type) {
    _type = type;
  _start = true;
  if(_path.size() > 0) {
    _curr_path_waypoint = _path.size() - 1;
    _dummy = _ai_char->_window_render.attach_new_node("dummy");
    _dummy.set_pos(_path[_curr_path_waypoint]);
    _ai_char->_steering->pursue(_dummy, _follow_weight);
    _time = _myClock->get_real_time();
  }
}

/**
 * This function allows continuous path finding by ai chars.  There are 2 ways
 * in which this is implemented.  1. The character re-calculates the optimal
 * path everytime the target changes its position.  Less computationally
 * expensive.  2. The character continuosly re-calculates its optimal path to
 * the target.  This is used in a scenario where the ai chars have to avoid
 * other ai chars.  More computationally expensive.
 */
void PathFollow::do_follow() {
  if((_myClock->get_real_time() - _time) > 0.5) {
      if(_type=="pathfind") {
      // This 'if' statement when 'true' causes the path to be re-calculated
      // irrespective of target position.  This is done when _dynamice_avoid
      // is active.  More computationally expensive.
      if(_ai_char->_steering->_path_find_obj->_dynamic_avoid) {
        _ai_char->_steering->_path_find_obj->do_dynamic_avoid();
        if(check_if_possible()) {
          _path.clear();
          _ai_char->_steering->_path_find_obj->path_find(_ai_char->_steering->_path_find_obj->_path_find_target);
          // Ensure that the path size is not 0.
          if(_path.size() > 0) {
            _curr_path_waypoint = _path.size() - 1;
            _dummy.set_pos(_path[_curr_path_waypoint]);
          }
          else {
          // Refresh the _curr_path_waypoint value if path size is <= 0.
          _curr_path_waypoint = -1;
          }
        }
      }
      // This 'if' statement causes the path to be re-calculated only when
      // there is a change in target position.  Less computationally
      // expensive.
      else if(_ai_char->_steering->_path_find_obj->_path_find_target.get_pos(_ai_char->_window_render)
        != _ai_char->_steering->_path_find_obj->_prev_position) {
        if(check_if_possible()) {
          _path.clear();
          _ai_char->_steering->_path_find_obj->path_find(_ai_char->_steering->_path_find_obj->_path_find_target);
          // Ensure that the path size is not 0.
          if(_path.size() > 0) {
            _curr_path_waypoint = _path.size() - 1;
            _dummy.set_pos(_path[_curr_path_waypoint]);
          }
          else {
            // Refresh the _curr_path_waypoint value if path size is 0.
            _curr_path_waypoint = -1;
          }
        }
      }
      _time = _myClock->get_real_time();
    }
    }

    if(_curr_path_waypoint > 0) {
    double distance = (_path[_curr_path_waypoint] - _ai_char->_ai_char_np.get_pos(_ai_char->_window_render)).length();

    if(distance < 5) {
      _curr_path_waypoint--;
      _dummy.set_pos(_path[_curr_path_waypoint]);
    }
  }
}

/**
 * This function checks if the current positions of the ai char and the target
 * char can be used to generate an optimal path.
 */
bool PathFollow::check_if_possible() {
  Node* src = find_in_mesh(_ai_char->_steering->_path_find_obj->_nav_mesh, _ai_char->_ai_char_np.get_pos(_ai_char->_window_render), _ai_char->_steering->_path_find_obj->_grid_size);
  LVecBase3 _prev_position = _ai_char->_steering->_path_find_obj->_path_find_target.get_pos(_ai_char->_window_render);
  Node* dst = find_in_mesh(_ai_char->_steering->_path_find_obj->_nav_mesh, _prev_position, _ai_char->_steering->_path_find_obj->_grid_size);

  if(src && dst) {
    return true;
  }
  else {
    return false;
  }
}
