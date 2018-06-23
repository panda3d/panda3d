/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterSlider.h
 * @author drose
 * @date 1999-03-03
 */

#ifndef CHARACTERSLIDER_H
#define CHARACTERSLIDER_H

#include "pandabase.h"

#include "movingPartScalar.h"

class CharacterVertexSlider;

/**
 * This is a morph slider within the character.  It's simply a single
 * floating-point value that animates generally between 0 and 1, that controls
 * the effects of one or more morphs within the character.
 */
class EXPCL_PANDA_CHAR CharacterSlider : public MovingPartScalar {
protected:
  CharacterSlider();
  CharacterSlider(const CharacterSlider &copy);

PUBLISHED:
  explicit CharacterSlider(PartGroup *parent, const std::string &name);
  virtual ~CharacterSlider();

public:
  virtual PartGroup *make_copy() const;

  virtual bool update_internals(PartBundle *root, PartGroup *parent,
                                bool self_changed, bool parent_changed,
                                Thread *current_thread);

private:
  typedef pset<CharacterVertexSlider *> VertexSliders;
  VertexSliders _vertex_sliders;

public:
  static void register_with_read_factory();

  static TypedWritable *make_CharacterSlider(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    MovingPartScalar::init_type();
    register_type(_type_handle, "CharacterSlider",
                  MovingPartScalar::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class CharacterVertexSlider;
};

#endif
