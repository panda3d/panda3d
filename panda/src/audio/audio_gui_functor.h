// Filename: audio_gui_functor.h
// Created by:  cary (19Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef __AUDIO_GUI_FUNCTOR_H__
#define __AUDIO_GUI_FUNCTOR_H__

#include <guiBehavior.h>
#include "audio_sound.h"

class EXPCL_PANDA AudioGuiFunctor : public GuiBehavior::BehaviorFunctor {
protected:
  GuiBehavior::BehaviorFunctor* _prev;
  PT(AudioSound) _sound;
public:
  virtual void doit(GuiBehavior*);
PUBLISHED:
  AudioGuiFunctor(AudioSound*, GuiBehavior::BehaviorFunctor* =
		  (GuiBehavior::BehaviorFunctor*)0L);
  virtual ~AudioGuiFunctor(void);
  INLINE AudioSound* get_sound(void) const { return _sound; }
  INLINE GuiBehavior::BehaviorFunctor* get_prev(void) const { return _prev; }
};

#endif /* __AUDIO_GUI_FUNCTOR_H__ */
