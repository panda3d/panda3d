/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiBehaviors.cxx
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#include "aiBehaviors.h"

#include "arrival.h"
#include "evade.h"
#include "flee.h"
#include "flock.h"
#include "obstacleAvoidance.h"
#include "pathFind.h"
#include "pathFollow.h"
#include "pursue.h"
#include "seek.h"
#include "wander.h"

using std::cout;
using std::endl;
using std::string;

static const float _PI = 3.14;

AIBehaviors::AIBehaviors() {
  _steering_force = LVecBase3(0.0, 0.0, 0.0);
  _behaviors_flags = _behaviors_flags & _none;
  _previous_conflict = false;
  _conflict = false;

  _seek_obj = nullptr;
  _flee_obj = nullptr;
  _pursue_obj = nullptr;
  _evade_obj = nullptr;
  _arrival_obj = nullptr;
  _wander_obj = nullptr;
  _flock_group = nullptr;
  _path_follow_obj = nullptr;
  _path_find_obj = nullptr;
  _obstacle_avoidance_obj = nullptr;

  turn_off("seek");
  turn_off("flee");
  turn_off("pursue");
  turn_off("evade");
  turn_off("arrival");
  turn_off("flock");
  turn_off("wander");
  turn_off("obstacle_avoidance");
}

AIBehaviors::~AIBehaviors() {

}

/**
 * Checks for conflict between steering forces.  If there is a conflict it
 * returns 'true' and sets _conflict to 'true'. If there is no conflict it
 * returns 'false' and sets _conflict to 'false'.
 */
bool AIBehaviors::is_conflict() {
  int value = int(is_on(_seek)) + int(is_on(_flee)) + int(is_on(_pursue)) + int(is_on(_evade)) + int(is_on(_wander)) + int(is_on(_flock))+ int(is_on(_obstacle_avoidance));

  if(value > 1) {
    if(_previous_conflict == false) {
      if(is_on(_seek)) {
        _seek_force *= _seek_obj->_seek_weight;
      }

      if(is_on(_flee)) {
        LVecBase3 dirn = _flee_force;
        dirn.normalize();
        _flee_force = _steering_force.length() * dirn * _flee_obj->_flee_weight;
      }

      if(is_on(_pursue)) {
        _pursue_force *= _pursue_obj->_pursue_weight;
      }

      if(is_on(_evade)) {
        LVecBase3 dirn = _evade_force;
        dirn.normalize();
        _evade_force = _steering_force.length() * dirn * _evade_obj->_evade_weight;
      }

      if(is_on(_flock)) {
        _flock_force *= _flock_weight;
      }

      if(is_on(_wander)) {
        _wander_force *= _wander_obj->_wander_weight;
      }

      _previous_conflict = true;
    }

    _conflict = true;
    return true;
  }

  _conflict = false;
  _previous_conflict = false;
  return false;
}

/**
 * This function updates the individual steering forces for each of the ai
 * characters.  These accumulated forces are eventually what comprise the
 * resultant steering force of the character.
 */

void AIBehaviors::accumulate_force(string force_type, LVecBase3 force) {

  LVecBase3 old_force;

  if(force_type == "seek") {
    old_force = _seek_force;
    _seek_force = old_force + force;
  }

  if(force_type == "flee") {
    old_force = _flee_force;
    _flee_force = old_force + force;
  }

  if(force_type == "pursue") {
    old_force = _pursue_force;
    double new_force = old_force.length() + force.length();
    _pursue_force = new_force * _pursue_obj->_pursue_direction;
  }

  if(force_type == "evade") {
    old_force = _evade_force;
    double new_force = old_force.length() + force.length();
    force.normalize();
    _evade_force = new_force * force;
  }

  if(force_type == "arrival") {
    _arrival_force = force;
  }

  if(force_type == "flock") {
    old_force = _flock_force;
    _flock_force = old_force + force;
  }

  if(force_type == "wander") {
    old_force = _wander_force;
    _wander_force = old_force + force;
  }

  if(force_type == "obstacle_avoidance") {
    old_force = _obstacle_avoidance_force;
    _obstacle_avoidance_force = old_force +force;
  }

}

/**
 * This function updates the main steering force for the ai character using
 * the accumulate function and checks for max force and arrival force.  It
 * finally returns this steering force which is accessed by the update
 * function in the AICharacter class.
 */

