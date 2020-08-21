/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMatrixTablePointer.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGMATRIXTABLEPOINTER_H
#define EGGMATRIXTABLEPOINTER_H

#include "pandatoolbase.h"

#include "eggJointPointer.h"

#include "eggTable.h"
#include "eggXfmSAnim.h"
#include "pointerTo.h"

/**
 * This stores a pointer back to an EggXfmSAnim table (i.e.  an <Xfm$Anim_S$>
 * entry in an egg file), corresponding to the animation data from a single
 * bundle for this joint.
 */
class EggMatrixTablePointer : public EggJointPointer {
public:
  EggMatrixTablePointer(EggObject *object);

  virtual double get_frame_rate() const;
  virtual int get_num_frames() const;
  virtual void extend_to(int num_frames);
  virtual LMatrix4d get_frame(int n) const;
  virtual void set_frame(int n, const LMatrix4d &mat);
  virtual bool add_frame(const LMatrix4d &mat);

  virtual void do_finish_reparent(EggJointPointer *new_parent);

  virtual bool do_rebuild(EggCharacterDb &db);

  virtual void optimize();
  virtual void zero_channels(const std::string &components);
  virtual void quantize_channels(const std::string &components, double quantum);

  virtual EggJointPointer *make_new_joint(const std::string &name);

  virtual void set_name(const std::string &name);

private:
  PT(EggTable) _table;
  PT(EggXfmSAnim) _xform;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggJointPointer::init_type();
    register_type(_type_handle, "EggMatrixTablePointer",
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
