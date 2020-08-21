/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file aiWorld.cxx
 * @author Deepak, John, Navin
 * @date 2009-09-08
 */

#include "aiWorld.h"

AIWorld::AIWorld(NodePath render) {
  _render = std::move(render);
}

AIWorld::~AIWorld() {
}

void AIWorld::add_ai_char(AICharacter *ai_char) {
  _ai_char_pool.push_back(ai_char);
  ai_char->_window_render = _render;
  ai_char->_world = this;
}

void AIWorld::remove_ai_char(std::string name) {
  AICharPool::iterator it;
  for (it = _ai_char_pool.begin(); it != _ai_char_pool.end(); ++it) {
    AICharacter *ai_char = *it;
    if (ai_char->_name == name) {
      nassertv(ai_char->_world == this);
      ai_char->_world = nullptr;
      _ai_char_pool.erase(it);
      break;
    }
  }

  remove_ai_char_from_flock(std::move(name));
}

void AIWorld::remove_ai_char_from_flock(std::string name) {
  for (AICharacter *ai_char : _ai_char_pool) {
    for (Flock *flock : _flock_pool) {
      if (ai_char->_ai_char_flock_id == flock->get_id()) {
        for (size_t j = 0; j < flock->_ai_char_list.size(); ++j) {
          if (flock->_ai_char_list[j]->_name == name) {
            flock->_ai_char_list.erase(flock->_ai_char_list.begin() + j);
            return;
          }
        }
      }
    }
  }
}

/**
 * This function prints the names of the AI characters that have been added to
 * the AIWorld.  Useful for debugging purposes.
 */
void AIWorld::print_list() {
  for (AICharacter *ai_char : _ai_char_pool) {
    std::cout << ai_char->_name << std::endl;
  }
}

/**
 * The AIWorld update function calls the update function of all the AI
 * characters which have been added to the AIWorld.
 */
void AIWorld::update() {
  for (AICharacter *ai_char : _ai_char_pool) {
    ai_char->update();
  }
}

/**
 * This function adds all the AI characters in the Flock object to the
 * AICharPool.  This function allows adding the AI characetrs as part of a
 * flock.
 */
void AIWorld::add_flock(Flock *flock) {
  // Add all the ai_characters in the flock to the AIWorld.
  for(unsigned int i = 0; i < flock->_ai_char_list.size(); ++i) {
    this->add_ai_char(flock->_ai_char_list[i]);
  }
  // Add the flock to the flock pool.
  _flock_pool.push_back(flock);
}

/**
 * This function returns a handle to the Flock whose id is passed.
 */
Flock AIWorld::get_flock(unsigned int flock_id) {
  for(unsigned int i=0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
      return *_flock_pool[i];
    }
  }
  Flock *null_flock = nullptr;
  return *null_flock;
}

/**
 * This function removes the flock behavior completely.
 */
void AIWorld::remove_flock(unsigned int flock_id) {
  for(unsigned int i = 0; i < _flock_pool.size(); ++i) {
    if(_flock_pool[i]->get_id() == flock_id) {
       for(unsigned int j = 0; j < _flock_pool[i]->_ai_char_list.size(); ++j) {
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock_activate");
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->turn_off("flock");
         _flock_pool[i]->_ai_char_list[j]->get_ai_behaviors()->_flock_group = nullptr;
       }
       _flock_pool.erase(_flock_pool.begin() + i);
       break;
    }
  }
}

/**
 * This function turns off the flock behavior temporarily.  Similar to pausing
 * the behavior.
 */
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

/**
 * This function turns on the flock behavior.
 */
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

/**
 * This function adds the nodepath as an obstacle that is needed by the
 * obstacle avoidance behavior.
 */
void AIWorld::add_obstacle(NodePath obstacle) {
  _obstacles.push_back(obstacle);
}

/**
 * This function removes the nodepath from the obstacles list that is needed
 * by the obstacle avoidance behavior.
 */
void AIWorld::remove_obstacle(NodePath obstacle) {
  for(unsigned int i = 0; i <= _obstacles.size(); ++i) {
    if(_obstacles[i] == obstacle) {
      _obstacles.erase(_obstacles.begin() + i);
    }
  }
}
