// Filename: fogAttribute.h
// Created by:  drose (24Mar00)
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

#ifndef FOGATTRIBUTE_H
#define FOGATTRIBUTE_H

#include <pandabase.h>

#include <onOffAttribute.h>
#include <fog.h>

////////////////////////////////////////////////////////////////////
//       Class : FogAttribute
// Description : See FogTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FogAttribute : public OnOffAttribute {
public:
  INLINE FogAttribute();

  INLINE void set_on(Fog *fog);
  INLINE Fog *get_fog() const;

public:
  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(Fog) _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffAttribute::init_type();
    register_type(_type_handle, "FogAttribute",
                  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fogAttribute.I"

#endif
