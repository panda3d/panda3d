// Filename: audio_music.h
// Created by:  cary (22Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_MUSIC_H__
#define __AUDIO_MUSIC_H__

#include "audio_trait.h"
#include "typedReferenceCount.h"
#include "namable.h"

class AudioPool;
class AudioManager;

class EXPCL_PANDA AudioMusic : public TypedReferenceCount, public Namable {
private:
  AudioTraits::MusicClass *_music;
  AudioTraits::PlayingClass *_state;
  AudioTraits::PlayerClass *_player;
  AudioTraits::DeleteMusicFunc *_destroy;
protected:
  INLINE AudioMusic(AudioTraits::MusicClass*, AudioTraits::PlayingClass*,
		    AudioTraits::PlayerClass*, AudioTraits::DeleteMusicFunc*,
		    const string&);
  INLINE AudioMusic(const AudioMusic&);
  INLINE AudioMusic& operator=(const AudioMusic&);

  INLINE AudioTraits::PlayerClass* get_player(void);
  INLINE AudioTraits::MusicClass* get_music(void);

  friend class AudioPool;
  friend class AudioManager;
public:
  virtual ~AudioMusic(void);
  INLINE bool operator==(const AudioMusic&) const;

  enum MusicStatus { BAD, READY, PLAYING };

  MusicStatus status(void);
public:
  // type stuff
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioMusic",
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

#include "audio_music.I"

#endif /* __AUDIO_MUSIC_H__ */
