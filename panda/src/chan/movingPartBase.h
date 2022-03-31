/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartBase.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef MOVINGPARTBASE_H
#define MOVINGPARTBASE_H

#include "pandabase.h"

#include "partGroup.h"
#include "partBundle.h"
#include "animChannelBase.h"

/**
 * This is the base class for a single animatable piece that may be bound to
 * one channel (or more, if blending is in effect).  It corresponds to, for
 * instance, a single joint or slider of a character.
 *
 * MovingPartBase does not have a particular value type.  See the derived
 * template class, MovingPart, for this.
 */
class EXPCL_PANDA_CHAN MovingPartBase : public PartGroup {
protected:
  INLINE MovingPartBase(const MovingPartBase &copy);

public:
  MovingPartBase(PartGroup *parent, const std::string &name);

PUBLISHED:
  INLINE int get_max_bound() const;
  INLINE AnimChannelBase *get_bound(int n) const;

public:
  virtual TypeHandle get_value_type() const=0;
  virtual AnimChannelBase *make_default_channel() const=0;

PUBLISHED:
  virtual bool clear_forced_channel();
  virtual AnimChannelBase *get_forced_channel() const;

  virtual void write(std::ostream &out, int indent_level) const;
  virtual void write_with_value(std::ostream &out, int indent_level) const;
  virtual void output_value(std::ostream &out) const=0;

public:
  virtual bool do_update(PartBundle *root, const CycleData *root_cdata,
                         PartGroup *parent, bool parent_changed,
                         bool anim_changed, Thread *current_thread);

  virtual void get_blend_value(const PartBundle *root)=0;
  virtual bool update_internals(PartBundle *root, PartGroup *parent,
                                bool self_changed, bool parent_changed,
                                Thread *current_thread);

protected:
  MovingPartBase();

  virtual void pick_channel_index(plist<int> &holes, int &next) const;
  virtual void bind_hierarchy(AnimGroup *anim, int channel_index,
                              int &joint_index, bool is_included,
                              BitArray &bound_joints,
                              const PartSubset &subset);
  virtual void find_bound_joints(int &joint_index, bool is_included,
                                 BitArray &bound_joints,
                                 const PartSubset &subset);
  virtual void determine_effective_channels(const CycleData *root_cdata);

  // This is the vector of all channels bound to this part.
  typedef pvector< PT(AnimChannelBase) > Channels;
  Channels _channels;

  // This is the single channel that has an effect on this part, as determined
  // by determine_effective_channels().  It is only set if there is exactly
  // one channel that affects this part (i.e. num_effective_channels is 1).
  // If there are multiple channels, or no channels at all, it is NULL.
  AnimControl *_effective_control;
  PT(AnimChannelBase) _effective_channel;

  // This is the particular channel that's been forced to this part, via
  // set_forced_channel().  It overrides all of the above if set.
  PT(AnimChannelBase) _forced_channel;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    PartGroup::init_type();
    register_type(_type_handle, "MovingPartBase",
                  PartGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "movingPartBase.I"

#endif
