// Filename: showHideAttribute.h
// Created by:  drose (26Apr00)
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

#ifndef SHOWHIDEATTRIBUTE_H
#define SHOWHIDEATTRIBUTE_H

#include <pandabase.h>

#include "showHideNameClass.h"

#include <multiAttribute.h>
#include <pointerTo.h>
#include <camera.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTIATTRIBUTE_CAMERA MultiAttribute<PT(Camera), ShowHideNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTIATTRIBUTE_CAMERA);

////////////////////////////////////////////////////////////////////
//       Class : ShowHideAttribute
// Description : See ShowHideTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ShowHideAttribute : public MultiAttribute<PT(Camera), ShowHideNameClass> {
public:
  INLINE ShowHideAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

protected:
  virtual void output_property(ostream &out, const PT(Camera) &prop) const;
  virtual void write_property(ostream &out, const PT(Camera) &prop,
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
    MultiAttribute<PT(Camera), ShowHideNameClass>::init_type();
    register_type(_type_handle, "ShowHideAttribute",
                  MultiAttribute<PT(Camera), ShowHideNameClass>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "showHideAttribute.I"

#endif
