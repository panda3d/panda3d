////////////////////////////////////////////////////////////////////////
// Filename    : aiWorld.h
// Created by  : Deepak, John, Navin
// Date        :  8 Sep 09
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#pragma warning (disable:4996)
#pragma warning (disable:4005)
#pragma warning(disable:4275)


#ifndef _AIWORLD_H
#define _AIWORLD_H

#include "aiGlobals.h"
#include "aiCharacter.h"
#include "flock.h"

class AICharacter;
class Flock;

///////////////////////////////////////////////////////////////////////
//
// Class : AICharPool
//  Description : This class implements a linked list of AI Characters allowing
//              the user to add and delete characters from the linked list.
//              This will be used in the AIWorld class.

////////////////////////////////////////////////////////////////////////


class EXPCL_PANDAAI AICharPool {
    public:
    struct node {
      AICharacter * _ai_char;
      node * _next;
    } ;

    node* _head;
    AICharPool();
    ~AICharPool();
    void append(AICharacter *ai_ch);
    void del(string name);
        void print_list();
};


///////////////////////////////////////////////////////////////////////
//
// Class : AIWorld
//  Description : A class that implements the virtual AI world which keeps track
//              of the AI characters active at any given time. It contains a linked
//              list of AI characters, obstactle data and unique name for each
//              character. It also updates each characters state. The AI characters
//              can also be added to the world as flocks.

////////////////////////////////////////////////////////////////////////


class EXPCL_PANDAAI AIWorld {
  private:
    AICharPool * _ai_char_pool;
    NodePath _render;
  public:
    vector<NodePath> _obstacles;
    typedef std::vector<Flock*> FlockPool;
    FlockPool _flock_pool;
    void remove_ai_char_from_flock(string name);

PUBLISHED:
    AIWorld(NodePath render);
    ~AIWorld();

    void add_ai_char(AICharacter *ai_ch);
    void remove_ai_char(string name);

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









