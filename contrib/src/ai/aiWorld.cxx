////////////////////////////////////////////////////////////////////////
// Filename    : aiWorld.cxx
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

#include "aiWorld.h"

AIWorld::AIWorld(NodePath render) {
  _ai_char_pool = new AICharPool();
  _render = render;
}

AIWorld::~AIWorld() {
}

void AIWorld::add_ai_char(AICharacter *ai_char) {
  _ai_char_pool->append(ai_char);
  ai_char->_window_render = _render;
  ai_char->_world = this;
}

void AIWorld::remove_ai_char(string name) {
  _ai_char_pool->del(name);
  remove_ai_char_from_flock(name);
}

void AIWorld::remove_ai_char_from_flock(string name) {
  AICharPool::node *ai_pool;
  ai_pool = _ai_char_pool->_head;
  while((ai_pool) != NULL) {
    for(unsigned int i = 0; i < _flock_pool.size(); ++i) {
      if(ai_pool->_ai_char->_ai_char_flock_id == _flock_pool[i]->get_id()) {
        for(unsigned int j = 0; j<_flock_pool[i]->_ai_char_list.size(); ++j) {
          if(_flock_pool[i]->_ai_char_list[j]->_name == name) {
            _flock_pool[i]->_ai_char_list.erase(_flock_pool[i]->_ai_char_list.begin() + j);
            return;
          }
        }
      }
    }
    ai_pool = ai_pool->_next;
  }
}

void AIWorld::print_list() {
  _ai_char_pool->print_list();
}

////////////////////////////////////////////////////////////////////////
// Function : update
// Description : The AIWorld update function calls the update function of all the
//                AI characters which have been added to the AIWorld.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::update() {
  AICharPool::node *ai_pool;
  ai_pool = _ai_char_pool->_head;

  while((ai_pool)!=NULL) {
    ai_pool->_ai_char->update();
    ai_pool = ai_pool->_next;
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : add_flock
// Description : This function adds all the AI characters in the Flock object to
//                the AICharPool. This function allows adding the AI characetrs as
//                part of a flock.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::add_flock(Flock *flock) {
  // Add all the ai_characters in the flock to the AIWorld.
  for(unsigned int i = 0; i < flock->_ai_char_list.size(); ++i) {
    this->add_ai_char(flock->_ai_char_list[i]);
  }
  // Add the flock to the flock pool.
  _flock_pool.push_back(flock);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : get_flock
// Description : This function returns a handle to the Flock whose id is passed.

/////////////////////////////////////////////////////////////////////////////////

Flock AIWorld::get_flock(unsigned int flock_id) {
  for(unsigned int i=0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
      return *_flock_pool[i];
    }
  }
  Flock *null_flock = NULL;
  return *null_flock;
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : remove_flock
// Description : This function removes the flock behavior completely.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::remove_flock(unsigned int flock_id) {
  for(unsigned int i = 0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
       for(unsigned int j = 0; j < _flock_pool[i]->_ai_char_list.size(); ++j) {
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock_activate");
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock");
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->_flock_group = NULL;
       }
       _flock_pool.erase(_flock_pool.begin() + i);
       break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : flock_off
// Description : This function turns off the flock behavior temporarily. Similar to
//                pausing the behavior.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::flock_off(unsigned int flock_id) {
  for(unsigned int i = 0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
       for(unsigned int j = 0; j < _flock_pool[i]->_ai_char_list.size(); ++j) {
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock_activate");
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock");
       }
       break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : flock_on
// Description : This function turns on the flock behavior.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::flock_on(unsigned int flock_id) {
  for(unsigned int i = 0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
       for(unsigned int j = 0; j < _flock_pool[i]->_ai_char_list.size(); ++j) {
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_on("flock_activate");
       }
       break;
    }
  }
}

AICharPool::AICharPool() {
  _head = NULL;
}

AICharPool::~AICharPool() {
}

void AICharPool::append(AICharacter *ai_ch) {
  node *q;
  node *t;

  if(_head == NULL) {
    q = new node();
    q->_ai_char = ai_ch;
    q->_next = NULL;
    _head = q;
  }
  else {
    q = _head;
    while( q->_next != NULL) {
      q = q->_next;
    }

    t = new node();
    t->_ai_char = ai_ch;
    t->_next = NULL;
    q->_next = t;
  }
}

void AICharPool::del(string name) {
  node *q;
  node *r;
  q = _head;

  if(_head==NULL) {
    return;
  }

  // Only one node in the linked list
  if(q->_next == NULL) {
    if(q->_ai_char->_name == name) {
      _head = NULL;
      delete q;
    }
    return;
  }

  r = q;
  while( q != NULL) {
    if( q->_ai_char->_name == name) {
      // Special case
      if(q == _head) {
        _head = q->_next;
        delete q;
        return;
      }

      r->_next = q->_next;
      delete q;
      return;
    }
    r = q;
    q = q->_next;
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : print_list
// Description : This function prints the ai characters in the AICharPool. Used for
//                debugging purposes.

/////////////////////////////////////////////////////////////////////////////////

void AICharPool::print_list() {
  node* q;
  q = _head;
  while(q != NULL) {
    cout<<q->_ai_char->_name<<endl;
    q = q->_next;
  }
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : add_obstacle
// Description : This function adds the nodepath as an obstacle that is needed
//                by the obstacle avoidance behavior.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::add_obstacle(NodePath obstacle) {
  _obstacles.push_back(obstacle);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Function : remove_obstacle
// Description : This function removes the nodepath from the obstacles list that is needed
//                by the obstacle avoidance behavior.

/////////////////////////////////////////////////////////////////////////////////

void AIWorld::remove_obstacle(NodePath obstacle) {
  for(unsigned int i = 0; i <= _obstacles.size(); ++i) {
    if(_obstacles[i] == obstacle) {
      _obstacles.erase(_obstacles.begin() + i);
    }
  }
}
