// Filename: nullAudioManager.h
// Created by:  skyler (June 6, 2001)
// Prior system by: cary
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __NULL_AUDIO_MANAGER_H__
#define __NULL_AUDIO_MANAGER_H__

#include "audioManager.h"
#include "nullAudioSound.h"

#define NAME_MACRO_FOR_AUDIO_MANAGER() NullAudioManager

class EXPCL_PANDA NullAudioManager : public AudioManager {
public:
  NullAudioManager();
  virtual ~NullAudioManager();
  
  // Get a sound:
  // You own this sound.  Be sure to delete it when you're done.
  virtual AudioSound* get_sound(const string&);
  // Tell the AudioManager there is no need to keep this one cached.
  virtual void drop_sound(const string&);

  // Control volume:
  // FYI:
  //   If you start a sound with the volume off and turn the volume 
  //   up later, you'll hear the sound playing at that late point.
  virtual void set_volume(float);
  virtual float get_volume();
  
  // Turn the manager on an off.
  // If you play a sound while the manager is inactive, it won't start.
  // If you deactivate the manager while sounds are playing, they'll
  // stop.
  // If you activate the manager while looping sounds are playing
  // (those that have a loop_count of zero),
  // they will start playing from the begining of their loop.
  virtual void set_active(bool);
  virtual bool get_active();
};

#endif /* __NULL_AUDIO_MANAGER_H__ */
