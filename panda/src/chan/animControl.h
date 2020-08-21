/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animControl.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef ANIMCONTROL_H
#define ANIMCONTROL_H

#include "pandabase.h"

#include "animInterface.h"
#include "animBundle.h"
#include "partGroup.h"
#include "bitArray.h"
#include "pandaNode.h"
#include "typedReferenceCount.h"
#include "namable.h"
#include "pmutex.h"
#include "conditionVar.h"

class PartBundle;
class AnimChannelBase;

/**
 * Controls the timing of a character animation.  An AnimControl object is
 * created for each character/bundle binding and manages the state of the
 * animation: whether started, stopped, or looping, and the current frame
 * number and play rate.
 */
class EXPCL_PANDA_CHAN AnimControl : public TypedReferenceCount, public AnimInterface, public Namable {
public:
  AnimControl(const std::string &name, PartBundle *part,
              double frame_rate, int num_frames);
  AnimControl(const AnimControl &copy) = delete;

  void setup_anim(PartBundle *part, AnimBundle *anim, int channel_index,
                  const BitArray &bound_joints);
  void set_bound_joints(const BitArray &bound_joints);
  void fail_anim(PartBundle *part);

PUBLISHED:
  virtual ~AnimControl();

  INLINE bool is_pending() const;
  void wait_pending();
  INLINE bool has_anim() const;
  void set_pending_done_event(const std::string &done_event);
  std::string get_pending_done_event() const;

  PartBundle *get_part() const;
  INLINE AnimBundle *get_anim() const;
  INLINE int get_channel_index() const;
  INLINE const BitArray &get_bound_joints() const;

  INLINE void set_anim_model(PandaNode *model);
  INLINE PandaNode *get_anim_model() const;

  virtual void output(std::ostream &out) const;

public:
  // The following functions aren't really part of the public interface;
  // they're just public so we don't have to declare a bunch of friends.

  bool channel_has_changed(AnimChannelBase *channel, bool frame_blend_flag) const;
  void mark_channels(bool frame_blend_flag);

protected:
  virtual void animation_activated();

private:
  bool _pending;
  std::string _pending_done_event;
  Mutex _pending_lock;  // protects the above two.
  ConditionVar _pending_cvar; // signals when _pending goes true.

  // This is a PT(PartGroup) instead of a PT(PartBundle), just because we
  // can't include partBundle.h for circular reasons.  But it actually keeps a
  // pointer to a PartBundle.
  const PT(PartGroup) _part;
  PT(AnimBundle) _anim;
  int _channel_index;

  // This is the frame number as of the last call to mark_channels(). In
  // frame_blend mode, we also record the fractional part of the frame number.
  int _marked_frame;
  double _marked_frac;

  // This is the bitmask of joints andor sliders from the animation that we
  // have actually bound into this AnimControl.  See get_bound_joints().
  BitArray _bound_joints;

  PT(PandaNode) _anim_model;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    AnimInterface::init_type();
    register_type(_type_handle, "AnimControl",
                  TypedReferenceCount::get_class_type(),
                  AnimInterface::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const AnimControl &control);

#include "animControl.I"

#endif
