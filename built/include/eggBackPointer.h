/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBackPointer.h
 * @author drose
 * @date 2001-02-26
 */

#ifndef EGGBACKPOINTER_H
#define EGGBACKPOINTER_H

#include "pandatoolbase.h"

#include "typedObject.h"

/**
 * This stores a pointer from an EggJointData or EggSliderData object back to
 * the referencing data in an egg file.  One of these objects corresponds to
 * each model appearing in an egg file, and may reference either a single
 * node, or a table, or a slew of vertices and primitives, depending on the
 * type of data stored.
 *
 * This is just an abstract base class.  The actual details are stored in the
 * various subclasses.
 */
class EggBackPointer : public TypedObject {
public:
  EggBackPointer();

  virtual double get_frame_rate() const;
  virtual int get_num_frames() const=0;
  virtual void extend_to(int num_frames);
  virtual bool has_vertices() const;

  virtual void set_name(const std::string &name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "EggBackPointer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