LVecBase3 AIBehaviors::calculate_prioritized() {
  LVecBase3 force;

  if(is_on(_seek)) {
    if(_conflict) {
      force = _seek_obj->do_seek() * _seek_obj->_seek_weight;
    }
    else {
      force = _seek_obj->do_seek();
    }
    accumulate_force("seek",force);
  }

  if(is_on(_flee_activate)) {
    for(_flee_itr = _flee_list.begin(); _flee_itr != _flee_list.end(); _flee_itr++) {
      _flee_itr->flee_activate();
    }
  }

  if(is_on(_flee)) {
    for(_flee_itr = _flee_list.begin(); _flee_itr != _flee_list.end(); _flee_itr++) {
      if(_flee_itr->_flee_activate_done) {
        if(_conflict) {
          force = _flee_itr->do_flee() * _flee_itr->_flee_weight;
        }
        else {
          force = _flee_itr->do_flee();
        }
        accumulate_force("flee",force);
      }
    }
  }

  if(is_on(_pursue)) {
    if(_conflict) {
      force = _pursue_obj->do_pursue() * _pursue_obj->_pursue_weight;
    }
    else {
      force = _pursue_obj->do_pursue();
    }
    accumulate_force("pursue",force);
  }

  if(is_on(_evade_activate)) {
    for(_evade_itr = _evade_list.begin(); _evade_itr != _evade_list.end(); _evade_itr++) {
      _evade_itr->evade_activate();
    }
  }

  if(is_on(_evade)) {
    for(_evade_itr = _evade_list.begin(); _evade_itr != _evade_list.end(); _evade_itr++) {
      if(_evade_itr->_evade_activate_done) {
        if(_conflict) {
          force = (_evade_itr->do_evade()) * (_evade_itr->_evade_weight);
        }
        else {
          force = _evade_itr->do_evade();
        }
        accumulate_force("evade",force);
      }
    }
  }

  if(is_on(_arrival_activate)) {
      _arrival_obj->arrival_activate();
  }

  if(is_on(_arrival)) {
    force = _arrival_obj->do_arrival();
    accumulate_force("arrival",force);
  }

  if(is_on(_flock_activate)) {
      flock_activate();
  }

  if(is_on(_flock)) {
    if(_conflict) {
      force = do_flock() * _flock_weight;
    }
    else {
      force = do_flock();
    }
    accumulate_force("flock",force);
  }

  if(is_on(_wander)) {
    if(_conflict) {
      force = _wander_obj->do_wander() * _wander_obj->_wander_weight;
    }
    else {
      force = _wander_obj->do_wander();
    }
    accumulate_force("wander", force);
  }

  if(is_on(_obstacle_avoidance_activate)) {
      _obstacle_avoidance_obj->obstacle_avoidance_activate();
  }

  if(is_on(_obstacle_avoidance)) {
    if(_conflict) {
      force = _obstacle_avoidance_obj->do_obstacle_avoidance();
    }
    else {
      force = _obstacle_avoidance_obj->do_obstacle_avoidance();
    }
    accumulate_force("obstacle_avoidance", force);
  }

  if(_path_follow_obj!=nullptr) {
    if(_path_follow_obj->_start) {
      _path_follow_obj->do_follow();
    }
  }

  is_conflict();

  _steering_force += _seek_force * int(is_on(_seek)) + _flee_force * int(is_on(_flee)) +
                      _pursue_force * int(is_on(_pursue)) + _evade_force * int(is_on(_evade)) +
                      _flock_force * int(is_on(_flock)) + _wander_force * int(is_on(_wander)) +
                      _obstacle_avoidance_force * int(is_on(_obstacle_avoidance));

  if(_steering_force.length() > _ai_char->get_max_force()) {
    _steering_force.normalize();
    _steering_force = _steering_force * _ai_char->get_max_force();
  }

  if(is_on(_arrival)) {
    if(_seek_obj != nullptr) {
      LVecBase3 dirn = _steering_force;
      dirn.normalize();
      _steering_force = ((_steering_force.length() - _arrival_force.length()) * dirn);
    }

    if(_pursue_obj != nullptr) {
      LVecBase3 dirn = _steering_force;
      dirn.normalize();
      _steering_force = ((_steering_force.length() - _arrival_force.length()) * _arrival_obj->_arrival_direction);
    }
  }
  return _steering_force;
}

