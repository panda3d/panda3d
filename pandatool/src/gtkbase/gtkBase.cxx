// Filename: gtkBase.cxx
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "gtkBase.h"

#include <notify.h>

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

