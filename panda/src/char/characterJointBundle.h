/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointBundle.h
 * @author drose
 * @date 1999-02-23
 */

#ifndef CHARACTERJOINTBUNDLE_H
#define CHARACTERJOINTBUNDLE_H

#include "pandabase.h"

#include "partBundle.h"
#include "partGroup.h"
#include "animControl.h"

class Character;

/**
 * The collection of all the joints and sliders in the character.
 */
class EXPCL_PANDA_CHAR CharacterJointBundle : public PartBundle {
protected:
  INLINE CharacterJointBundle(const CharacterJointBundle &copy);

PUBLISHED:
  explicit CharacterJointBundle(const std::string &name = "");
  virtual ~CharacterJointBundle();

PUBLISHED:
  INLINE Character *get_node(int n) const;

protected:
  virtual PartGroup *make_copy() const;
  virtual void add_node(PartBundleNode *node);
  virtual void remove_node(PartBundleNode *node);

private:
  void r_set_character(PartGroup *group, Character *character);

public:
  static void register_with_read_factory();

  static TypedWritable *make_CharacterJointBundle(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PartBundle::init_type();
    register_type(_type_handle, "CharacterJointBundle",
                  PartBundle::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "characterJointBundle.I"

#endif
