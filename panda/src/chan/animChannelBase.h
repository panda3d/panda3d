/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelBase.h
 * @author drose
 * @date 1999-02-19
 */

#ifndef ANIMCHANNELBASE_H
#define ANIMCHANNELBASE_H

#include "pandabase.h"

#include "animGroup.h"
#include "animControl.h"

#include "pointerTo.h"

/**
 * Parent class for all animation channels.  An AnimChannel is an arbitrary
 * function that changes over time (actually, over frames), usually defined by
 * a table read from an egg file (but possibly computed or generated in any
 * other way).
 */
class EXPCL_PANDA_CHAN AnimChannelBase : public AnimGroup {
protected:
  // The default constructor is protected: don't try to create an AnimChannel
  // without a parent.  To create an AnimChannel hierarchy, you must first
  // create an AnimBundle, and use that to create any subsequent children.
  INLINE AnimChannelBase(const std::string &name = "");
  INLINE AnimChannelBase(AnimGroup *parent, const AnimChannelBase &copy);

public:
  INLINE AnimChannelBase(AnimGroup *parent, const std::string &name);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);

  virtual TypeHandle get_value_type() const=0;

protected:

  int _last_frame;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

public:
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimGroup::init_type();
    register_type(_type_handle, "AnimChannelBase",
                  AnimGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelBase.I"

#endif
