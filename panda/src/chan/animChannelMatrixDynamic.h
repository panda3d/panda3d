/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixDynamic.h
 * @author drose
 * @date 2003-10-20
 */

#ifndef ANIMCHANNELMATRIXDYNAMIC_H
#define ANIMCHANNELMATRIXDYNAMIC_H

#include "pandabase.h"

#include "animChannel.h"
#include "transformState.h"
#include "pandaNode.h"
#include "pointerTo.h"

/**
 * An animation channel that accepts a matrix each frame from some dynamic
 * input provided by code.
 *
 * This object operates in two modes: in explicit mode, the programmer should
 * call set_value() each frame to indicate the new value; in implicit mode,
 * the programmer should call set_value_node() to indicate the node whose
 * transform will be copied to the joint each frame.
 */
class EXPCL_PANDA_CHAN AnimChannelMatrixDynamic : public AnimChannelMatrix {
protected:
  AnimChannelMatrixDynamic();
  AnimChannelMatrixDynamic(AnimGroup *parent, const AnimChannelMatrixDynamic &copy);

public:
  AnimChannelMatrixDynamic(const std::string &name);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, LMatrix4 &mat);

  virtual void get_value_no_scale_shear(int frame, LMatrix4 &value);
  virtual void get_scale(int frame, LVecBase3 &scale);
  virtual void get_hpr(int frame, LVecBase3 &hpr);
  virtual void get_quat(int frame, LQuaternion &quat);
  virtual void get_pos(int frame, LVecBase3 &pos);
  virtual void get_shear(int frame, LVecBase3 &shear);

PUBLISHED:
  void set_value(const LMatrix4 &value);
  void set_value(const TransformState *value);
  void set_value_node(PandaNode *node);

  INLINE const TransformState *get_value_transform() const;
  INLINE PandaNode *get_value_node() const;

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

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

  static TypedWritable *make_AnimChannelMatrixDynamic(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannelMatrix::init_type();
    register_type(_type_handle, "AnimChannelMatrixDynamic",
                  AnimChannelMatrix::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelMatrixDynamic.I"

#endif
