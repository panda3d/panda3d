// Filename: eggTopstrip.h
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTOPSTRIP_H
#define EGGTOPSTRIP_H

#include <pandatoolbase.h>

#include <eggCharacterFilter.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggTopstrip
// Description : Reads a character model and/or animations and strips
//               out the animation from one of the top joints from the
//               entire character.  Particularly useful for generating
//               stackable character models from separately-extracted
//               characters.
////////////////////////////////////////////////////////////////////
class EggTopstrip : public EggCharacterFilter {
public:
  EggTopstrip();

  void run();
};

#endif

