// Filename: audio_linux_traits.h
// Created by:  cary (02Oct00)
// 
////////////////////////////////////////////////////////////////////

// yes, this needs to be outside the ifdef protection
#include "audio_trait.h"

#ifdef AUDIO_USE_LINUX

#ifndef __AUDIO_LINUX_TRAITS_H__
#define __AUDIO_LINUX_TRAITS_H__

#include "config_audio.h"

#ifndef HAVE_DEFINED_BYTE
typedef unsigned char byte;
#define HAVE_DEFINED_BYTE
#endif /* HAVE_DEFINED_BYTE */

extern byte* zero_buffer;

class EXPCL_PANDA Buffer {
private:
  byte* _data;
  unsigned long _size;
  unsigned long _pos;
  bool _done;

  INLINE unsigned long buffer_min(unsigned long a, unsigned long b) {
    return (a<b)?a:b;
  }
public:
  INLINE Buffer(byte* data, unsigned long size) : _data(data), _size(size),
						  _pos(0), _done(false) {}
  INLINE byte* get_buffer(byte* buf) {
    unsigned long copy_size = buffer_min(_size-_pos, audio_buffer_size);
    unsigned long residue = audio_buffer_size - copy_size;
    memcpy(buf, &_data[_pos], copy_size);
    _pos += copy_size;
    if (residue != 0) {
      _pos -= _size;
      memcpy(&buf[copy_size], zero_buffer, residue);
      _done = true;
    }
    return buf;
  }
  INLINE bool is_done(void) const {
    return _done;
  }
  INLINE unsigned long get_size(void) const {
    return _size;
  }
  INLINE void reset(void) {
    _done = false;
    _pos = 0;
  }
};

class EXPCL_PANDA LinuxSample : public AudioTraits::SampleClass {
private:
  Buffer* _data;
public:
  INLINE LinuxSample(Buffer*);
  virtual ~LinuxSample(void);

  virtual float length(void);
  virtual AudioTraits::SampleClass::SampleStatus status(void);

  static void destroy(AudioTraits::SampleClass*);
public:
  // used by the loader
  static LinuxSample* load_raw(byte*, unsigned long);
  // used by the players
  INLINE Buffer* get_data(void);
};

class EXPCL_PANDA LinuxMusic : public AudioTraits::MusicClass {
public:
  INLINE LinuxMusic(void);
  virtual ~LinuxMusic(void);

  virtual AudioTraits::MusicClass::MusicStatus status(void);
};

class EXPCL_PANDA LinuxPlayer : public AudioTraits::PlayerClass {
public:
  INLINE LinuxPlayer(void);
  virtual ~LinuxPlayer(void);

  virtual void play_sample(AudioTraits::SampleClass*);
  virtual void play_music(AudioTraits::MusicClass*);
  virtual void set_volume(AudioTraits::SampleClass*, int);
  virtual void set_volume(AudioTraits::MusicClass*, int);
public:
  // used by the readers
  static LinuxPlayer* get_instance(void);
private:
  static LinuxPlayer* _global_instance;
};

#include "audio_linux_traits.I"

#endif /* __AUDIO_LINUX_TRAITS_H__ */
#endif /* AUDIO_USE_LINUX */
