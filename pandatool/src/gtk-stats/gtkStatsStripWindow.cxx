// Filename: gtkStatsStripWindow.cxx
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "gtkStatsStripWindow.h"
#include "gtkStatsStripChart.h"
#include "gtkStatsGuide.h"

#include <string_utils.h>
#include <stdio.h>  // for sprintf


using Gtk::Menu_Helpers::MenuElem;
using Gtk::Menu_Helpers::SeparatorElem;

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GtkStatsStripWindow::
GtkStatsStripWindow(GtkStatsMonitor *monitor, int thread_index, 
		    int collector_index, int chart_xsize, int chart_ysize) :
  GtkStatsWindow(monitor),
  _thread_index(thread_index),
  _collector_index(collector_index)
{
  _title_unknown = false;

  setup_menu();
  layout_window(chart_xsize, chart_ysize);
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
//     Function: GtkStatsStripWindow::idle
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
idle() {
  _chart->update();

  const PStatThreadData *thread_data = _chart->get_view().get_thread_data();
  if (!thread_data->is_empty()) {
    double frame_rate = thread_data->get_frame_rate();
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
	      bind(slot(this, &GtkStatsStripWindow::menu_hscale), 1.0)));
  speed_menu->items().push_back
    (MenuElem("2",  // 2 chart widths scroll by per minute.
	      bind(slot(this, &GtkStatsStripWindow::menu_hscale), 2.0)));
  speed_menu->items().push_back
    (MenuElem("3", 
	      bind(slot(this, &GtkStatsStripWindow::menu_hscale), 3.0)));
  speed_menu->items().push_back
    (MenuElem("6", 
	      bind(slot(this, &GtkStatsStripWindow::menu_hscale), 6.0)));
  speed_menu->items().push_back
    (MenuElem("12", 
	      bind(slot(this, &GtkStatsStripWindow::menu_hscale), 12.0)));

  _menu->items().push_back(MenuElem("Speed", *manage(speed_menu)));


  Gtk::Menu *scale_menu = new Gtk::Menu;

  scale_menu->items().push_back
    (MenuElem("10000 ms (0.1 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 0.1)));
  scale_menu->items().push_back
    (MenuElem("1000 ms (1 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 1.0)));
  scale_menu->items().push_back
    (MenuElem("200 ms (5 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 5.0)));
  scale_menu->items().push_back
    (MenuElem("100 ms (10 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 10.0)));
  scale_menu->items().push_back
    (MenuElem("50.0 ms (20 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 20.0)));
  scale_menu->items().push_back
    (MenuElem("33.3 ms (30 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 30.0)));
  scale_menu->items().push_back
    (MenuElem("16.7 ms (60 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 60.0)));
  scale_menu->items().push_back
    (MenuElem("8.3 ms (120 Hz)",
	      bind(slot(this, &GtkStatsStripWindow::menu_vscale), 120.0)));

  _menu->items().push_back(MenuElem("Scale", *manage(scale_menu)));
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_new_window
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_new_window() {
  new GtkStatsStripWindow(_monitor, _thread_index, _collector_index,
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
menu_hscale(double wpm) {
  _chart->set_horizontal_scale(60.0 / wpm);
}

////////////////////////////////////////////////////////////////////
//     Function: GtkStatsStripWindow::menu_vscale
//       Access: Protected
//  Description: Selects a new vertical scale for the strip chart.
//               This is done from the menu called "Scale".
//
//               The units is in Hz.
////////////////////////////////////////////////////////////////////
void GtkStatsStripWindow::
menu_vscale(double hz) {
  _chart->set_vertical_scale(1.0 / hz);
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

  _chart = new GtkStatsStripChart(_monitor, 
				  _monitor->get_view(_thread_index), 
				  _collector_index,
				  chart_xsize, chart_ysize);
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
    text = client_data->get_collector_name(_collector_index) + " time";
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

