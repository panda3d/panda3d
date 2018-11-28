/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelScalarDynamic.h
 * @author drose
 * @date 2003-10-20
 */

#ifndef ANIMCHANNELSCALARDYNAMIC_H
#define ANIMCHANNELSCALARDYNAMIC_H

#include "pandabase.h"

#include "animChannel.h"

class PandaNode;
class TransformState;

/**
 * An animation channel that accepts a scalar each frame from some dynamic
 * input provided by code.
 *
 * This object operates in two modes: in explicit mode, the programmer should
 * call set_value() each frame to indicate the new value; in implicit mode,
 * the programmer should call set_value_node() to indicate the node whose X
 * component will be copied to the scalar each frame.
 */
class EXPCL_PANDA_CHAN AnimChannelScalarDynamic : public AnimChannelScalar {
protected:
  AnimChannelScalarDynamic();
  AnimChannelScalarDynamic(AnimGroup *parent, const AnimChannelScalarDynamic &copy);

public:
  AnimChannelScalarDynamic(const std::string &name);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, PN_stdfloat &value);
  INLINE PN_stdfloat get_value() const;
  INLINE PandaNode *get_value_node() const;

PUBLISHED:
  void set_value(PN_stdfloat value);
  void set_value_node(PandaNode *node);

  MAKE_PROPERTY(value, get_value, set_value);
  MAKE_PROPERTY(value_node, get_value_node, set_value_node);

protected:
  virtual AnimGroup *make_copy(AnimGroup *parent) const;

private:
  // This is filled in only if we are using the set_value_node() interface to
  // get an implicit value from the transform on the indicated node each
  // frame.
  PT(PandaNode) _value_node;
  CPT(TransformState) _value;
  CPT(TransformState) _last_value;

  // This is used only if we are using the explicit set_value() interface.
  bool _value_changed;
  PN_stdfloat _float_value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

  static TypedWritable *make_AnimChannelScalarDynamic(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannelScalar::init_type();
    register_type(_type_handle, "AnimChannelScalarDynamic",
                  AnimChannelScalar::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelScalarDynamic.I"

#endif
