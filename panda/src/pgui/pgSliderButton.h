// Filename: pgSliderButton.h
// Created by:  masad (21Oct04)
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

#ifndef PGSLIDERBUTTON_H
#define PGSLIDERBUTTON_H

#include "pandabase.h"

#include "pgButton.h"
#include "nodePath.h"
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : PGSliderButton
// Description : This is a particular kind of PGItem that is
//               specialized to behave like a normal button object.
//               It keeps track of its own state, and handles mouse
//               events sensibly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGSliderButton : public PGButton {
PUBLISHED:
  PGSliderButton(const string &name);
  virtual ~PGSliderButton();

public:

  virtual void move(const MouseWatcherParameter &param);
  
PUBLISHED:

  INLINE bool is_drag_n_drop();
  INLINE void set_drag_n_drop(bool value);
  INLINE void set_slider_bar(PGItem *item);

public:
  
  bool _drag_n_drop;
  PGItem *_slider_bar;

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGButton::init_type();
    register_type(_type_handle, "PGSliderButton",
                  PGButton::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "pgSliderButton.I"

#endif
