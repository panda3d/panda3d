/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggJointNodePointer.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGJOINTNODEPOINTER_H
#define EGGJOINTNODEPOINTER_H

#include "pandatoolbase.h"

#include "eggJointPointer.h"

#include "eggGroup.h"
#include "pointerTo.h"

/**
 * This stores a pointer back to a <Joint> node.
 */
class EggJointNodePointer : public EggJointPointer {
public:
  EggJointNodePointer(EggObject *object);

  virtual int get_num_frames() const;
  virtual LMatrix4d get_frame(int n) const;
  virtual void set_frame(int n, const LMatrix4d &mat);

  virtual void do_finish_reparent(EggJointPointer *new_parent);
  virtual void move_vertices_to(EggJointPointer *new_joint);

  virtual bool do_rebuild(EggCharacterDb &db);
  virtual void expose(EggGroup::DCSType dcs_type);
  virtual void apply_default_pose(EggJointPointer *source_joint, int frame);

  virtual bool has_vertices() const;

  virtual EggJointPointer *make_new_joint(const std::string &name);

  virtual void set_name(const std::string &name);

private:
  PT(EggGroup) _joint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggJointPointer::init_type();
    register_type(_type_handle, "EggJointNodePointer",
                  EggJointPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
