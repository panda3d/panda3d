// Filename: gtkBase.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKBASE_H
#define GTKBASE_H

#include <pandatoolbase.h>

#include <programBase.h>

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


