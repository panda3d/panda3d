// Filename: animControl.h
// Created by:  drose (19Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCONTROL_H
#define ANIMCONTROL_H

#include "pandabase.h"

#include "animInterface.h"
#include "animBundle.h"
#include "partGroup.h"

#include "referenceCount.h"

class PartBundle;
class AnimChannelBase;

////////////////////////////////////////////////////////////////////
//       Class : AnimControl
// Description : Controls the timing of a character animation.  An
//               AnimControl object is created for each
//               character/bundle binding and manages the state of the
//               animation: whether started, stopped, or looping, and
//               the current frame number and play rate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimControl : public ReferenceCount, public AnimInterface {
public:
  AnimControl(PartBundle *part, AnimBundle *anim, int channel_index);

PUBLISHED:
  virtual ~AnimControl();

  PartBundle *get_part() const;
  INLINE AnimBundle *get_anim() const;
  INLINE int get_channel_index() const;

  virtual void output(ostream &out) const;

public:
  // The following functions aren't really part of the public
  // interface; they're just public so we don't have to declare a
  // bunch of friends.

  bool channel_has_changed(AnimChannelBase *channel) const;
  void mark_channels();

protected:
  virtual void animation_activated();

private:
  // This is a PT(PartGroup) instead of a PT(PartBundle), just because
  // we can't include partBundle.h for circular reasons.  But it
  // actually keeps a pointer to a PartBundle.
  PT(PartGroup) _part;
  PT(AnimBundle) _anim;
  int _channel_index;

  // This is the frame number as of the last call to mark_channels().
  int _marked_frame;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    AnimInterface::init_type();
    register_type(_type_handle, "AnimControl",
                  ReferenceCount::get_class_type(),
                  AnimInterface::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animControl.I"

#endif
