// Filename: gtkStatsStripWindow.cxx
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

#include "gtkStatsStripWindow.h"
#include "gtkStatsStripChart.h"
#include "gtkStatsGuide.h"

#include "pStatCollectorDef.h"
#include "string_utils.h"

#include <stdio.h>  // for sprintf


using Gtk::Menu_Helpers::MenuElem;
using Gtk::Menu_Helpers::CheckMenuElem;
using Gtk::Menu_Helpers::SeparatorElem;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GtkStatsStripWindow::
GtkStatsStripWindow(GtkStatsMonitor *monitor, int thread_index,
                    int collector_index, bool show_level,
                    int chart_xsize, int chart_ysize) :
  GtkStatsWindow(monitor),
  _thread_index(thread_index),
  _collector_index(collector_index),
  _show_level(show_level)
{
  _title_unknown = false;
  _setup_scale_menu = false;
  _smooth = false;

  setup_menu();
  layout_window(chart_xsize, chart_ysize);

  new_collector();  // To set up the menus in case we can.
  show();
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::mark_dead
//       Access: Public, Virtual
//  Description: Called when the client's connection has been lost,
//               this should update the window in some obvious way to
//               indicate that the window is no longer live.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
mark_dead() {
  GtkStatsWindow::mark_dead();
  _chart->mark_dead();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::new_collector
//       Access: Public, Virtual
//  Description: Called when a new collector has become known.
//
//               For the GtkStatsStripWindow, this forces a rebuild of
//               the menu that selects the collectors available for
//               picking levels.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
new_collector() {
  const PStatClientData *client_data = _monitor->get_client_data();

  // Determine the set of collectors that display level data.  We'll
  // want to put these on the "Levels" pull-down menu.

  pset<int> levels;

  int num_collectors = client_data->get_num_collectors();
  for (int i = 0; i < num_collectors; i++) {
    if (client_data->has_collector(i) &&
        client_data->get_collector_has_level(i)) {
      // We only put top-level entries on the menu.  Thus, walk up
      // from this collector to its top level (the one below Frame).
      int collector_index = i;
      const PStatCollectorDef &def =
        client_data->get_collector_def(collector_index);
      int parent_index = def._parent_index;

      while (parent_index != 0) {
        collector_index = parent_index;
        const PStatCollectorDef &def =
          client_data->get_collector_def(collector_index);
        parent_index = def._parent_index;
      }

      levels.insert(collector_index);
    }
  }

  // Now put the collectors we found on the menu.
  _levels_menu->items().clear();
  pset<int>::const_iterator li;
  for (li = levels.begin(); li != levels.end(); ++li) {
    int collector_index = (*li);
    _levels_menu->items().push_back
      (MenuElem(client_data->get_collector_name(collector_index),
                bind(slot(this, &GtkStatsStripWindow::menu_show_levels), collector_index)));
  }

  // Also re-set-up the scale menu, in case the properties have changed.
  setup_scale_menu();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::idle
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
idle() {
  GtkStatsWindow::idle();
  _chart->update();

  const PStatThreadData *thread_data = _chart->get_view().get_thread_data();
  if (!thread_data->is_empty()) {
    float frame_rate = thread_data->get_frame_rate();
    char buffer[128];
    sprintf(buffer, "Frame rate: %0.1f Hz", frame_rate);
    _frame_rate_label->set_text(buffer);
  }

  if (_title_unknown) {
    _title_label->set_text(get_title_text());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::setup_menu
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
setup_menu() {
  GtkStatsWindow::setup_menu();

  Gtk::Menu *speed_menu = new Gtk::Menu;

  speed_menu->items().push_back
    (MenuElem("1",  // 1 chart width scrolls by per minute.
              bind(slot(this, &GtkStatsStripWindow::menu_hscale), 1.0f)));
  speed_menu->items().push_back
    (MenuElem("2",  // 2 chart widths scroll by per minute.
              bind(slot(this, &GtkStatsStripWindow::menu_hscale), 2.0f)));
  speed_menu->items().push_back
    (MenuElem("3",
              bind(slot(this, &GtkStatsStripWindow::menu_hscale), 3.0f)));
  speed_menu->items().push_back
    (MenuElem("6",
              bind(slot(this, &GtkStatsStripWindow::menu_hscale), 6.0f)));
  speed_menu->items().push_back
    (MenuElem("12",
              bind(slot(this, &GtkStatsStripWindow::menu_hscale), 12.0f)));

  _menu->items().push_back(MenuElem("Speed", *manage(speed_menu)));

  _scale_menu = new Gtk::Menu;
  _scale_menu->items().push_back
    (CheckMenuElem("Smooth",
                   slot(this, &GtkStatsStripWindow::menu_smooth)));
  _scale_menu->items().push_back(SeparatorElem());
  _scale_menu->items().push_back
    (MenuElem("Auto scale",
              slot(this, &GtkStatsStripWindow::menu_auto_vscale)));

  _menu->items().push_back(MenuElem("Scale", *manage(_scale_menu)));

  _levels_menu = new Gtk::Menu;
  _menu->items().push_back(MenuElem("Levels", *manage(_levels_menu)));
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::setup_scale_menu
//       Access: Protected
//  Description: Sets up the options on the scale menu.  We can't do
//               this until we have initialized the _chart member and
//               we have gotten our first collector_index.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
setup_scale_menu() {
  if (_setup_scale_menu) {
    // Already done it.
    return;
  }

  const PStatClientData *client_data = _monitor->get_client_data();
  if (!client_data->has_collector(_collector_index)) {
    // Can't set up the scale menu yet.
    return;
  }

  const PStatCollectorDef &def = client_data->get_collector_def(_collector_index);
  float base_scale = 1.0;
  string unit_name = def._level_units;

  if (_show_level) {
    _chart->set_guide_bar_unit_name(unit_name);
    _chart->set_guide_bar_units(PStatGraph::GBU_named);

  } else {
    _chart->set_guide_bar_units(PStatGraph::GBU_ms);
  }

  if (def._suggested_scale != 0.0) {
    base_scale = def._suggested_scale;
  } else if (!_show_level) {
    base_scale = 1.0 / _chart->get_target_frame_rate();
  }

  static const float scales[] = {
    50.0,
    10.0,
    5.0,
    2.0,
    1.0,
    0.5,
    0.2,
    0.1,
    0.02,
  };
  static const int num_scales = sizeof(scales) / sizeof(float);

  for (int i = 0; i < num_scales; i++) {
    float scale = base_scale * scales[i];
    string label;

    if (_show_level) {
      label = _chart->format_number(scale, PStatGraph::GBU_named | PStatGraph::GBU_show_units, unit_name);
    } else {
      label = _chart->format_number(scale, PStatGraph::GBU_ms | PStatGraph::GBU_hz | PStatGraph::GBU_show_units);
    }

    _scale_menu->items().push_back
      (MenuElem(label,
                bind(slot(this, &GtkStatsStripWindow::menu_vscale), scale)));
  }

  _setup_scale_menu = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_new_window
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_new_window() {
  new GtkStatsStripWindow(_monitor, _thread_index, _collector_index,
                          _show_level,
                          _chart->get_xsize(), _chart->get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_hscale
//       Access: Protected
//  Description: Selects a new horizontal scale for the strip chart.
//               This is done from the menu called "Speed", since
//               changing the horizontal scale most obviously affects
//               the scrolling speed.
//
//               The units is in chart width per minute.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_hscale(float wpm) {
  _chart->set_horizontal_scale(60.0 / wpm);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_vscale
//       Access: Protected
//  Description: Selects a new vertical scale for the strip chart.
//               This is done from the menu called "Scale".
//
//               The units is in seconds, or whatever the units
//               of choice are.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_vscale(float max_height) {
  _chart->set_vertical_scale(max_height);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_smooth
//       Access: Protected
//  Description: Toggles the "smooth" state of the check menu.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_smooth() {
  _smooth = !_smooth;

  _chart->set_average_mode(_smooth);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_auto_vscale
//       Access: Protected
//  Description: Selects a suitable vertical scale based on the data
//               already visible in the chart.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_auto_vscale() {
  _chart->set_auto_vertical_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_show_levels
//       Access: Protected
//  Description: Shows the level values known for the indicated
//               collector.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_show_levels(int collector_index) {
  new GtkStatsStripWindow(_monitor, _thread_index, collector_index,
                          true,
                          _chart->get_xsize(), _chart->get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::open_subchart
//       Access: Protected
//  Description: This is called in response to the collector_picked
//               signal from the strip chart, which is generated when
//               the user double-clicks on a band of color or a label.
//
//               This opens up a new window focusing just on the
//               indicated collector.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
open_subchart(int collector_index) {
  new GtkStatsStripWindow(_monitor, _thread_index, collector_index,
                          _show_level,
                          _chart->get_xsize(), _chart->get_ysize());
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::layout_window
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
layout_window(int chart_xsize, int chart_ysize) {
  Gtk::HBox *hbox = new Gtk::HBox;
  hbox = new Gtk::HBox;
  hbox->show();
  _main_box->pack_start(*manage(hbox), true, true, 8);

  Gtk::Table *chart_table = new Gtk::Table(3, 2);
  chart_table->show();
  hbox->pack_start(*manage(chart_table), true, true, 8);

  Gtk::HBox *title_hbox = new Gtk::HBox;
  title_hbox->show();
  chart_table->attach(*manage(title_hbox), 1, 2, 0, 1,
                      (GTK_FILL|GTK_EXPAND), 0);

  _title_label = new Gtk::Label(get_title_text());
  if (_collector_index != 0 || _thread_index != 0) {
    _title_label->show();
    _title_label->set_alignment(0.0, 0.5);
    title_hbox->pack_start(*manage(_title_label), true, true);
  }

  _frame_rate_label = new Gtk::Label;
  if (_collector_index == 0) {
    _frame_rate_label->show();
    _frame_rate_label->set_alignment(1.0, 0.5);
    title_hbox->pack_start(*manage(_frame_rate_label), true, true);
  }

  Gtk::Frame *frame = new Gtk::Frame;
  frame->set_shadow_type(GTK_SHADOW_ETCHED_OUT);
  frame->show();
  chart_table->attach(*manage(frame), 1, 2, 1, 2);

  if (_show_level) {
    _chart = new GtkStatsStripChart(_monitor,
                                    _monitor->get_level_view(_collector_index, _thread_index),
                                    _collector_index,
                                    chart_xsize, chart_ysize);
  } else {
    _chart = new GtkStatsStripChart(_monitor,
                                    _monitor->get_view(_thread_index),
                                    _collector_index,
                                    chart_xsize, chart_ysize);
  }

  _chart->collector_picked.
    connect(slot(this, &GtkStatsStripWindow::open_subchart));
  frame->add(*manage(_chart));

  chart_table->attach(*_chart->get_labels(), 0, 1, 1, 2,
                      0, (GTK_FILL|GTK_EXPAND), 4, 0);
  chart_table->attach(*_chart->get_guide(), 2, 3, 1, 2,
                      0, (GTK_FILL|GTK_EXPAND), 4, 0);
  _chart->show();
}


////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::get_title_text
//       Access: Private
//  Description: Returns the text suitable for the title label on the
//               top line.
////////////////////////////////////////////////////////////////////
string GtkStatsStripWindow::
get_title_text() {
  string text;

  _title_unknown = false;

  const PStatClientData *client_data = _monitor->get_client_data();
  if (client_data->has_collector(_collector_index)) {
    const PStatCollectorDef &def = client_data->get_collector_def(_collector_index);
    if (_show_level) {
      if (def._level_units.empty()) {
        text = def._name;
      } else {
        text = def._name + " (" + def._level_units + ")";
      }
    } else {
      text = def._name + " time";
    }
  } else {
    _title_unknown = true;
  }

  if (_thread_index != 0) {
    if (client_data->has_thread(_thread_index)) {
      text += "(" + client_data->get_thread_name(_thread_index) + " thread)";
    } else {
      _title_unknown = true;
    }
  }

  return text;
}

