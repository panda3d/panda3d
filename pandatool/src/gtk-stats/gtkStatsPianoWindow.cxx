// Filename: gtkStatsPianoWindow.cxx
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

#include "gtkStatsPianoWindow.h"
#include "gtkStatsPianoRoll.h"

using Gtk::Menu_Helpers::MenuElem;
using Gtk::Menu_Helpers::SeparatorElem;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsPianoWindow::
GtkStatsPianoWindow(GtkStatsMonitor *monitor, int thread_index,
                    int chart_xsize, int chart_ysize) :
  GtkStatsWindow(monitor),
  _thread_index(thread_index)
{
  setup_menu();
  layout_window(chart_xsize, chart_ysize);
  show();
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::mark_dead
//       Access: Public, Virtual
//  Description: Called when the client's connection has been lost,
//               this should update the window in some obvious way to
//               indicate that the window is no longer live.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
mark_dead() {
  GtkStatsWindow::mark_dead();
  _chart->mark_dead();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::idle
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
idle() {
  GtkStatsWindow::idle();
  _chart->update();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::setup_menu
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
setup_menu() {
  GtkStatsWindow::setup_menu();

  Gtk::Menu *scale_menu = new Gtk::Menu;

  scale_menu->items().push_back
    (MenuElem("0.1 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 0.1f)));
  scale_menu->items().push_back
    (MenuElem("1 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 1.0f)));
  scale_menu->items().push_back
    (MenuElem("5 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 5.0f)));
  scale_menu->items().push_back
    (MenuElem("10 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 10.0f)));
  scale_menu->items().push_back
    (MenuElem("20 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 20.0f)));
  scale_menu->items().push_back
    (MenuElem("30 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 30.0f)));
  scale_menu->items().push_back
    (MenuElem("60 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 60.0f)));
  scale_menu->items().push_back
    (MenuElem("120 Hz",
              bind(slot(this, &GtkStatsPianoWindow::menu_hscale), 120.0f)));

  _menu->items().push_back(MenuElem("Scale", *manage(scale_menu)));
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::menu_new_window
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
menu_new_window() {
  new GtkStatsPianoWindow(_monitor, _thread_index,
                          _chart->get_xsize(), _chart->get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::menu_hscale
//       Access: Protected
//  Description: Selects a new horizontal scale for the piano roll.
//               This is done from the menu called "Scale".
//
//               The units is in Hz.
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
menu_hscale(float hz) {
  _chart->set_horizontal_scale(1.0 / hz);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsPianoWindow::layout_window
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsPianoWindow::
layout_window(int chart_xsize, int chart_ysize) {
  Gtk::HBox *hbox = new Gtk::HBox;
  hbox = new Gtk::HBox;
  hbox->show();
  _main_box->pack_start(*manage(hbox), true, true, 8);

  Gtk::Table *chart_table = new Gtk::Table(1, 2);
  chart_table->show();
  hbox->pack_start(*manage(chart_table), true, true, 8);

  Gtk::Frame *frame = new Gtk::Frame;
  frame->set_shadow_type(GTK_SHADOW_ETCHED_OUT);
  frame->show();
  chart_table->attach(*manage(frame), 1, 2, 0, 1);

  _chart = new GtkStatsPianoRoll(_monitor, _thread_index,
                                  chart_xsize, chart_ysize);
  frame->add(*manage(_chart));

  // We put the labels in a frame, too, so they'll line up vertically.
  Gtk::Frame *label_frame = new Gtk::Frame;
  label_frame->set_shadow_type(GTK_SHADOW_NONE);
  label_frame->show();
  label_frame->add(*manage(_chart->get_labels()));

  chart_table->attach(*manage(label_frame), 0, 1, 0, 1,
                      0, (GTK_FILL|GTK_EXPAND), 4, 0);

  _chart->show();
}
