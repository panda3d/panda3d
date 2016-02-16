/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterVertexSlider.h
 * @author drose
 * @date 2005-03-28
 */

#ifndef CHARACTERVERTEXSLIDER_H
#define CHARACTERVERTEXSLIDER_H

#include "pandabase.h"
#include "characterSlider.h"
#include "vertexSlider.h"
#include "pointerTo.h"

/**
 * This is a specialization on VertexSlider that returns the slider value
 * associated with a particular CharacterSlider object.
 */
class EXPCL_PANDA_CHAR CharacterVertexSlider : public VertexSlider {
private:
  CharacterVertexSlider();

PUBLISHED:
  CharacterVertexSlider(CharacterSlider *char_slider);
  virtual ~CharacterVertexSlider();

  INLINE const CharacterSlider *get_char_slider() const;

  virtual PN_stdfloat get_slider() const;

private:
  PT(CharacterSlider) _char_slider;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexSlider::init_type();
    register_type(_type_handle, "CharacterVertexSlider",
                  VertexSlider::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CharacterSlider;
};

#include "characterVertexSlider.I"

#endif
