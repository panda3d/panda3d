// Filename: animChannelScalarDynamic.h
// Created by:  drose (20Oct03)
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

#ifndef ANIMCHANNELSCALARDYNAMIC_H
#define ANIMCHANNELSCALARDYNAMIC_H

#include "pandabase.h"

#include "animChannel.h"

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelScalarDynamic
// Description : An animation channel that accepts a scalar each frame
//               from some dynamic input provided by code.
//
//               This object operates in two modes: in explicit mode,
//               the programmer should call set_value() each frame to
//               indicate the new value; in implicit mode, the
//               programmer should call set_value_node() to indicate
//               the node whose X component will be copied to the
//               scalar each frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimChannelScalarDynamic : public AnimChannelScalar {
public:
  AnimChannelScalarDynamic(AnimGroup *parent, const string &name);
protected:
  AnimChannelScalarDynamic();

public:
  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, float &value);

PUBLISHED:
  void set_value(float value);
  void set_value_node(PandaNode *node);

private:
  // This is filled in only if we are using the set_value_node()
  // interface to get an implicit value from the transform on the
  // indicated node each frame.
  PT(PandaNode) _value_node;
  CPT(TransformState) _value;
  CPT(TransformState) _last_value;

  // This is used only if we are using the explicit set_value()
  // interface.
  bool _value_changed;
  float _float_value;

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
