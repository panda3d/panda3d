// Filename: audio_trait.h
// Created by:  frang (06Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_TRAIT_H__
#define __AUDIO_TRAIT_H__

#include <pandabase.h>

class EXPCL_PANDA AudioTraits {
public:
  class SampleClass;
  class MusicClass;

  typedef void DeleteSampleFunc(SampleClass*);
  typedef void DeleteMusicFunc(MusicClass*);

  class EXPCL_PANDA SampleClass {
  public:
    SampleClass(void) {}
    virtual ~SampleClass(void);

    enum SampleStatus { READY, PLAYING } ;

    virtual float length(void) = 0;
    virtual SampleStatus status(void) = 0;
  };
  class EXPCL_PANDA MusicClass {
  public:
    MusicClass(void) {}
    virtual ~MusicClass(void);

    enum MusicStatus { READY, PLAYING };
    virtual MusicStatus status(void) = 0;
  };
  class EXPCL_PANDA PlayerClass {
  public:
    PlayerClass(void) {}
    virtual ~PlayerClass(void);

    virtual void play_sample(SampleClass*) = 0;
    virtual void play_music(MusicClass*) = 0;
    virtual void set_volume(SampleClass*, int) = 0;
    virtual void set_volume(MusicClass*, int) = 0;
  };
};

#endif /* __AUDIO_TRAIT_H__ */
