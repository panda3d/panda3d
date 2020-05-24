/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiBehaviors.h
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#ifndef _AIBEHAVIORS_H
#define _AIBEHAVIORS_H

#include "aiGlobals.h"

class AICharacter;
class Seek;
class Flee;
class Pursue;
class Evade;
class Arrival;
class Flock;
class Wander;
class PathFollow;
class PathFind;
class ObstacleAvoidance;

#include "flee.h"
#include "evade.h"

typedef std::list<Flee, std::allocator<Flee> > ListFlee;
typedef std::list<Evade, std::allocator<Evade> > ListEvade;

/**
 * This class implements all the steering behaviors of the AI framework, such
 * as seek, flee, pursue, evade, wander and flock.  Each steering behavior has
 * a weight which is used when more than one type of steering behavior is
 * acting on the same ai character.  The weight decides the contribution of
 * each type of steering behavior.  The AICharacter class has a handle to an
 * object of this class and this allows to invoke the steering behaviors via
 * the AICharacter.  This class also provides functionality such as pausing,
 * resuming and removing the AI behaviors of an AI character at anytime.
 */
class EXPCL_PANDAAI AIBehaviors {
public:
  enum _behavior_type {
      _none = 0x00000,
      _seek = 0x00002,
      _flee = 0x00004,
      _flee_activate = 0x00100,
      _arrival = 0x00008,
      _arrival_activate = 0x01000,
      _wander = 0x00010,
      _pursue = 0x00040,
      _evade = 0x00080,
      _evade_activate = 0x00800,
      _flock = 0x00200,
      _flock_activate = 0x00400,
      _obstacle_avoidance = 0x02000,
      _obstacle_avoidance_activate = 0x04000
  };

  AICharacter *_ai_char;
  Flock *_flock_group;

  int _behaviors_flags;
  LVecBase3 _steering_force;

  Seek *_seek_obj;
  LVecBase3 _seek_force;

  Flee *_flee_obj;
  LVecBase3 _flee_force;

  // ! This list is used if the ai character needs to flee from multiple
  // onjects.
  ListFlee _flee_list;
  ListFlee::iterator _flee_itr;

  Pursue *_pursue_obj;
  LVecBase3 _pursue_force;

  Evade *_evade_obj;
  LVecBase3 _evade_force;

  // ! This list is used if the ai character needs to evade from multiple
  // onjects.
  ListEvade _evade_list;
  ListEvade::iterator _evade_itr;

  Arrival *_arrival_obj;
  LVecBase3 _arrival_force;

  // ! Since Flock is a collective behavior the variables are declared within
  // the AIBehaviors class.
  float _flock_weight;
  LVecBase3 _flock_force;
  bool _flock_done;

  Wander * _wander_obj;
  LVecBase3 _wander_force;

  ObstacleAvoidance *_obstacle_avoidance_obj;
  LVecBase3 _obstacle_avoidance_force;

  PathFollow *_path_follow_obj;

  PathFind *_path_find_obj;

  bool _conflict, _previous_conflict;

  AIBehaviors();
  ~AIBehaviors();

  bool is_on(_behavior_type bt);
  bool is_on(std::string ai_type); // special cases for pathfollow and pathfinding
  bool is_off(_behavior_type bt);
  bool is_off(std::string ai_type); // special cases for pathfollow and pathfinding
  void turn_on(std::string ai_type);
  void turn_off(std::string ai_type);

  bool is_conflict();

  void accumulate_force(std::string force_type, LVecBase3 force);
  LVecBase3 calculate_prioritized();

  void flock_activate();
  LVecBase3 do_flock();

  int char_to_int(std::string ai_type);

PUBLISHED:
  void seek(NodePath target_object, float seek_wt = 1.0);
  void seek(LVecBase3 pos, float seek_wt = 1.0);

  void flee(NodePath target_object, double panic_distance = 10.0, double relax_distance = 10.0, float flee_wt = 1.0);
  void flee(LVecBase3 pos, double panic_distance = 10.0, double relax_distance = 10.0, float flee_wt = 1.0);

  void pursue(NodePath target_object, float pursue_wt = 1.0);

  void evade(NodePath target_object, double panic_distance = 10.0, double relax_distance = 10.0, float evade_wt = 1.0);

  void arrival(double distance = 10.0);

  void flock(float flock_wt);

  void wander(double wander_radius = 5.0, int flag =0, double aoe = 0.0, float wander_weight = 1.0);

  void obstacle_avoidance(float feeler_length = 1.0);

  void path_follow(float follow_wt);
  void add_to_path(LVecBase3 pos);
  void start_follow(std::string type = "normal");

  // should have different function names.
  void init_path_find(const char* navmesh_filename);
  void path_find_to(LVecBase3 pos, std::string type = "normal");
  void path_find_to(NodePath target, std::string type = "normal");
  void add_static_obstacle(NodePath obstacle);
  void add_dynamic_obstacle(NodePath obstacle);


  void remove_ai(std::string ai_type);
  void pause_ai(std::string ai_type);
  void resume_ai(std::string ai_type);

  std::string behavior_status(std::string ai_type);
};

#endif
