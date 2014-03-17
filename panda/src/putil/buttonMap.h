// Filename: buttonMap.h
// Created by:  rdb (07Mar14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BUTTONMAP_H
#define BUTTONMAP_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "buttonHandle.h"
#include "buttonRegistry.h"
#include "pmap.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonMap
// Description : This class represents a map containing all of the
//               buttons of a (keyboard) device, though it can also
//               be used as a generic mapping between ButtonHandles.
//               It maps an underlying 'raw' button to a 'virtual'
//               button, which may optionally be associated with an
//               appropriate platform-specific name for the button.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL ButtonMap : public TypedReferenceCount {
PUBLISHED:
  INLINE int get_num_buttons() const;
  INLINE ButtonHandle get_raw_button(int i) const;
  INLINE ButtonHandle get_mapped_button(int i) const;
  INLINE const string &get_mapped_button_label(int i) const;

  INLINE ButtonHandle get_mapped_button(ButtonHandle raw) const;
  INLINE ButtonHandle get_mapped_button(const string &raw_name) const;
  INLINE const string &get_mapped_button_label(ButtonHandle raw) const;
  INLINE const string &get_mapped_button_label(const string &raw_name) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  void map_button(ButtonHandle raw_button, ButtonHandle button, const string &label = "");

private:
  struct ButtonNode {
    ButtonHandle _raw;
    ButtonHandle _mapped;
    string _label;
  };

  pmap<int, ButtonNode> _button_map;
  pvector<ButtonNode*> _buttons;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "ButtonMap",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "buttonMap.I"

#endif
