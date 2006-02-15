// Filename: partBundle.cxx
// Created by:  drose (22Feb99)
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


#include "partBundle.h"
#include "animBundle.h"
#include "animControl.h"
#include "config_chan.h"
#include "bitArray.h"

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
  PartGroup(copy)
{
  CDWriter cdata(_cycler, true);
  cdata->_blend_type = copy.get_blend_type();
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
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDReader cdata(_cycler);
  if (cdata->_blend_type != bt) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_blend_type = bt;

    if (cdataw->_blend_type == BT_single && cdataw->_blend.size() > 1) {
      // If we just changed to a single blend type, i.e. no blending,
      // we should eliminate all the AnimControls other than the
      // most-recently-added one.

      nassertv(cdataw->_last_control_set != NULL);
      clear_and_stop_intersecting(cdataw->_last_control_set, cdataw);
    }

    cdataw->_anim_changed = true;
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
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDReader cdata(_cycler);
  if (!cdata->_blend.empty()) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_blend.clear();
    cdataw->_net_blend = 0.0f;
    cdataw->_anim_changed = true;
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
//               If subset is specified, it restricts the binding only
//               to the named subtree of joints.
//
//               The AnimControl is not stored within the PartBundle;
//               it is the user's responsibility to maintain the
//               pointer.  The animation will automatically unbind
//               itself when the AnimControl destructs (i.e. its
//               reference count goes to zero).
////////////////////////////////////////////////////////////////////
PT(AnimControl) PartBundle::
bind_anim(AnimBundle *anim, int hierarchy_match_flags,
          const PartSubset &subset) {
  nassertr(Thread::get_current_pipeline_stage() == 0, NULL);

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

  int joint_index = 0;
  BitArray bound_joints;
  if (subset.is_include_empty()) {
    bound_joints = BitArray::all_on();
  }
  bind_hierarchy(anim, channel_index, joint_index, subset.is_include_empty(), 
                 bound_joints, subset);
  return new AnimControl(this, anim, channel_index, bound_joints);
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
  bool anim_changed;
  {
    CDReader cdata(_cycler);
    anim_changed = cdata->_anim_changed;
  }
  bool any_changed = do_update(this, NULL, false, anim_changed);

  // Now update all the controls for next time.
  CDWriter cdata(_cycler, false);
  ChannelBlend::const_iterator cbi;
  for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->mark_channels();
  }
  
  cdata->_anim_changed = false;

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
  CDWriter cdata(_cycler, false);
  ChannelBlend::const_iterator cbi;
  for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->mark_channels();
  }
  
  cdata->_anim_changed = false;

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
  nassertv(Thread::get_current_pipeline_stage() == 0);
  nassertv(control->get_part() == this);

  CDReader cdata(_cycler);
  // If (and only if) our blend type is BT_single, which means no
  // blending, then starting an animation implicitly enables it.
  if (cdata->_blend_type == BT_single) {
    CDWriter cdataw(_cycler, cdata);
    do_set_control_effect(control, 1.0f, cdataw);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: PartBundle::do_set_control_effect
//       Access: Private
//  Description: The private implementation of set_control_effect().
////////////////////////////////////////////////////////////////////
void PartBundle::
do_set_control_effect(AnimControl *control, float effect, CData *cdata) {
  nassertv(control->get_part() == this);

  if (effect == 0.0f) {
    // An effect of zero means to eliminate the control.
    ChannelBlend::iterator cbi = cdata->_blend.find(control);
    if (cbi != cdata->_blend.end()) {
      cdata->_blend.erase(cbi);
      cdata->_anim_changed = true;
    }

  } else {
    // Otherwise we define it.

    // If we currently have BT_single, we only allow one AnimControl
    // at a time.  Stop all of the other AnimControls.
    if (cdata->_blend_type == BT_single) {
      clear_and_stop_intersecting(control, cdata);
    }

    if (do_get_control_effect(control, cdata) != effect) {
      cdata->_blend[control] = effect;
      cdata->_anim_changed = true;
    }
    cdata->_last_control_set = control;
  }

  recompute_net_blend(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::do_get_control_effect
//       Access: Private
//  Description: The private implementation of get_control_effect().
////////////////////////////////////////////////////////////////////
float PartBundle::
do_get_control_effect(AnimControl *control, const CData *cdata) const {
  nassertr(control->get_part() == this, 0.0f);

  ChannelBlend::const_iterator cbi = cdata->_blend.find(control);
  if (cbi == cdata->_blend.end()) {
    // The control is not in effect.
    return 0.0f;
  } else {
    return (*cbi).second;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::recompute_net_blend
//       Access: Private
//  Description: Recomputes the total blending amount after a control
//               effect has been adjusted.  This value must be kept
//               up-to-date so we can normalize the blending amounts.
////////////////////////////////////////////////////////////////////
void PartBundle::
recompute_net_blend(CData *cdata) {
  cdata->_net_blend = 0.0f;

  ChannelBlend::const_iterator bti;
  for (bti = cdata->_blend.begin(); bti != cdata->_blend.end(); ++bti) {
    cdata->_net_blend += (*bti).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::clear_and_stop_intersecting
//       Access: Private
//  Description: Removes and stops all the currently activated
//               AnimControls that animate some joints also animated
//               by the indicated AnimControl.  This is a special
//               internal function that's only called when _blend_type
//               is BT_single, to automatically stop all the other
//               currently-executing animations.
////////////////////////////////////////////////////////////////////
void PartBundle::
clear_and_stop_intersecting(AnimControl *control, CData *cdata) {
  double new_net_blend = 0.0f;
  ChannelBlend new_blend;
  bool any_changed = false;

  ChannelBlend::iterator cbi;
  for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
    AnimControl *ac = (*cbi).first;
    if (ac == control ||
        !ac->get_bound_joints().has_bits_in_common(control->get_bound_joints())) {
      // Save this control--it's either the target control, or it has
      // no joints in common with the target control.
      new_blend.insert(new_blend.end(), (*cbi));
      new_net_blend += (*cbi).second;
    } else {
      // Remove and stop this control.
      ac->stop();
      any_changed = true;
    }
  }

  if (any_changed) {
    cdata->_net_blend = new_net_blend;
    cdata->_blend.swap(new_blend);
    cdata->_anim_changed = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void PartBundle::
finalize(BamReader *) {
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
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_PartBundle);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PartBundle::CData::
CData() {
  _blend_type = BT_single;
  _last_control_set = NULL;
  _net_blend = 0.0f;
  _anim_changed = false;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PartBundle::CData::
CData(const PartBundle::CData &copy) :
  _blend_type(copy._blend_type),
  _last_control_set(copy._last_control_set),
  _blend(copy._blend),
  _net_blend(copy._net_blend),
  _anim_changed(copy._anim_changed)
{
  // Note that this copy constructor is not used by the PartBundle
  // copy constructor!  Any elements that must be copied between
  // PartBundles should also be explicitly copied there.
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *PartBundle::CData::
make_copy() const {
  return new CData(*this);
}
