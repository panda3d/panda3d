// Filename: audio_trait.h
// Created by:  frang (06Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_TRAIT_H__
#define __AUDIO_TRAIT_H__

#include <pandabase.h>

class EXPCL_PANDA AudioTraits {
public:
  class SoundClass;
  class PlayingClass;
  class PlayerClass;

  typedef void DeleteSoundFunc(SoundClass*);
  typedef void DeletePlayingFunc(PlayingClass*);

  class EXPCL_PANDA SoundClass {
  public:
    SoundClass(void) {}
    virtual ~SoundClass(void);

    virtual float length(void) const = 0;
    virtual PlayingClass* get_state(void) const = 0;
    virtual PlayerClass* get_player(void) const = 0;
    virtual DeleteSoundFunc* get_destroy(void) const = 0;
    virtual DeletePlayingFunc* get_delstate(void) const = 0;
  };
  class EXPCL_PANDA PlayingClass {
  protected:
    SoundClass* _sound;
  public:
    PlayingClass(SoundClass* s) : _sound(s) {}
    virtual ~PlayingClass(void);

    enum PlayingStatus { BAD, READY, PLAYING } ;

    virtual PlayingStatus status(void) = 0;
  };
  class EXPCL_PANDA PlayerClass {
  public:
    PlayerClass(void) {}
    virtual ~PlayerClass(void);

    virtual void play_sound(SoundClass*, PlayingClass*) = 0;
    virtual void stop_sound(SoundClass*, PlayingClass*) = 0;
    virtual void set_volume(PlayingClass*, float) = 0;
  };
};

// this is really ugly.  But since we have to be able to include/compile
// all of the driver files on any system, I need to centralize a switch
// for which one is real.
#ifdef HAVE_SYS_SOUNDCARD_H
#define AUDIO_USE_LINUX
#elif defined(WIN32_VC)
#define AUDIO_USE_WIN32
#elif defined(HAVE_MIKMOD)
#define AUDIO_USE_MIKMOD
#else
#define AUDIO_USE_NULL
#endif

#endif /* __AUDIO_TRAIT_H__ */
