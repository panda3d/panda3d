// Filename: lightTransition.h
// Created by:  mike (19Jan99)
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

#ifndef LIGHTTRANSITION_H
#define LIGHTTRANSITION_H

#include <pandabase.h>

#include "light.h"
#include "pt_Light.h"
#include "lightNameClass.h"

#include <multiTransition.h>
#include <pointerTo.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTITRANSITION_LIGHT MultiTransition<PT_Light, LightNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTITRANSITION_LIGHT);

////////////////////////////////////////////////////////////////////
//       Class : LightTransition
// Description : The LightTransition allows specifying the set of
//               lights that affect the geometry in the scene graph.
//               If the set of lights is empty (e.g. under a
//               LightTransition::all_off() transition), then lighting
//               is disabled; otherwise, lighting is enabled with the
//               indicated lights in effect.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LightTransition : public MultiTransition<PT_Light, LightNameClass> {
PUBLISHED:
  LightTransition();
  LightTransition(const LightTransition &copy);
  ~LightTransition();
  INLINE static LightTransition all_off();

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;
  virtual NodeTransition *make_identity() const;
  virtual NodeTransition *make_initial() const;

#ifdef CPPPARSER
  // Interrogate seems to have difficulty figuring out that we do
  // implement this pure virtual function properly in MultiTransition.
  // For now, we'll pretend for interrogate's sake that it's also
  // implemented here, just so interrogate doesn't believe the class
  // is abstract.
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;
#endif

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void output_property(ostream &out, const PT_Light &prop) const;
  virtual void write_property(ostream &out, const PT_Light &prop,
                              int indent_level) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiTransition<PT_Light, LightNameClass>::init_type();
    register_type(_type_handle, "LightTransition",
                  MultiTransition<PT_Light, LightNameClass>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class LightAttribute;
};

#include "lightTransition.I"

#endif


