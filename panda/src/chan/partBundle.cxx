// Filename: partBundle.cxx
// Created by:  drose (22Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "partBundle.h"
#include "animBundle.h"
#include "animControl.h"
#include "config_chan.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle PartBundle::_type_handle;



////////////////////////////////////////////////////////////////////
//     Function: PartBundle::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
PartBundle::
PartBundle(const PartBundle &copy) :
  PartGroup(copy),
  _blend_type(copy._blend_type)
{
  // We don't invoke the AnimControlCollection's copy, or any of the
  // bound animations.
  _last_control_set = NULL;
  _net_blend = 0.0f;
  _anim_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::Constructor
//       Access: Public
//  Description: Normally, a PartBundle constructor should not be
//               called directly--it will get created when a
//               PartBundleNode is created.
////////////////////////////////////////////////////////////////////
PartBundle::
PartBundle(const string &name) : PartGroup(name) {
  _blend_type = BT_single;

  _last_control_set = NULL;
  _net_blend = 0.0f;
  _anim_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *PartBundle::
make_copy() const {
  return new PartBundle(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_blend_type
//       Access: Published
//  Description: Defines the way the character responds to multiple
//               set_control_effect()).  By default, the blend_type is
//               BT_single, which disallows multiple animations.  In
//               BT_single mode, it is not necessary to explicitly set
//               the control_effect when starting an animation;
//               starting the animation will implicitly remove the
//               control_effect from the previous animation and set it
//               on the current one.
//
//               However, if the blend_type is set to any other value,
//               the control_effect must be explicitly set via
//               set_control_effect() whenever an animation is to
//               affect the character.
//
//               See partBundle.h for a description of the meaning of
//               each of the BlendType values.
////////////////////////////////////////////////////////////////////
void PartBundle::
set_blend_type(BlendType bt) {
  if (_blend_type != bt) {
    _blend_type = bt;

    if (_blend_type == BT_single && control_size() > 1) {
      // If we just changed to a single blend type, i.e. no blending,
      // we should eliminate all the AnimControls other than the
      // most-recently-added one.

      nassertv(_last_control_set != NULL);
      clear_and_stop_except(_last_control_set);
    }

    _anim_changed = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::clear_control_effects
//       Access: Published
//  Description: Sets the control effect of all AnimControls to zero
//               (but does not "stop" the AnimControls).  The
//               character will no longer be affected by any
//               animation, and will return to its original Jesus
//               pose.
//
//               The AnimControls which are no longer associated will
//               not be using any CPU cycles, but they may still be in
//               the "playing" state; if they are later reassociated
//               with the PartBundle they will resume at their current
//               frame as if they'd been running all along.
////////////////////////////////////////////////////////////////////
void PartBundle::
clear_control_effects() {
  if (!_blend.empty()) {
    _blend.clear();
    _net_blend = 0.0f;
    _anim_changed = true;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::set_control_effect
//       Access: Published
//  Description: Sets the amount by which the character is affected by
//               the indicated AnimControl (and its associated
//               animation).  Normally, this will only be zero or one.
//               Zero indicates the animation does not affect the
//               character, and one means it does.
//
//               If the blend_type is not BT_single (see
//               set_blend_type()), it is possible to have multiple
//               AnimControls in effect simultaneously.  In this case,
//               the effect is a weight that indicates the relative
//               importance of each AnimControl to the final
//               animation.
////////////////////////////////////////////////////////////////////
void PartBundle::
set_control_effect(AnimControl *control, float effect) {
  nassertv(control->get_part() == this);

  if (effect == 0.0f) {
    // An effect of zero means to eliminate the control.
    ChannelBlend::iterator cbi = _blend.find(control);
    if (cbi != _blend.end()) {
      _blend.erase(cbi);
      _anim_changed = true;
    }

  } else {
    // Otherwise we define it.

    // If we currently have BT_single, we only allow one AnimControl
    // at a time.  Stop all of the other AnimControls.
    if (get_blend_type() == BT_single) {
      clear_and_stop_except(control);
    }

    if (get_control_effect(control) != effect) {
      _blend[control] = effect;
      _anim_changed = true;
    }
    _last_control_set = control;
  }

  recompute_net_blend();
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_control_effect
//       Access: Published
//  Description: Returns the amount by which the character is affected
//               by the indicated AnimControl and its associated
//               animation.  See set_control_effect().
////////////////////////////////////////////////////////////////////
float PartBundle::
get_control_effect(AnimControl *control) {
  nassertr(control->get_part() == this, 0.0f);

  ChannelBlend::iterator cbi = _blend.find(control);
  if (cbi == _blend.end()) {
    // The control is not in effect.
    return 0.0f;
  } else {
    return (*cbi).second;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: PartBundle::output
//       Access: Published, Virtual
//  Description: Writes a one-line description of the bundle.
////////////////////////////////////////////////////////////////////
void PartBundle::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::write
//       Access: Published, Virtual
//  Description: Writes a brief description of the bundle and all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void PartBundle::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::bind_anim
//       Access: Published
//  Description: Binds the animation to the bundle, if possible, and
//               returns a new AnimControl that can be used to start
//               and stop the animation.  If the anim hierarchy does
//               not match the part hierarchy, returns NULL.
//
//               If hierarchy_match_flags is 0, only an exact match is
//               accepted; otherwise, it may contain a union of
//               PartGroup::HierarchyMatchFlags values indicating
//               conditions that will be tolerated (but warnings will
//               still be issued).
//
//               This flavor of bind_anim() does not associate a name
//               with the channel, and the AnimControl is not stored
//               within the PartBundle; it is the user's
//               responsibility to maintain the pointer.  The
//               animation will automatically unbind itself when the
//               AnimControl destructs (i.e. its reference count goes
//               to zero).
////////////////////////////////////////////////////////////////////
PT(AnimControl) PartBundle::
bind_anim(AnimBundle *anim, int hierarchy_match_flags) {
  if ((hierarchy_match_flags & HMF_ok_wrong_root_name) == 0) {
    // Make sure the root names match.
    if (get_name() != anim->get_name()) {
      if (chan_cat.is_error()) {
        chan_cat.error()
          << "Root name of part (" << get_name()
          << ") does not match that of anim (" << anim->get_name()
          << ")\n";
      }
      return NULL;
    }
  }

  if (!check_hierarchy(anim, NULL, hierarchy_match_flags)) {
    return NULL;
  }

  plist<int> holes;
  int channel_index = 0;
  pick_channel_index(holes, channel_index);

  if (!holes.empty()) {
    channel_index = holes.front();
  }

  bind_hierarchy(anim, channel_index);
  return new AnimControl(this, anim, channel_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::bind_anim
//       Access: Published
//  Description: Binds the animation to the bundle, if possible, and
//               returns a new AnimControl that can be used to start
//               and stop the animation.  If the anim hierarchy does
//               not match the part hierarchy, returns NULL.
//
//               If hierarchy_match_flags is 0, only an exact match is
//               accepted; otherwise, it may contain a union of
//               PartGroup::HierarchyMatchFlags values indicating
//               conditions that will be tolerated (but warnings will
//               still be issued).
//
//               This flavor of bind_anim() automatically stores the
//               bound AnimControl in the PartBundle with the
//               indicated name, so that it may later be referenced by
//               name.  This means that the animation will not be
//               unbound until another animation with the same name is
//               bound, or it is explicitly unbound with
//               unbind_anim().
//
//               The return value is true if the animation was
//               successfully bound, false if there was some error.
////////////////////////////////////////////////////////////////////
bool PartBundle::
bind_anim(AnimBundle *anim, const string &name,
          int hierarchy_match_flags) {
  PT(AnimControl) control = bind_anim(anim, hierarchy_match_flags);
  if (control == (AnimControl *)NULL) {
    return false;
  }

  store_anim(control, name);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::advance_time
//       Access: Public
//  Description: Calls advance_time() on all AnimControls currently
//               in effect.
////////////////////////////////////////////////////////////////////
void PartBundle::
advance_time(double time) {
  ChannelBlend::const_iterator cbi;
  for (cbi = _blend.begin(); cbi != _blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->advance_time(time);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::update
//       Access: Public
//  Description: Updates all the parts in the bundle to reflect the
//               data for the current frame (as set in each of the
//               AnimControls).
//
//               Returns true if any part has changed as a result of
//               this, or false otherwise.
////////////////////////////////////////////////////////////////////
bool PartBundle::
update() {
  bool any_changed = do_update(this, NULL, false, _anim_changed);

  // Now update all the controls for next time.
  ChannelBlend::const_iterator cbi;
  for (cbi = _blend.begin(); cbi != _blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->mark_channels();
  }
  _anim_changed = false;

  return any_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::force_update
//       Access: Public
//  Description: Updates all the parts in the bundle to reflect the
//               data for the current frame, whether we believe it
//               needs it or not.
////////////////////////////////////////////////////////////////////
bool PartBundle::
force_update() {
  bool any_changed = do_update(this, NULL, true, true);

  // Now update all the controls for next time.
  ChannelBlend::const_iterator cbi;
  for (cbi = _blend.begin(); cbi != _blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->mark_channels();
  }
  _anim_changed = false;

  return any_changed;
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::control_activated
//       Access: Public, Virtual
//  Description: Called by the AnimControl whenever it starts an
//               animation.  This is just a hook so the bundle can do
//               something, if necessary, before the animation starts.
////////////////////////////////////////////////////////////////////
void PartBundle::
control_activated(AnimControl *control) {
  nassertv(control->get_part() == this);

  // If (and only if) our blend type is BT_single, which means no
  // blending, then starting an animation implicitly enables it.
  if (get_blend_type() == BT_single) {
    set_control_effect(control, 1.0f);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: PartBundle::recompute_net_blend
//       Access: Protected
//  Description: Recomputes the total blending amount after a control
//               effect has been adjusted.  This value must be kept
//               up-to-date so we can normalize the blending amounts.
////////////////////////////////////////////////////////////////////
void PartBundle::
recompute_net_blend() {
  _net_blend = 0.0f;

  ChannelBlend::const_iterator bti;
  for (bti = _blend.begin(); bti != _blend.end(); ++bti) {
    _net_blend += (*bti).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::finalize
//       Access: Public
//  Description: Method to ensure that any necessary clean up tasks
//               that have to be performed by this object are performed
////////////////////////////////////////////////////////////////////
void PartBundle::
finalize(void)
{
  do_update(this, NULL, true, true);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::make_PartBundle
//       Access: Protected
//  Description: Factory method to generate a PartBundle object
////////////////////////////////////////////////////////////////////
TypedWritable* PartBundle::
make_PartBundle(const FactoryParams &params)
{
  PartBundle *me = new PartBundle;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  manager->register_finalize(me);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a PartBundle object
////////////////////////////////////////////////////////////////////
void PartBundle::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_PartBundle);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::clear_and_stop_except
//       Access: Protected
//  Description: Removes and stops all the currently activated
//               AnimControls, except for the indicated one.  This is
//               a special internal function that's only called when
//               _blend_type is BT_single, to automatically stop all
//               the other currently-executing animations.
////////////////////////////////////////////////////////////////////
void PartBundle::
clear_and_stop_except(AnimControl *control) {
  double new_net_blend = 0.0f;
  ChannelBlend new_blend;
  bool any_changed = false;

  ChannelBlend::iterator cbi;
  for (cbi = _blend.begin(); cbi != _blend.end(); ++cbi) {
    AnimControl *ac = (*cbi).first;
    if (ac == control) {
      // Save this control, but only this one.
      new_blend.insert(new_blend.end(), (*cbi));
      new_net_blend += (*cbi).second;
    } else {
      // Remove and stop this control.
      ac->stop();
      any_changed = true;
    }
  }

  if (any_changed) {
    _net_blend = new_net_blend;
    _blend.swap(new_blend);
    _anim_changed = true;
  }
}
