// Filename: gtkBase.h
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

#ifndef GTKBASE_H
#define GTKBASE_H

#include "pandatoolbase.h"

#include "programBase.h"

#include <gtk--.h>

////////////////////////////////////////////////////////////////////
//       Class : GtkBase
// Description : This is a specialization of ProgramBase for programs
//               that use the Gtk-- GUI toolkit.
////////////////////////////////////////////////////////////////////
class GtkBase : public ProgramBase {
public:
  GtkBase();
  ~GtkBase();

  virtual void parse_command_line(int argc, char *argv[]);
  void main_loop();

public:
  static Gtk::Main *_gtk;
};

#endif


