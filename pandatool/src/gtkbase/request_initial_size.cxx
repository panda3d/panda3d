// Filename: request_initial_size.cxx
// Created by:  drose (15Jul00)
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