/**
 * This function removes individual or all the AIs.
 */
void AIBehaviors::remove_ai(string ai_type) {
  switch(char_to_int(ai_type)) {
    case 0: {
              remove_ai("seek");
              remove_ai("flee");
              remove_ai("pursue");
              remove_ai("evade");
              remove_ai("arrival");
              remove_ai("flock");
              remove_ai("wander");
              remove_ai("obstacle_avoidance");
              remove_ai("pathfollow");
              break;
            }

    case 1:  {
              if(_seek_obj != nullptr) {
                turn_off("seek");
                delete _seek_obj;
                _seek_obj = nullptr;
              }
              break;
            }

    case 2: {
              while (!_flee_list.empty()) {
                turn_off("flee");
                turn_off("flee_activate");
                _flee_list.pop_front();
              }
              break;
            }

    case 3: {
              if(_pursue_obj != nullptr) {
                turn_off("pursue");
                delete _pursue_obj;
                _pursue_obj = nullptr;
              }
              break;
            }

    case 4: {
              while (!_evade_list.empty()) {
                turn_off("evade");
                turn_off("evade_activate");
                _evade_list.pop_front();
              }
              break;
            }

    case 5: {
              if(_arrival_obj != nullptr) {
                turn_off("arrival");
                turn_off("arrival_activate");
                delete _arrival_obj;
                _arrival_obj = nullptr;
              }
              break;
            }

    case 6: {
              if(_flock_group != nullptr) {
                turn_off("flock");
                turn_off("flock_activate");
                _flock_group = nullptr;
              }
              break;
            }

    case 7: {
              if(_wander_obj != nullptr) {
                turn_off("wander");
                delete _wander_obj;
                _wander_obj = nullptr;
              }
              break;
            }

    case 8: {
              if(_obstacle_avoidance_obj !=nullptr) {
                turn_off("obstacle_avoidance");
                delete _obstacle_avoidance_obj;
                _obstacle_avoidance_obj = nullptr;
              }
              break;
            }

    case 9: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_off("pursue");
                delete _pursue_obj;
                _pursue_obj = nullptr;
                delete _path_follow_obj;
                _path_follow_obj = nullptr;
              }
              break;
            }
    case 16: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_off("pursue");
                delete _pursue_obj;
                _pursue_obj = nullptr;
                delete _path_follow_obj;
                _path_follow_obj = nullptr;
              }
              break;
            }
    default:
            cout<<"Invalid option!"<<endl;
  }
}

/**
 * This function pauses individual or all the AIs.
 */
void AIBehaviors::pause_ai(string ai_type) {
  switch(char_to_int(ai_type)) {
    case 0: {
              pause_ai("seek");
              pause_ai("flee");
              pause_ai("pursue");
              pause_ai("evade");
              pause_ai("arrival");
              pause_ai("flock");
              pause_ai("wander");
              pause_ai("obstacle_avoidance");
              pause_ai("pathfollow");
              break;
            }

    case 1:  {
              if(_seek_obj != nullptr) {
                turn_off("seek");
              }
              break;
            }

    case 2: {
              for(_flee_itr = _flee_list.begin(); _flee_itr != _flee_list.end(); _flee_itr++) {
                turn_off("flee");
                turn_off("flee_activate");
              }
              break;
            }

    case 3: {
              if(_pursue_obj != nullptr) {
                turn_off("pursue");
              }
              break;
            }

    case 4: {
              for(_evade_itr = _evade_list.begin(); _evade_itr != _evade_list.end(); _evade_itr++) {
                turn_off("evade");
                turn_off("evade_activate");
              }
              break;
            }

    case 5: {
              if(_arrival_obj != nullptr) {
                turn_off("arrival");
                turn_off("arrival_activate");
              }
              break;
            }

    case 6: {
              if(_flock_group != nullptr) {
                turn_off("flock");
                turn_off("flock_activate");
              }
              break;
            }

    case 7: {
              if(_wander_obj != nullptr) {
                turn_off("wander");
              }
              break;
            }

    case 8: {
              if(_obstacle_avoidance_obj != nullptr) {
                turn_off("obstacle_avoidance");
                turn_off("obstacle_avoidance_activate");
              }
              break;
            }

    case 9: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_off("pursue");
                _path_follow_obj->_start = false;
              }
              break;
            }
    case 16: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_off("pursue");
                _path_follow_obj->_start = false;
              }
              break;
            }
    default:
            cout<<"Invalid option!"<<endl;
  }
}

