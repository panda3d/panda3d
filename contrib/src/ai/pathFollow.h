
#ifndef _PATHFOLLOW_H
#define _PATHFOLLOW_H

#include "aiGlobals.h"
#include "aiCharacter.h"
#include "meshNode.h"

class AICharacter;

class EXPCL_PANDAAI PathFollow {

public:
  AICharacter *_ai_char;
  float _follow_weight;
  std::vector<LVecBase3> _path;
  int _curr_path_waypoint;
  bool _start;
  NodePath _dummy;
  std::string _type;
  ClockObject *_myClock;
  float _time;

  PathFollow(AICharacter *ai_ch, float follow_wt);
  ~PathFollow();
  void add_to_path(LVecBase3 pos);
  void start(std::string type);
  void do_follow();
  bool check_if_possible();
};

#endif
