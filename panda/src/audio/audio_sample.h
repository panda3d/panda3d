// Filename: audio_sample.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_SAMPLE_H__
#define __AUDIO_SAMPLE_H__

#include "audio_trait.h"
#include "typedReferenceCount.h"
#include "namable.h"

class AudioPool;
class AudioManager;

class EXPCL_PANDA AudioSample : public TypedReferenceCount, public Namable {
private:
  AudioTraits::SampleClass *_sample;
  AudioTraits::PlayingClass *_state;
  AudioTraits::PlayerClass *_player;
  AudioTraits::DeleteSampleFunc *_destroy;
protected:
  INLINE AudioSample(AudioTraits::SampleClass*, AudioTraits::PlayingClass*,
		     AudioTraits::PlayerClass*, AudioTraits::DeleteSampleFunc*,
		     const string&);
  INLINE AudioSample(const AudioSample&);
  INLINE AudioSample& operator=(const AudioSample&);

  INLINE AudioTraits::PlayerClass* get_player(void);
  INLINE AudioTraits::SampleClass* get_sample(void);

  friend class AudioPool;
  friend class AudioManager;
public:
  virtual ~AudioSample(void);
  INLINE bool operator==(const AudioSample&) const;

  enum SampleStatus { BAD, READY, PLAYING } ;
  
  float length(void);
  SampleStatus status(void);
public:
  // type stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSample",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "audio_sample.I"

#endif /* __AUDIO_SAMPLE_H__ */
