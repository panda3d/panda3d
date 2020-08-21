/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file uvScrollNode.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef UVSCROLLNODE_H
#define UVSCROLLNODE_H

#include "pandabase.h"

#include "animInterface.h"
#include "pandaNode.h"


/**
 * This node is placed at key points within the scene graph to animate uvs.
 */
class EXPCL_PANDA_PGRAPHNODES UvScrollNode : public PandaNode {
PUBLISHED:
  INLINE explicit UvScrollNode(const std::string &name, PN_stdfloat u_speed, PN_stdfloat v_speed, PN_stdfloat w_speed, PN_stdfloat r_speed);
  INLINE explicit UvScrollNode(const std::string &name);

protected:
  INLINE UvScrollNode(const UvScrollNode &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_combine() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  INLINE void set_u_speed(PN_stdfloat u_speed);
  INLINE void set_v_speed(PN_stdfloat v_speed);
  INLINE void set_w_speed(PN_stdfloat w_speed);
  INLINE void set_r_speed(PN_stdfloat r_speed);
  INLINE PN_stdfloat get_u_speed() const;
  INLINE PN_stdfloat get_v_speed() const;
  INLINE PN_stdfloat get_w_speed() const;
  INLINE PN_stdfloat get_r_speed() const;

PUBLISHED:
  MAKE_PROPERTY(u_speed, get_u_speed, set_u_speed);
  MAKE_PROPERTY(v_speed, get_v_speed, set_v_speed);
  MAKE_PROPERTY(w_speed, get_w_speed, set_w_speed);
  MAKE_PROPERTY(r_speed, get_r_speed, set_r_speed);

private:
  PN_stdfloat _u_speed;
  PN_stdfloat _v_speed;
  PN_stdfloat _w_speed;
  PN_stdfloat _r_speed;

  double _start_time;

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
