// Filename: characterSlider.h
// Created by:  drose (03Mar99)
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

#ifndef CHARACTERSLIDER_H
#define CHARACTERSLIDER_H

#include "pandabase.h"

#include "movingPartScalar.h"

class CharacterVertexSlider;

////////////////////////////////////////////////////////////////////
//       Class : CharacterSlider
// Description : This is a morph slider within the character.  It's
//               simply a single floating-point value that animates
//               generally between 0 and 1, that controls the effects
//               of one or more morphs within the character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CharacterSlider : public MovingPartScalar {
protected:
  CharacterSlider();
  CharacterSlider(const CharacterSlider &copy);

public:
  CharacterSlider(PartGroup *parent, const string &name);
  virtual ~CharacterSlider();

  virtual PartGroup *make_copy() const;

  virtual bool update_internals(PartGroup *parent, bool self_changed,
                                bool parent_changed);

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
  static TypeHandle get_class_type() {
    return _type_handle;
  }
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


