// Filename: colorTransition.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef COLORTRANSITION_H
#define COLORTRANSITION_H

#include <pandabase.h>

#include "colorProperty.h"

#include <onOffTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorTransition
// Description : This is scene graph color (which is not to be
//               confused with geometry color).  By setting a
//               ColorTransition on the scene graph, you can override
//               the color at the object level, but not at the
//               individual primitive or vertex level.  If a scene
//               graph color is set, it overrides the primitive color;
//               otherwise, the primitive color shows through.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorTransition : public OnOffTransition {
PUBLISHED:
  INLINE ColorTransition();
  INLINE ColorTransition(const Colorf &color);
  INLINE ColorTransition(float r, float g, float b, float a);
  INLINE static ColorTransition uncolor();
  INLINE static ColorTransition off();

  INLINE void set_on(const Colorf &color);
  INLINE void set_on(float r, float g, float b, float a);
  INLINE void set_uncolor();

  INLINE bool is_real() const;
  INLINE Colorf get_color() const;

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorProperty _value;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

protected:
  static TypedWritable *make_ColorTransition(const FactoryParams &params);
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffTransition::init_type();
    register_type(_type_handle, "ColorTransition",
                  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "colorTransition.I"

#endif
