// Filename: basicGtkWindow.h
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

#ifndef BASICGTKWINDOW_H
#define BASICGTKWINDOW_H

#include <gtk--.h>


////////////////////////////////////////////////////////////////////
//       Class : BasicGtkWindow
// Description : This is just a handy wrapper around Gtk::Window that
//               provides some convenient setup functions.
////////////////////////////////////////////////////////////////////
class BasicGtkWindow : public Gtk::Window {
public:
  BasicGtkWindow(bool free_store = true);
  virtual ~BasicGtkWindow();
  void setup();
  virtual bool destruct();

protected:
  void delete_self();
  static gint static_delete(BasicGtkWindow *window);

private:
  void window_destroyed();
  gint idle_event();

  enum State {
    S_virgin,
    S_setup,
    S_ready,
    S_gone,
  };

  bool _destroyed;
  bool _free_store;
  State _state;
  SigC::Connection _destroy_connection;
};


#endif

