/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartBase.cxx
 * @author drose
 * @date 1999-02-22
 */

#include "movingPartBase.h"
#include "animControl.h"
#include "animChannelBase.h"
#include "bitArray.h"
#include "config_chan.h"
#include "dcast.h"
#include "indent.h"

TypeHandle MovingPartBase::_type_handle;


/**
 *
 */
MovingPartBase::
MovingPartBase(PartGroup *parent, const std::string &name) :
  PartGroup(parent, name),
  _effective_control(nullptr)
{
}

/**
 *
 */
MovingPartBase::
MovingPartBase() :
  _effective_control(nullptr)
{
}

/**
 * Undoes the effect of a previous call to apply_freeze() or apply_control().
 * Returns true if the joint was modified, false otherwise.
 */
bool MovingPartBase::
clear_forced_channel() {
  if (_forced_channel != nullptr) {
    _forced_channel.clear();
    return true;
  }
  return false;
}

/**
 * Returns the AnimChannelBase that has been forced to this joint by a
 * previous call to apply_freeze() or apply_control(), or NULL if no such
 * channel has been applied.
 */
AnimChannelBase *MovingPartBase::
get_forced_channel() const {
  return _forced_channel;
}

/**
 * Writes a brief description of the channel and all of its descendants.
 */
void MovingPartBase::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_value_type() << " " << get_name();
  if (_children.empty()) {
    out << "\n";
  } else {
    out << " {\n";
    write_descendants(out, indent_level + 2);
    indent(out, indent_level) << "}\n";
  }
}

/**
 * Writes a brief description of the channel and all of its descendants, along
 * with their values.
 */
void MovingPartBase::
write_with_value(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_value_type() << " " << get_name() << "\n";
  indent(out, indent_level);
  output_value(out);

  if (_children.empty()) {
    out << "\n";
  } else {
    out << " {\n";
    write_descendants_with_value(out, indent_level + 2);
    indent(out, indent_level) << "}\n";
  }
}

/**
 * Recursively update this particular part and all of its descendents for the
 * current frame.  This is not really public and is not intended to be called
 * directly; it is called from the top of the tree by PartBundle::update().
 *
 * The return value is true if any part has changed, false otherwise.
 */
bool MovingPartBase::
do_update(PartBundle *root, const CycleData *root_cdata, PartGroup *parent,
          bool parent_changed, bool anim_changed,
          Thread *current_thread) {
  bool any_changed = false;
  bool needs_update = anim_changed;

  // See if any of the channel values have changed since last time.

  if (!needs_update) {
    if (_forced_channel != nullptr) {
      needs_update = _forced_channel->has_changed(0, 0.0, 0, 0.0);

    } else if (_effective_control != nullptr) {
      const PartBundle::CData *cdata = (const PartBundle::CData *)root_cdata;
      needs_update = _effective_control->channel_has_changed(_effective_channel, cdata->_frame_blend_flag);

    } else {
      const PartBundle::CData *cdata = (const PartBundle::CData *)root_cdata;
      PartBundle::ChannelBlend::const_iterator bci;
      for (bci = cdata->_blend.begin();
           !needs_update && bci != cdata->_blend.end();
           ++bci) {
        AnimControl *control = (*bci).first;

        AnimChannelBase *channel = nullptr;
        int channel_index = control->get_channel_index();
        if (channel_index >= 0 && channel_index < (int)_channels.size()) {
          channel = _channels[channel_index];
        }
        if (channel != nullptr) {
          needs_update = control->channel_has_changed(channel, cdata->_frame_blend_flag);
        }
      }
    }
  }

  if (needs_update) {
    // Ok, get the latest value.
    get_blend_value(root);
  }

  if (parent_changed || needs_update) {
    any_changed = update_internals(root, parent, needs_update, parent_changed,
                                   current_thread);
  }

  // Now recurse.
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    if ((*ci)->do_update(root, root_cdata, this,
                         parent_changed || needs_update,
                         anim_changed, current_thread)) {
      any_changed = true;
    }
  }

  return any_changed;
}


/**
 * This is called by do_update() whenever the part or some ancestor has
 * changed values.  It is a hook for derived classes to update whatever cache
 * they may have that depends on these.
 *
 * The return value is true if the part has changed as a result of the update,
 * or false otherwise.
 */
bool MovingPartBase::
update_internals(PartBundle *, PartGroup *, bool, bool, Thread *) {
  return true;
}

/**
 * Walks the part hierarchy, looking for a suitable channel index number to
 * use.  Available index numbers are the elements of the holes set, as well as
 * next to infinity.
 */
void MovingPartBase::
pick_channel_index(plist<int> &holes, int &next) const {
  // Verify each of the holes.

  plist<int>::iterator ii, ii_next;
  ii = holes.begin();
  while (ii != holes.end()) {
    ii_next = ii;
    ++ii_next;

    int hole = (*ii);
    nassertv(hole >= 0 && hole < next);
    if (hole < (int)_channels.size() ||
        _channels[hole] != nullptr) {
      // We can't accept this hole; we're using it!
      holes.erase(ii);
    }
    ii = ii_next;
  }

  // Now do we have any more to restrict?
  if (next < (int)_channels.size()) {
    int i;
    for (i = next; i < (int)_channels.size(); i++) {
      if (_channels[i] == nullptr) {
        // Here's a hole we do have.
        holes.push_back(i);
      }
    }
    next = _channels.size();
  }

  PartGroup::pick_channel_index(holes, next);
}



