// Filename: gtkStatsStripWindow.h
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

#ifndef GTKSTATSSTRIPWINDOW_H
#define GTKSTATSSTRIPWINDOW_H

#include "pandatoolbase.h"

#include "gtkStatsMonitor.h"
#include "gtkStatsWindow.h"

class GtkStatsStripChart;

////////////////////////////////////////////////////////////////////
//       Class : GtkStatsStripWindow
// Description : A window that contains your basic one-thread,
//               one-level strip chart.
////////////////////////////////////////////////////////////////////
class GtkStatsStripWindow : public GtkStatsWindow {
public:
  GtkStatsStripWindow(GtkStatsMonitor *monitor, int thread_index,
                      int collector_index, bool show_level,
                      int chart_xsize, int chart_ysize);

  virtual void mark_dead();
  virtual void new_collector();
  virtual void idle();

protected:
  virtual void setup_menu();
  void setup_scale_menu();
  virtual void menu_new_window();
  void menu_hscale(float wpm);
  void menu_vscale(float max_height);
  void menu_auto_vscale();
  void menu_show_levels(int collector_index);
  void open_subchart(int collector_index);

private:
  void layout_window(int chart_xsize, int chart_ysize);
  string get_title_text();

private:
  int _thread_index;
  int _collector_index;
  bool _show_level;
  bool _title_unknown;
  bool _setup_scale_menu;

  Gtk::Label *_title_label;
  Gtk::Label *_frame_rate_label;
  GtkStatsStripChart *_chart;

  Gtk::Menu *_scale_menu;
  Gtk::Menu *_levels_menu;
};


#endif