/**
 * This function resumes individual or all the AIs
 */
void AIBehaviors::resume_ai(string ai_type) {
  switch(char_to_int(ai_type)) {
    case 0: {
              resume_ai("seek");
              resume_ai("flee");
              resume_ai("pursue");
              resume_ai("evade");
              resume_ai("arrival");
              resume_ai("flock");
              resume_ai("wander");
              resume_ai("obstacle_avoidance");
              resume_ai("pathfollow");
              break;
            }

    case 1:  {
              if(_seek_obj != nullptr) {
                turn_on("seek");
              }
              break;
            }

    case 2: {
              for(_flee_itr = _flee_list.begin(); _flee_itr != _flee_list.end(); _flee_itr++) {
                turn_on("flee");
              }
              break;
            }

    case 3: {
              if(_pursue_obj != nullptr) {
                turn_on("pursue");
              }
              break;
            }

    case 4: {
              for(_evade_itr = _evade_list.begin(); _evade_itr != _evade_list.end(); _evade_itr++) {
                turn_on("evade");
              }
              break;
            }

    case 5: {
              if(_arrival_obj != nullptr) {
                turn_on("arrival");
              }
              break;
            }

    case 6: {
              if(_flock_group != nullptr) {
                turn_on("flock");
              }
              break;
            }

    case 7: {
              if(_wander_obj != nullptr) {
                turn_on("wander");
              }
              break;
            }

    case 8: {
              if(_obstacle_avoidance_obj != nullptr) {
                turn_on("obstacle_avoidance");
              }
              break;
            }

    case 9: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_on("pursue");
                _path_follow_obj->_start = true;
              }
              break;
            }
    case 16: {
              if(_pursue_obj != nullptr && _path_follow_obj != nullptr) {
                turn_off("pursue");
                _path_follow_obj->_start = false;
              }
              break;
            }
    default:
            cout<<"Invalid option!"<<endl;
  }
}

/**
 * This function activates seek and makes an object of the Seek class.  This
 * is the function we want the user to call for seek to be done.  This
 * function is overloaded to accept a NodePath or an LVecBase3.
 */
void AIBehaviors::seek(NodePath target_object, float seek_wt) {
  _seek_obj = new Seek(_ai_char, target_object, seek_wt);
  turn_on("seek");
}

void AIBehaviors::seek(LVecBase3 pos, float seek_wt) {
  _seek_obj = new Seek(_ai_char, pos, seek_wt);
  turn_on("seek");
}

/**
 * This function activates flee_activate and creates an object of the Flee
 * class.  This function is overloaded to accept a NodePath or an LVecBase3.
 */
void AIBehaviors::flee(NodePath target_object, double panic_distance, double relax_distance, float flee_wt) {
  _flee_obj = new Flee(_ai_char, target_object, panic_distance, relax_distance, flee_wt);
  _flee_list.insert(_flee_list.end(), *_flee_obj);

  turn_on("flee_activate");
}

void AIBehaviors::flee(LVecBase3 pos, double panic_distance, double relax_distance, float flee_wt) {
  _flee_obj = new Flee(_ai_char, pos, panic_distance, relax_distance, flee_wt);
  _flee_list.insert(_flee_list.end(), *_flee_obj);

  turn_on("flee_activate");
}

/**
 * This function activates pursue.  This is the function we want the user to
 * call for pursue to be done.
 */
void AIBehaviors::pursue(NodePath target_object, float pursue_wt) {
  _pursue_obj = new Pursue(_ai_char, target_object, pursue_wt);

  turn_on("pursue");
}

/**
 * This function activates evade_activate.
 */
void AIBehaviors::evade(NodePath target_object, double panic_distance, double relax_distance, float evade_wt) {
  _evade_obj = new Evade(_ai_char, target_object, panic_distance, relax_distance, evade_wt);
  _evade_list.insert(_evade_list.end(), *_evade_obj);

  turn_on("evade_activate");
}

/**
 * This function activates arrival.  This is the function we want the user to
 * call for arrival to be done.
 */
