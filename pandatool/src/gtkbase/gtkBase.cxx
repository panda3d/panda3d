// Filename: gtkBase.cxx
// Created by:  drose (14Jul00)
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

#include "gtkBase.h"

#include "notify.h"

Gtk::Main *GtkBase::_gtk = NULL;

////////////////////////////////////////////////////////////////////
//     Function: GtkBase::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkBase::
GtkBase() {
  if (_gtk != (Gtk::Main *)NULL) {
    nout << "Invalid attempt to create multiple instances of GtkBase!\n";
    abort();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkBase::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkBase::
~GtkBase() {
  nassertv(_gtk != (Gtk::Main *)NULL);
  delete _gtk;
  _gtk = NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: GtkBase::parse_command_line
//       Access: Public, Virtual
//  Description: This is overridden for GtkBase to give Gtk a chance
//               to pull out its X-related parameters.
////////////////////////////////////////////////////////////////////
void GtkBase::
parse_command_line(int argc, char *argv[]) {
  nassertv(_gtk == (Gtk::Main *)NULL);
  _gtk = new Gtk::Main(argc, argv);
  ProgramBase::parse_command_line(argc, argv);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkBase::main_loop
//       Access: Public
//  Description: Call this after all is set up to yield control of the
//               main loop to Gtk.  This normally doesn't return.
////////////////////////////////////////////////////////////////////
void GtkBase::
main_loop() {
  nassertv(_gtk != NULL);

  _gtk->run();
}

