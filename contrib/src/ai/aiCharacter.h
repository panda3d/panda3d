/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiCharacter.h
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#ifndef _AICHARACTER_H
#define _AICHARACTER_H

#include "aiBehaviors.h"
#include "referenceCount.h"

/**
 * This class is used for creating the AI characters.  It assigns both physics
 * and AI attributes to the character.  It also has an update function which
 * updates the physics and AI of the character.  This update function is
 * called by the AIWorld update.
 */
class AIBehaviors;
class AIWorld;

class EXPCL_PANDAAI AICharacter : public ReferenceCount {
 public:
  double _mass;
  double _max_force;
  LVecBase3 _velocity;
  LVecBase3 _steering_force;
  std::string _name;
  double _movt_force;
  unsigned int _ai_char_flock_id;
  AIWorld *_world;
  AIBehaviors *_steering;
  NodePath _window_render;
  NodePath _ai_char_np;
  bool _pf_guide;

  void update();
  void set_velocity(LVecBase3 vel);
  void set_char_render(NodePath render);
  NodePath get_char_render();

PUBLISHED:
    double get_mass();
    void set_mass(double m);

    LVecBase3 get_velocity();

    double get_max_force();
    void set_max_force(double max_force);

    NodePath get_node_path();
    void set_node_path(NodePath np);

    AIBehaviors * get_ai_behaviors();

    // This function is used to enable or disable the guides for path finding.
    void set_pf_guide(bool pf_guide);

    explicit AICharacter(std::string model_name, NodePath model_np, double mass, double movt_force, double max_force);
    ~AICharacter();
};

#endif