void AIBehaviors::arrival(double distance) {
  if(_pursue_obj) {
    _arrival_obj = new Arrival(_ai_char, distance);
    _arrival_obj->_arrival_type = true;
    turn_on("arrival_activate");
  }
  else if(_seek_obj) {
    _arrival_obj = new Arrival(_ai_char, distance);
    _arrival_obj->_arrival_type = false;
    turn_on("arrival_activate");
  }
  else {
    cout<<"Note: A Seek or Pursue behavior is required to use Arrival behavior."<<endl;
  }
}

/**
 * This function activates flock.  This is the function we want the user to
 * call for flock to be done.
 */
void AIBehaviors::flock(float flock_wt) {
  _flock_weight = flock_wt;

  _flock_done = false;
  turn_on("flock_activate");
}

/**
 * This function checks whether any other behavior exists to work with flock.
 * When this is true, it calls the do_flock function.
 */
void AIBehaviors::flock_activate() {
  if(is_on(_seek) || is_on(_flee) || is_on(_pursue) || is_on(_evade) || is_on(_wander)) {
      turn_off("flock_activate");
      turn_on("flock");
  }
}

/**
 * This function contains the logic for flocking behavior.  This is an
 * emergent behavior and is obtained by combining three other behaviors which
 * are separation, cohesion and alignment based on Craig Reynold's algorithm.
 * Also, this behavior does not work by itself.  It works only when combined
 * with other steering behaviors such as wander, pursue, evade, seek and flee.
 */
LVecBase3 AIBehaviors::do_flock() {

  // ! Initialize variables required to compute the flocking force on the ai
  // char.
  unsigned int neighbor_count = 0;
  LVecBase3 separation_force = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 alignment_force = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 cohesion_force = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 avg_neighbor_heading = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 total_neighbor_heading = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 avg_center_of_mass = LVecBase3(0.0, 0.0, 0.0);
  LVecBase3 total_center_of_mass = LVecBase3(0.0, 0.0, 0.0);

  // ! Loop through all the other AI units in the flock to check if they are
  // neigbours.
  for(unsigned int i = 0; i < _flock_group->_ai_char_list.size(); i++) {
    if(_flock_group->_ai_char_list[i]->_name != _ai_char->_name) {

      // ! Using visibilty cone to detect neighbors.
      LVecBase3 dist_vect = _flock_group->_ai_char_list[i]->_ai_char_np.get_pos() - _ai_char->_ai_char_np.get_pos();
      LVecBase3 ai_char_heading = _ai_char->get_velocity();
      ai_char_heading.normalize();

      // ! Check if the current unit is a neighbor.
      if(dist_vect.dot(ai_char_heading) > ((dist_vect.length()) * (ai_char_heading.length()) * cos(_flock_group->_flock_vcone_angle * (_PI / 180)))
        && (dist_vect.length() < _flock_group->_flock_vcone_radius)) {
          // ! Separation force calculation.
          LVecBase3 ai_char_to_units = _ai_char->_ai_char_np.get_pos() - _flock_group->_ai_char_list[i]->_ai_char_np.get_pos();
          float to_units_dist = ai_char_to_units.length();
          ai_char_to_units.normalize();
          separation_force += (ai_char_to_units / to_units_dist);

          // ! Calculating the total heading and center of mass of all the
          // neighbors.
          LVecBase3 neighbor_heading = _flock_group->_ai_char_list[i]->get_velocity();
          neighbor_heading.normalize();
          total_neighbor_heading += neighbor_heading;
          total_center_of_mass += _flock_group->_ai_char_list[i]->_ai_char_np.get_pos();

          // ! Update the neighbor count.
          ++neighbor_count;
      }
    }
  }

  if(neighbor_count > 0) {
    // ! Alignment force calculation
    avg_neighbor_heading = total_neighbor_heading / neighbor_count;
    LVector3 ai_char_heading = _ai_char->get_velocity();
    ai_char_heading.normalize();
    avg_neighbor_heading -= ai_char_heading;
    avg_neighbor_heading.normalize();
    alignment_force = avg_neighbor_heading;

    // ! Cohesion force calculation
    avg_center_of_mass = total_center_of_mass / neighbor_count;
    LVecBase3 cohesion_dir = avg_center_of_mass - _ai_char->_ai_char_np.get_pos();
    cohesion_dir.normalize();
    cohesion_force = cohesion_dir * _ai_char->_movt_force;
  }
  else if(is_on(_seek) || is_on(_flee) || is_on(_pursue) || is_on(_evade) || is_on(_wander)) {
    _flock_done = true;
    turn_off("flock");
    turn_on("flock_activate");
    return(LVecBase3(0.0, 0.0, 0.0));
  }

  // ! Calculate the resultant force on the ai character by taking into
  // account the separation, alignment and cohesion ! forces along with their
  // corresponding weights.
  return (separation_force * _flock_group->_separation_wt + avg_neighbor_heading * _flock_group->_alignment_wt
    + cohesion_force * _flock_group->_cohesion_wt);
}

