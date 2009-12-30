// Filename: pathFollow.h
// Created by:  Deepak, John, Navin (24Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised 
// BSD license.  You should have received a copy of this license
// along with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PATHFOLLOW_H
#define PATHFOLLOW_H

#include "aiGlobals.h"
#include "aiCharacter.h"
#include "aiNode.h"

class AICharacter;

////////////////////////////////////////////////////////////////////
//       Class : PathFollow
// Description : This class handles all calls to the path follow
//               behavior and has functions to handle pathfinding
////////////////////////////////////////////////////////////////////
class PathFollow {

public:
  AICharacter *_ai_char;
  float _follow_weight;
  vector<LVecBase3f> _path;
  int _curr_path_waypoint;
  bool _start;
  NodePath _dummy;
  string _type;
  ClockObject *_myClock;
  float _time;

  PathFollow(AICharacter *ai_ch, float follow_wt);
  ~PathFollow();
  void add_to_path(LVecBase3f pos);
  void start(string type);
  void do_follow();
  bool check_if_possible();
};

#endif
