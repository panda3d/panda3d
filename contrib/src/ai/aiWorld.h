/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiWorld.h
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#ifndef _AIWORLD_H
#define _AIWORLD_H

#include "aiGlobals.h"
#include "aiCharacter.h"
#include "flock.h"

class AICharacter;
class Flock;

/**
 * A class that implements the virtual AI world which keeps track of the AI
 * characters active at any given time.  It contains a linked list of AI
 * characters, obstactle data and unique name for each character.  It also
 * updates each characters state.  The AI characters can also be added to the
 * world as flocks.
 */
class EXPCL_PANDAAI AIWorld {
  private:
    typedef std::vector<PT(AICharacter)> AICharPool;
    AICharPool _ai_char_pool;
    NodePath _render;
  public:
    std::vector<NodePath> _obstacles;
    typedef std::vector<Flock*> FlockPool;
    FlockPool _flock_pool;
    void remove_ai_char_from_flock(std::string name);

PUBLISHED:
    AIWorld(NodePath render);
    ~AIWorld();

    void add_ai_char(AICharacter *ai_ch);
    void remove_ai_char(std::string name);

    void add_flock(Flock *flock);
    void flock_off(unsigned int flock_id);
    void flock_on(unsigned int flock_id);
    void remove_flock(unsigned int flock_id);
    Flock get_flock(unsigned int flock_id);

    void add_obstacle(NodePath obstacle);
    void remove_obstacle(NodePath obstacle);

    void print_list();
    void update();
};

#endif