/**
 * This function activates wander.  This is the function we want the user to
 * call for flock to be done.
 */
void AIBehaviors::wander(double wander_radius, int flag, double aoe, float wander_weight) {
  _wander_obj = new Wander(_ai_char, wander_radius, flag, aoe, wander_weight);
  turn_on("wander");
}

/**
 * This function activates obstacle avoidance for a given character.  This is
 * the function we want the user to call for obstacle avoidance to be
 * performed.
 */
void AIBehaviors::obstacle_avoidance(float obstacle_avoidance_weight) {
  _obstacle_avoidance_obj = new ObstacleAvoidance(_ai_char, obstacle_avoidance_weight);
  turn_on("obstacle_avoidance_activate");
}

/**
 * This function activates path following.  This is the function we want the
 * user to call for path following.
 */
void AIBehaviors::path_follow(float follow_wt) {
  _path_follow_obj = new PathFollow(_ai_char, follow_wt);
}

/**
 * This function adds positions to the path to follow.
 */
void AIBehaviors::add_to_path(LVecBase3 pos) {
  _path_follow_obj->add_to_path(pos);
}

/**
 * This function starts the path follower.
 */

void AIBehaviors::start_follow(string type) {
  _path_follow_obj->start(type);
}

/**
 * This function activates path finding in the character.  This function
 * accepts the meshdata in .csv format.
 *
 */
void AIBehaviors::init_path_find(const char* navmesh_filename) {
  _path_find_obj = new PathFind(_ai_char);
  _path_find_obj->set_path_find(navmesh_filename);
}

/**
 * This function checks for the source and target in the navigation mesh for
 * its availability and then finds the best path via the A* algorithm Then it
 * calls the path follower to make the object follow the path.
 */
void AIBehaviors::path_find_to(LVecBase3 pos, string type) {
  _path_find_obj->path_find(pos, type);
}

/**
 * This function checks for the source and target in the navigation mesh for
 * its availability and then finds the best path via the A* algorithm Then it
 * calls the path follower to make the object follow the path.
 */
void AIBehaviors::path_find_to(NodePath target, string type) {
  _path_find_obj->path_find(target, type);
}

/**
 * This function allows the user to dynamically add obstacles to the game
 * environment.  The function will update the nodes within the bounding volume
 * of the obstacle as non-traversable.  Hence will not be considered by the
 * pathfinding algorithm.
 */
void AIBehaviors::add_static_obstacle(NodePath obstacle) {
  _path_find_obj->add_obstacle_to_mesh(obstacle);
}

/**
 * This function starts the pathfinding obstacle navigation for the passed in
 * obstacle.
 */
void AIBehaviors::add_dynamic_obstacle(NodePath obstacle) {
  _path_find_obj->dynamic_avoid(obstacle);
}

/**
 * This function returns the status of an AI Type whether it is active, paused
 * or disabled.  It returns -1 if an invalid string is passed.
 */
