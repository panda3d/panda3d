// Filename: gtkStats.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATS_H
#define GTKSTATS_H

#include <pandatoolbase.h>

#include <gtkBase.h>

class GtkStatsMainWindow;

////////////////////////////////////////////////////////////////////
//       Class : GtkStats
// Description : A fancy graphical pstats server written using gtk+
//               (actually, Gtk--, the C++ layer over gtk+).
////////////////////////////////////////////////////////////////////
class GtkStats : public GtkBase {
public:
  GtkStats();

  void run();
  static void quit();

  int _port;
  static GtkStatsMainWindow *_main_window;
};

#endif

