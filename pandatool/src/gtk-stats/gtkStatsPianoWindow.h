// Filename: gtkStatsPianoWindow.h
// Created by:  drose (18Jul00)
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

#ifndef GTKSTATSPIANOWINDOW_H
#define GTKSTATSPIANOWINDOW_H

#include "pandatoolbase.h"

#include "gtkStatsMonitor.h"
#include "gtkStatsWindow.h"

class GtkStatsPianoRoll;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsPianoWindow
// Description : A window that contains a GtkStatsPianoRoll.
////////////////////////////////////////////////////////////////////
class GtkStatsPianoWindow : public GtkStatsWindow {
public:
  GtkStatsPianoWindow(GtkStatsMonitor *monitor, int thread_index,
                      int chart_xsize, int chart_ysize);

  virtual void mark_dead();
  virtual void idle();

protected:
  virtual void setup_menu();
  virtual void menu_new_window();
  void menu_hscale(float hz);

private:
  void layout_window(int chart_xsize, int chart_ysize);

private:
  int _thread_index;

  GtkStatsPianoRoll *_chart;
};


#endif

