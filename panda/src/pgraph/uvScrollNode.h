// Filename: modelNode.h
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef UVSCROLLNODE_H
#define UVSCROLLNODE_H

#include "pandabase.h"

#include "modelNode.h"

////////////////////////////////////////////////////////////////////
//       Class : UvScrollNode
// Description : This node is placed at key points within the scene
//               graph to animate uvs. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH UvScrollNode : public ModelNode {
PUBLISHED:
  INLINE UvScrollNode(const string &name, float u_speed, float v_speed);

protected:
  INLINE UvScrollNode(const UvScrollNode &copy);

public:
  virtual PandaNode *make_copy() const;

PUBLISHED:
  INLINE void set_u_speed(float u_speed);
  INLINE void set_v_speed(float v_speed);

private:
  float _u_speed;
  float _v_speed;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "UvScrollNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "uvScrollNode.I"

#endif


