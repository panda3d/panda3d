// Filename: audio_midi.h
// Created by:  cary (26Sep00)
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

#ifndef __AUDIO_MIDI_H__
#define __AUDIO_MIDI_H__

#include <filename.h>
#include <list>

// define an internal representation for a midi file
class AudioMidi {
private:
  typedef list<string> StrList;
  StrList _seq;
public:
  AudioMidi(Filename, int = 1);
  AudioMidi(const AudioMidi&);
  ~AudioMidi(void);

  AudioMidi& operator=(const AudioMidi&);
  bool operator==(const AudioMidi&) const;
};

#endif /* __AUDIO_MIDI_H__ */
