// Filename: gtkStatsStripWindow.h
// Created by:  drose (14Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSSTRIPWINDOW_H
#define GTKSTATSSTRIPWINDOW_H

#include <pandatoolbase.h>

#include "gtkStatsMonitor.h"
#include "gtkStatsWindow.h"

class GtkStatsStripChart;

////////////////////////////////////////////////////////////////////
// 	 Class : GtkStatsStripWindow
// Description : A window that contains your basic one-thread,
//               one-level strip chart.
////////////////////////////////////////////////////////////////////
class GtkStatsStripWindow : public GtkStatsWindow {
public:
  GtkStatsStripWindow(GtkStatsMonitor *monitor, int thread_index, 
		      int collector_index, int chart_xsize, int chart_ysize);

  virtual void mark_dead();
  virtual void idle();

protected:
  virtual void setup_menu();
  virtual void menu_new_window();
  void menu_hscale(double wpm);
  void menu_vscale(double hz);
  void open_subchart(int collector_index);

private:
  void layout_window(int chart_xsize, int chart_ysize);
  string get_title_text();

private:
  int _thread_index;
  int _collector_index;
  bool _title_unknown;
  
  Gtk::Label *_title_label;
  Gtk::Label *_frame_rate_label;
  GtkStatsStripChart *_chart;
};


#endif

