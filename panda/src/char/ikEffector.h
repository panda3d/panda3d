/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikEffector.h
 * @author rdb
 * @date 2020-11-16
 */

#ifndef IKEFFECTOR_H
#define IKEFFECTOR_H

#include "pandabase.h"

#include "partGroup.h"

/**
 * This object is placed at the end of a chain of joints in order to pull it
 * towards a particular position using Inverse Kinematics.
 */
class EXPCL_PANDA_CHAR IKEffector : public PartGroup {
protected:
  INLINE IKEffector(const IKEffector &copy);

PUBLISHED:
  explicit IKEffector(PartGroup *parent, PandaNode *node);

  bool r_init_ik(const LPoint3 &parent_pos);
  void r_forward_ik(const LPoint3 &parent_pos);
  bool r_reverse_ik(LPoint3 &out_pos);

protected:
  IKEffector();

  PT(PandaNode) _node;
  LPoint3 _ik_pos;
  PN_stdfloat _length;

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
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PartGroup::init_type();
    register_type(_type_handle, "IKEffector",
                  PartGroup::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "ikEffector.I"

#endif
