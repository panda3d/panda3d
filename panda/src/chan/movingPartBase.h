// Filename: movingPartBase.h
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

#ifndef MOVINGPARTBASE_H
#define MOVINGPARTBASE_H

#include "pandabase.h"

#include "partGroup.h"
#include "partBundle.h"
#include "animChannelBase.h"

////////////////////////////////////////////////////////////////////
//       Class : MovingPartBase
// Description : This is the base class for a single animatable piece
//               that may be bound to one channel (or more, if
//               blending is in effect).  It corresponds to, for
//               instance, a single joint or slider of a character.
//
//               MovingPartBase does not have a particular value type.
//               See the derived template class, MovingPart, for this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MovingPartBase : public PartGroup {
protected:
  INLINE MovingPartBase(const MovingPartBase &copy);

public:
  MovingPartBase(PartGroup *parent, const string &name);

PUBLISHED:
  INLINE int get_max_bound() const;
  INLINE AnimChannelBase *get_bound(int n) const;

public:
  virtual TypeHandle get_value_type() const=0;
  virtual AnimChannelBase *make_initial_channel() const=0;
  virtual void write(ostream &out, int indent_level) const;
  virtual void write_with_value(ostream &out, int indent_level) const;
  virtual void output_value(ostream &out) const=0;

  virtual bool do_update(PartBundle *root, PartGroup *parent,
                         bool parent_changed, bool anim_changed);

  virtual void get_blend_value(const PartBundle *root)=0;
  virtual bool update_internals(PartGroup *parent, bool self_changed,
                                bool parent_changed);

protected:
  MovingPartBase();

  virtual void pick_channel_index(plist<int> &holes, int &next) const;
  virtual void bind_hierarchy(AnimGroup *anim, int channel_index);

  typedef pvector< PT(AnimChannelBase) > Channels;
  Channels _channels;

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




