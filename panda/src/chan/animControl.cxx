// Filename: animControl.cxx
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

#include "animControl.h"
#include "animChannelBase.h"
#include "partBundle.h"
#include "config_chan.h"

TypeHandle AnimControl::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimControl::
AnimControl(PartBundle *part, AnimBundle *anim, int channel_index) {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif

  _part = part;
  _anim = anim;
  _channel_index = channel_index;
  set_frame_rate(_anim->get_base_frame_rate());
  set_num_frames(_anim->get_num_frames());

  _marked_frame = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AnimControl::
~AnimControl() {
  get_part()->set_control_effect(this, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::get_part
//       Access: Published
//  Description: Returns the PartBundle bound in with this
//               AnimControl.
////////////////////////////////////////////////////////////////////
PartBundle *AnimControl::
get_part() const {
  return DCAST(PartBundle, _part);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void AnimControl::
output(ostream &out) const {
  out << "AnimControl(" << get_part()->get_name()
      << ", " << get_anim()->get_name() << ": ";
  AnimInterface::output(out);
  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::channel_has_changed
//       Access: Public
//  Description: Returns true if the indicated channel value has
//               changed since the last call to mark_channels().
////////////////////////////////////////////////////////////////////
bool AnimControl::
channel_has_changed(AnimChannelBase *channel) const {
  if (_marked_frame < 0) {
    return true;
  }

  int this_frame = get_frame();
  return channel->has_changed(_marked_frame, this_frame);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::mark_channels
//       Access: Public
//  Description: Marks this point as the point of reference for the
//               next call to channel_has_changed().
////////////////////////////////////////////////////////////////////
void AnimControl::
mark_channels() {
  _marked_frame = get_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: AnimControl::animation_activated
//       Access: Protected, Virtual
//  Description: This is provided as a callback method for when the
//               user calls one of the play/loop/pose type methods to
//               start the animation playing.
////////////////////////////////////////////////////////////////////
void AnimControl::
animation_activated() {
  get_part()->control_activated(this);
}
