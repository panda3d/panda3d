// Filename: characterJointBundle.h
// Created by:  drose (23Feb99)
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

#ifndef CHARACTERJOINTBUNDLE_H
#define CHARACTERJOINTBUNDLE_H

#include "pandabase.h"

#include "partBundle.h"
#include "partGroup.h"
#include "animControl.h"

class Character;

////////////////////////////////////////////////////////////////////
//       Class : CharacterJointBundle
// Description : The collection of all the joints and sliders in the
//               character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterJointBundle : public PartBundle {
protected:
  INLINE CharacterJointBundle(const CharacterJointBundle &copy);

public:
  CharacterJointBundle(const string &name = "");

PUBLISHED:
  INLINE Character *get_node() const;

public:
  virtual PartGroup *make_copy() const;

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


