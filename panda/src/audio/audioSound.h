/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file audioSound.h
 * @author skyler
 * @date 2001-06-06
 * Prior system by: cary
 */

#ifndef __AUDIOSOUND_H__
#define __AUDIOSOUND_H__

#include "config_audio.h"
#include "typedReferenceCount.h"
#include "pointerTo.h"
#include "filterProperties.h"
#include "luse.h"

class AudioManager;

class EXPCL_PANDA_AUDIO AudioSound : public TypedReferenceCount {
PUBLISHED:
  virtual ~AudioSound();

  // For best compatibility, set the loop_count, volume, and balance, prior to
  // calling play().  You may set them while they're playing, but it's
  // implementation specific whether you get the results.  - Calling play() a
  // second time on the same sound before it is finished will start the sound
  // again (creating a skipping or stuttering effect).
  virtual void play() = 0;
  virtual void stop() = 0;

  // loop: false = play once; true = play forever.  inits to false.
  virtual void set_loop(bool loop=true) = 0;
  virtual bool get_loop() const = 0;

  // loop_count: 0 = forever; 1 = play once; n = play n times.  inits to 1.
  virtual void set_loop_count(unsigned long loop_count=1) = 0;
  virtual unsigned long get_loop_count() const = 0;

  /**
   * Control time position within the sound, in seconds.  This is similar (in
   * concept) to the seek position within a file.  The value starts at 0.0 (the
   * default) and ends at the value given by the length() method.
   *
   * The current time position will not change while the sound is playing; you
   * must call play() again to effect the change.  To play the same sound from
   * a time offset a second time, explicitly set the time position again.  When
   * looping, the second and later loops will start from the beginning of the
   * sound.
   *
   * If a sound is playing, calling get_time() repeatedly will return different
   * results over time.  e.g.
   * @code
   * PN_stdfloat percent_complete = s.get_time() / s.length();
   * @endcode
   */
  virtual void set_time(PN_stdfloat start_time=0.0) = 0;
  virtual PN_stdfloat get_time() const = 0;

  // 0 = minimum; 1.0 = maximum.  inits to 1.0.
  virtual void set_volume(PN_stdfloat volume=1.0) = 0;
  virtual PN_stdfloat get_volume() const = 0;

  // -1.0 is hard left 0.0 is centered 1.0 is hard right inits to 0.0.
  virtual void set_balance(PN_stdfloat balance_right=0.0) = 0;
  virtual PN_stdfloat get_balance() const = 0;

  // play_rate is any positive PN_stdfloat value.  inits to 1.0.
  virtual void set_play_rate(PN_stdfloat play_rate=1.0f) = 0;
  virtual PN_stdfloat get_play_rate() const = 0;

  // inits to manager's state.
  virtual void set_active(bool flag=true) = 0;
  virtual bool get_active() const = 0;

  // Set (or clear) the event that will be thrown when the sound finishes
  // playing.  To clear the event, pass an empty string.
  virtual void set_finished_event(const std::string& event) = 0;
  virtual const std::string& get_finished_event() const = 0;

  // There is no set_name(), this is intentional.
  virtual const std::string& get_name() const = 0;

  // return: playing time in seconds.
  virtual PN_stdfloat length() const = 0;
  
  // Controls the position of this sound's emitter.  px, py and pz are the
  // emitter's position.  vx, vy and vz are the emitter's velocity in UNITS
  // PER SECOND (default: meters).
  virtual void set_3d_attributes(PN_stdfloat px, PN_stdfloat py, PN_stdfloat pz, PN_stdfloat vx, PN_stdfloat vy, PN_stdfloat vz);
  virtual void get_3d_attributes(PN_stdfloat *px, PN_stdfloat *py, PN_stdfloat *pz, PN_stdfloat *vx, PN_stdfloat *vy, PN_stdfloat *vz);

  // Controls the direction of this sound emitter. Currently implemented only for OpenAL.
  virtual void set_3d_direction(LVector3 d);
  virtual void get_3d_direction(LVector3 *d);

  // Controls the distance (in units) that this sound begins to fall off.
  // Also affects the rate it falls off.  Default is 1.0 CloserFaster, <1.0
  // FartherSlower, >1.0
  virtual void set_3d_min_distance(PN_stdfloat dist);
  virtual PN_stdfloat get_3d_min_distance() const;

  // Controls the maximum distance (in units) that this sound stops falling
  // off.  The sound does not stop at that point, it just doesn't get any
  // quieter.  You should rarely need to adjust this.  Default is 1000000000.0
  virtual void set_3d_max_distance(PN_stdfloat dist);
  virtual PN_stdfloat get_3d_max_distance() const;

  // Sets the angle of the inner cone of a directional sound source. In the zone inside of the inner cone
  // sound is emitted with the (normal) volume set by set_volume().
  virtual void set_3d_cone_inner_angle(PN_stdfloat angle);
  virtual PN_stdfloat get_3d_cone_inner_angle() const;
  // Sets the angle of the outer cone of a directional sound source. In the zone between
  // the inner and the outer cone the volume is attenuated.
  virtual void set_3d_cone_outer_angle(PN_stdfloat angle);
  virtual PN_stdfloat get_3d_cone_outer_angle() const;
  // Sets a factor applied to the volume set by set_volume() for the zone outside the outer cone.
  // By default this is 0 (so no sound is heard inside the outer zone).
  virtual void set_3d_cone_outer_gain(PN_stdfloat gain);
  virtual PN_stdfloat get_3d_cone_outer_gain() const;

  // speaker_mix and speaker_level(s) serve the same purpose.
  // speaker_mix is for use with FMOD. speaker_level(s) is for use with
  // Miles.  Both interfaces exist because of a significant difference in the
  // two APIs.  Hopefully the difference can be reconciled into a single
  // interface at some point.
  virtual PN_stdfloat get_speaker_mix(int speaker);
  virtual void set_speaker_mix(PN_stdfloat frontleft, PN_stdfloat frontright, PN_stdfloat center, PN_stdfloat sub, PN_stdfloat backleft, PN_stdfloat backright, PN_stdfloat sideleft, PN_stdfloat  sideright);

  virtual PN_stdfloat get_speaker_level(int index);
  virtual void set_speaker_levels(PN_stdfloat level1, PN_stdfloat level2=-1.0f, PN_stdfloat level3=-1.0f, PN_stdfloat level4=-1.0f, PN_stdfloat level5=-1.0f, PN_stdfloat level6=-1.0f, PN_stdfloat level7=-1.0f, PN_stdfloat level8=-1.0f, PN_stdfloat level9=-1.0f);

  virtual int get_priority();
  virtual void set_priority(int priority);

  virtual bool configure_filters(FilterProperties *config);

  enum SoundStatus { BAD, READY, PLAYING };
  virtual SoundStatus status() const = 0;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out) const;

protected:
  AudioSound();

  friend class AudioManager;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "AudioSound",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

inline std::ostream &
operator << (std::ostream &out, const AudioSound &sound) {
  sound.output(out);
  return out;
}

#include "audioSound.I"

EXPCL_PANDA_AUDIO std::ostream &
operator << (std::ostream &out, AudioSound::SoundStatus status);

#endif /* __AUDIOSOUND_H__ */