/**
 * Binds the indicated anim hierarchy to the part hierarchy, at the given
 * channel index number.
 */
void MovingPartBase::
bind_hierarchy(AnimGroup *anim, int channel_index, int &joint_index,
               bool is_included, BitArray &bound_joints,
               const PartSubset &subset) {
  if (subset.matches_include(get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(get_name())) {
    is_included = false;
  }

  if (chan_cat.is_debug()) {
    if (anim == nullptr) {
      chan_cat.debug()
        << "binding " << *this << " to NULL, is_included = "
        << is_included << "\n";
    } else {
      chan_cat.debug()
        << "binding " << *this << " to " << *anim << ", is_included = "
        << is_included << "\n";
    }
  }
  while ((int)_channels.size() <= channel_index) {
    _channels.push_back(nullptr);
  }

  nassertv(_channels[channel_index] == nullptr);

  if (is_included) {
    if (anim == nullptr) {
      // If we're binding to the NULL anim, it means actually to create a
      // default AnimChannel that just returns the part's initial value.
      _channels[channel_index] = make_default_channel();
    } else {
      _channels[channel_index] = DCAST(AnimChannelBase, anim);
    }

    // Record that we have bound this joint in the bound_joints BitArray.
    bound_joints.set_bit(joint_index);
  } else {
    // Record that we have *not* bound this particular joint.
    bound_joints.clear_bit(joint_index);
  }
  ++joint_index;

  PartGroup::bind_hierarchy(anim, channel_index, joint_index,
                            is_included, bound_joints, subset);
}

/**
 * Similar to bind_hierarchy, but does not actually perform any binding.  All
 * it does is compute the BitArray bount_joints according to the specified
 * subset.  This is useful in preparation for asynchronous binding--in this
 * case, we may need to know bound_joints immediately, without having to wait
 * for the animation itself to load and bind.
 */
void MovingPartBase::
find_bound_joints(int &joint_index, bool is_included, BitArray &bound_joints,
                  const PartSubset &subset) {
  if (subset.matches_include(get_name())) {
    is_included = true;
  } else if (subset.matches_exclude(get_name())) {
    is_included = false;
  }

  bound_joints.set_bit_to(joint_index, is_included);
  ++joint_index;

  PartGroup::find_bound_joints(joint_index, is_included, bound_joints, subset);
}

/**
 * Should be called whenever the ChannelBlend values have changed, this
 * recursively updates the _effective_channel member in each part.
 */
void MovingPartBase::
determine_effective_channels(const CycleData *root_cdata) {
  _effective_control = nullptr;
  _effective_channel = nullptr;

  AnimControl *effective_control = nullptr;
  AnimChannelBase *effective_channel = nullptr;
  int num_effective_channels = 0;

  const PartBundle::CData *cdata = (const PartBundle::CData *)root_cdata;
  PartBundle::ChannelBlend::const_iterator cbi;
  for (cbi = cdata->_blend.begin();
       cbi != cdata->_blend.end();
       ++cbi) {
    AnimControl *control = (*cbi).first;
    int channel_index = control->get_channel_index();
    if (channel_index >= 0 && channel_index < (int)_channels.size()) {
      if (_channels[channel_index] != nullptr) {
        effective_control = control;
        effective_channel = _channels[channel_index];
        ++num_effective_channels;
      }
    }
  }

  if (num_effective_channels == 1) {
    _effective_control = effective_control;
    _effective_channel = effective_channel;
  }

  PartGroup::determine_effective_channels(root_cdata);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MovingPartBase::
write_datagram(BamWriter *manager, Datagram &dg) {
  PartGroup::write_datagram(manager, dg);

  manager->write_pointer(dg, _forced_channel);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 *
 * This is the callback function that is made by the BamReader at some later
 * point, after all of the required pointers have been filled in.  It is
 * necessary because there might be forward references in a bam file; when we
 * call read_pointer() in fillin(), the object may not have been read from the
 * file yet, so we do not have a pointer available at that time.  Thus,
 * instead of returning a pointer, read_pointer() simply reserves a later
 * callback.  This function provides that callback.  The calling object is
 * responsible for keeping track of the number of times it called
 * read_pointer() and extracting the same number of pointers out of the
 * supplied vector, and storing them appropriately within the object.
 */
int MovingPartBase::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PartGroup::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 20) {
    _forced_channel = DCAST(AnimChannelBase, p_list[pi++]);
  }

  return pi;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MovingPartBase.
 */
void MovingPartBase::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartGroup::fillin(scan, manager);

  if (manager->get_file_minor_ver() >= 20) {
    manager->read_pointer(scan);
  }
}
