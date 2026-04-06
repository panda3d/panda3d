/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggJointPointer.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGJOINTPOINTER_H
#define EGGJOINTPOINTER_H

#include "pandatoolbase.h"
#include "eggBackPointer.h"
#include "eggGroup.h"
#include "luse.h"

class EggCharacterDb;

/**
 * This is a base class for EggJointNodePointer and EggMatrixTablePointer.  It
 * stores a back pointer to either a <Joint> entry or an xform <Table> data,
 * and thus presents an interface that returns 1-n matrices, one for each
 * frame.  (<Joint> entries, for model files, appear the same as one-frame
 * animations.)
 */
class EggJointPointer : public EggBackPointer {
public:
  virtual int get_num_frames() const=0;
  virtual LMatrix4d get_frame(int n) const=0;
  virtual void set_frame(int n, const LMatrix4d &mat)=0;
  virtual bool add_frame(const LMatrix4d &mat);

  virtual void do_finish_reparent(EggJointPointer *new_parent)=0;
  virtual void move_vertices_to(EggJointPointer *new_joint);

  virtual bool do_rebuild(EggCharacterDb &db);

  virtual void optimize();
  virtual void expose(EggGroup::DCSType dcs_type);
  virtual void zero_channels(const std::string &components);
  virtual void quantize_channels(const std::string &components, double quantum);
  virtual void apply_default_pose(EggJointPointer *source_joint, int frame);

  virtual EggJointPointer *make_new_joint(const std::string &name)=0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggJointPointer",
                  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggJointPointer.I"

#endif