string AIBehaviors::behavior_status(string ai_type) {
  switch(char_to_int(ai_type)) {
    case 1:
      if(_seek_obj) {
        if(is_on(_seek)) {
                    return "active";
        }
        else {
            if(_seek_obj->_seek_done) {
            return "done";
          }
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 2:
      if(_flee_obj) {
        if(is_on(_flee)) {
          unsigned int i = 0;
          for(_flee_itr = _flee_list.begin(); _flee_itr != _flee_list.end(); _flee_itr++) {
            if(_flee_itr->_flee_done == true) {
              ++i;
            }
          }
          if(i == _flee_list.size()) {
            return "done";
          }
          else {
            return "active";
          }
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 3:
      if(_pursue_obj) {
        if(is_on(_pursue)) {
          if(_pursue_obj->_pursue_done) {
            return "done";
          }
          else {
            return "active";
          }
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 4:
      if(_evade_obj) {
        if(is_on(_evade)) {
          unsigned int i = 0;
          for(_evade_itr = _evade_list.begin(); _evade_itr != _evade_list.end(); _evade_itr++) {
            if(_evade_itr->_evade_done == true) {
              ++i;
            }
          }
          if(i == _evade_list.size()) {
            return "done";
          }
          else {
            return "active";
          }
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 5:
      if(_arrival_obj) {
        if(is_on(_arrival)) {
          if(_arrival_obj->_arrival_done) {
            return "done";
          }
          else {
            return "active";
          }
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 6:
      if(_flock_group) {
        if(is_on(_flock)) {
          if(_flock_done) {
            return "done";
          }
          else {
            return "active";
          }
          return "active";
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

    case 7:
      if(_wander_obj) {
        if(is_on(_wander)) {
          return "active";
        }
        else {
          return "paused";
        }
      }
      else {
          return "disabled";
      }
      break;

      case 8:
        if(_obstacle_avoidance_obj) {
          if(is_on(_obstacle_avoidance)) {
            return "active";
          }
          else {
            return "paused";
          }
        }
        else {
          return "disabled";
        }
        break;

      case 9:
        if(_path_follow_obj) {
          if(is_on("pathfollow")) {
            if(_pursue_obj->_pursue_done) {
              return "done";
            }
            else {
              return "active";
            }
          }
          else {
            return "paused";
          }
        }
        else {
          return "disabled";
        }
        break;

      case 16:
        if(_path_find_obj) {
          if(is_on("pathfind")) {
            if(_pursue_obj->_pursue_done) {
              return "done";
            }
            else {
              return "active";
            }
          }
          else {
            return "paused";
          }
        }
        else {
          return "disabled";
        }
        break;

      case 10:
        if(_seek_obj || _flee_obj || _pursue_obj || _evade_obj || _arrival_obj || _flock_group || _wander_obj || _obstacle_avoidance_obj || _path_follow_obj) {
          if(is_on(_seek) || is_on(_flee) || is_on(_pursue)|| is_on(_evade) || is_on(_arrival) || is_on(_flock) || is_on(_wander)
            || is_on(_obstacle_avoidance) || is_on("pathfollow") || is_on("pathfinding")) {
            return "active";
          }
          else {
            return "paused";
          }
        }
        else {
          return "disabled";
        }
        break;

      default:
        cout<<"Invalid value!"<<endl;
        return "invalid";
    }
  }

/**
 * This function is used to derive int values from the ai types strings.
 * Returns -1 if an invalid string is passed.
 */
int AIBehaviors::char_to_int(string ai_type) {
  if(ai_type == "all") {
    return 0;
  }
  else if(ai_type == "seek") {
    return 1;
  }
  else if(ai_type == "flee") {
    return 2;
  }
  else if(ai_type == "pursue") {
    return 3;
  }
  else if(ai_type == "evade") {
    return 4;
  }
  else if(ai_type == "arrival") {
    return 5;
  }
  else if(ai_type == "flock") {
    return 6;
  }
  else if(ai_type == "wander") {
    return 7;
  }
  else if(ai_type == "obstacle_avoidance") {
    return 8;
  }
  else if(ai_type == "pathfollow") {
    return 9;
  }
  else if(ai_type == "any") {
    return 10;
  }
  else if(ai_type == "flee_activate") {
    return 11;
  }
  else if(ai_type == "evade_activate") {
    return 12;
  }
  else if(ai_type == "arrival_activate") {
    return 13;
  }
  else if(ai_type == "flock_activate") {
    return 14;
  }
  else if(ai_type == "obstacle_avoidance_activate") {
    return 15;
  }
  else if(ai_type == "path_finding") {
    return 16;
  }

  return -1;
}

/**
 * This function turns on any aiBehavior which is passed as a string.
 */
void AIBehaviors::turn_on(string ai_type) {
  switch(char_to_int(ai_type)) {
    case 1:  {
              _behaviors_flags |= _seek;
              break;
            }
    case 2: {
              _behaviors_flags |= _flee;
              break;
            }
    case 3: {
              _behaviors_flags |= _pursue;
              break;
            }
    case 4: {
              _behaviors_flags |= _evade;
              break;
            }
    case 5: {
              _behaviors_flags |= _arrival;
              break;
            }
    case 6: {
              _behaviors_flags |= _flock;
              break;
            }
    case 7: {
              _behaviors_flags |= _wander;
              break;
            }
    case 8: {
              _behaviors_flags |= _obstacle_avoidance;
              break;
            }
    case 11:{
              _behaviors_flags |= _flee_activate;
              break;
            }
    case 12:{
              _behaviors_flags |= _evade_activate;
              break;
            }
    case 13:{
              _behaviors_flags |= _arrival_activate;
              break;
            }
    case 14:{
              _behaviors_flags |= _flock_activate;
              break;
            }
    case 15:{
              _behaviors_flags |= _obstacle_avoidance_activate;
              break;
            }
    default:
            cout<<"Invalid option!"<<endl;
  }
}

/**
 * This function turns off any aiBehavior which is passed as a string.
 */
void AIBehaviors::turn_off(string ai_type) {
switch(char_to_int(ai_type)) {
    case 1:  {
              if (is_on(_seek)) {
                _behaviors_flags ^= _seek;
              }
              _seek_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 2: {
              if (is_on(_flee)) {
                _behaviors_flags ^= _flee;
              }
              _flee_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 3: {
              if(is_on(_pursue)) {
                _behaviors_flags ^= _pursue;
              }
              _pursue_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 4: {
              if(is_on(_evade)) {
                _behaviors_flags ^= _evade;
              }
              _evade_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 5: {
              if (is_on(_arrival)) {
                  _behaviors_flags ^= _arrival;
                }
                _arrival_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 6: {
              if(is_on(_flock)) {
                _behaviors_flags ^= _flock;
              }
              _flock_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 7: {
              if(is_on(_wander)) {
                _behaviors_flags ^= _wander;
              }
              _wander_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 8: {
              if(is_on(_obstacle_avoidance)) {
                _behaviors_flags ^= _obstacle_avoidance;
              }
              _obstacle_avoidance_force = LVecBase3(0.0f, 0.0f, 0.0f);
              break;
            }
    case 9:{
              turn_off("pursue");
              break;
            }
    case 11:{
              if (is_on(_flee_activate)) {
                _behaviors_flags ^= _flee_activate;
              }
              break;
            }
    case 12:{
              if (is_on(_evade_activate)) {
                _behaviors_flags ^= _evade_activate;
              }
              break;
            }
    case 13:{
              if (is_on(_arrival_activate)) {
                _behaviors_flags ^= _arrival_activate;
              }
              break;
            }
    case 14:{
              if (is_on(_flock_activate)) {
                _behaviors_flags ^= _flock_activate;
              }
              break;
            }
    case 15:{
              if (is_on(_obstacle_avoidance_activate)) {
                _behaviors_flags ^= _obstacle_avoidance_activate;
              }
              break;
            }
    case 16:{
              turn_off("pathfollow");
              break;
            }
    default:
            cout<<"Invalid option!"<<endl;
  }
}

/**
 * This function returns true if an aiBehavior is on
 */
bool AIBehaviors::is_on(_behavior_type bt)  {
  return (_behaviors_flags & bt) == bt;
}

/**
 * This function returns true if pathfollow or pathfinding is on
 */
bool AIBehaviors::is_on(string ai_type) {
  if(ai_type == "pathfollow") {
    if(_path_follow_obj) {
      return (is_on(_pursue) && _path_follow_obj->_start);
    }
    else {
      return false;
    }
  }

  if(ai_type == "pathfinding") {
    if(_path_follow_obj && _path_find_obj) {
      return (is_on(_pursue) && _path_follow_obj->_start);
    }
    else {
      return false;
    }
  }

  return false;
}

/**
 * This function returns true if an aiBehavior is off
 */
bool AIBehaviors::is_off(_behavior_type bt)  {
  return ((_behaviors_flags | bt) == bt);
}

/**
 * This function returns true if pathfollow or pathfinding is off
 */
bool AIBehaviors::is_off(string ai_type) {
  if(ai_type == "pathfollow") {
    if(_path_follow_obj && _path_follow_obj->_start) {
      return true;
    }
    else {
      return false;
    }
  }

  if(ai_type == "pathfinding") {
    if(_path_find_obj && _path_follow_obj && _path_follow_obj->_start) {
      return true;
    }
    else {
      return false;
    }
  }

  cout<<"You passed an invalid string, defaulting return value to false!"<<endl;
  return false;
}
