// Filename: basicGtkWindow.cxx
// Created by:  drose (14Jul00)
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

#include "basicGtkWindow.h"
#include "gtkBase.h"


////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::Constructor
//       Access: Public
//  Description: The free_store parameter should be true if the window
//               object has been allocated from the free store (using
//               new) and can be safely deleted using delete when the
//               window is destroyed by the user, or false if this is
//               not the case.
////////////////////////////////////////////////////////////////////
BasicGtkWindow::
BasicGtkWindow(bool free_store) : _free_store(free_store) {
  _destroyed = false;
  _state = S_virgin;
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BasicGtkWindow::
~BasicGtkWindow() {
  destruct();
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::setup
//       Access: Public
//  Description: Call this after initializing the window.
////////////////////////////////////////////////////////////////////
void BasicGtkWindow::
setup() {
  _state = S_setup;
  _destroy_connection =
    destroy.connect(slot(this, &BasicGtkWindow::window_destroyed));
  show();

  // Calling show() sets in motion some X events that must flow
  // completely through the queue before we can safely hide() the
  // thing again.  To measure when this has happened, we'll drop our
  // own event into the queue.  When this event makes it through the
  // queue, we'll assume all relevant X events have also, and it will
  // be safe to hide the window.
  Gtk::Main::idle.connect(slot(this, &BasicGtkWindow::idle_event));
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::destruct
//       Access: Public, Virtual
//  Description: Call this to remove the window, etc.  It's not tied
//               directly to the real destructor because that seems to
//               just lead to trouble.  This returns true if it
//               actually destructed, or false if it had already
//               destructed previously and did nothing this time.
////////////////////////////////////////////////////////////////////
bool BasicGtkWindow::
destruct() {
  if (_state != S_gone) {
    // We must hide the window before we destruct, or it won't disappear
    // from the screen.  Strange.  But we also don't want to try to hide
    // the window if we're destructing because of a window_destroyed
    // event, so we check our little flag.
    if (!_destroyed && _state != S_virgin) {
      // Now, in case the window was never completely shown, we must
      // wait for that to happen before we can hide it.
      while (!_destroyed && _state == S_setup) {
        GtkBase::_gtk->iteration();
      }

      if (!_destroyed) {
        hide();
      }
    }
    _state = S_gone;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::delete_self
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
void BasicGtkWindow::
delete_self() {
  destruct();
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::window_destroyed
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void BasicGtkWindow::
window_destroyed() {
  _destroyed = true;
  destruct();

  // We should probably also delete the pointer here.  But maybe not.
  // Gtk-- is very mysterious about this, so we'll just let it maybe
  // leak.
}

////////////////////////////////////////////////////////////////////
//     Function: BasicGtkWindow::idle_event
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
gint BasicGtkWindow::
idle_event() {
  // Now, we're finally a bona fide window with all rights thereunto
  // appertaining.
  _state = S_ready;
  return false;
}

