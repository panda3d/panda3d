// Filename: basicGtkDialog.h
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

#ifndef BASICGTKDIALOG_H
#define BASICGTKDIALOG_H

#include "basicGtkWindow.h"

#include <gtk--.h>


////////////////////////////////////////////////////////////////////
//       Class : BasicGtkDialog
// Description : This looks like a wrapper around Gtk::Dialog.
//               Actually, it doesn't inherit from Gtk::Dialog at all,
//               but instead (indirectly) from Gtk::Window; it just
//               duplicates the default functionality of Gtk::Dialog
//               by defining get_vbox() and a get_action_area().
////////////////////////////////////////////////////////////////////
class BasicGtkDialog : public BasicGtkWindow {
public:
  BasicGtkDialog(bool free_store = true);

  Gtk::VBox *get_vbox() const;
  Gtk::HBox *get_action_area() const;

private:
  Gtk::VBox *_vbox;
  Gtk::HBox *_action_area;
};


#endif
