// Filename: request_initial_size.cxx
// Created by:  drose (15Jul00)
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

#include "request_initial_size.h"

static int
restore_usize(Gtk::Widget *widget) {
  widget->set_usize(0, 0);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: request_initial_size
//  Description: Gtk-- hack to request an initial size for a widget,
//               while still allowing the user to resize it smaller.
////////////////////////////////////////////////////////////////////
void
request_initial_size(Gtk::Widget &widget, int xsize, int ysize) {
  if (xsize != 0 || ysize != 0) {
    // I can't find a way to request an initial size for a general
    // widget.  The best I can do is specify its minimum size.
    widget.set_usize(xsize, ysize);

    // However, I don't want the minimum size to be enforced forever;
    // the user should be able to resize the window smaller if he wants
    // to.  Thus, this ugly hack: at the first idle signal, we return
    // the usize to 0.
    Gtk::Main::idle.connect(bind(slot(&restore_usize), &widget));
  }
}

