// Filename: audio_midi.h
// Created by:  cary (26Sep00)
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
