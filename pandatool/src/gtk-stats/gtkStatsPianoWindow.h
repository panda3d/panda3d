// Filename: gtkStatsPianoWindow.h
// Created by:  drose (18Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSPIANOWINDOW_H
#define GTKSTATSPIANOWINDOW_H

#include <pandatoolbase.h>

#include "gtkStatsMonitor.h"
#include "gtkStatsWindow.h"

class GtkStatsPianoRoll;

////////////////////////////////////////////////////////////////////
// 	 Class : GtkStatsPianoWindow
// Description : A window that contains a GtkStatsPianoRoll.
////////////////////////////////////////////////////////////////////
class GtkStatsPianoWindow : public GtkStatsWindow {
public:
  GtkStatsPianoWindow(GtkStatsMonitor *monitor, int thread_index, 
		      int chart_xsize, int chart_ysize);

  virtual void idle();

protected:
  virtual void setup_menu();
  virtual void menu_new_window();
  void menu_hscale(double hz);

private:
  void layout_window(int chart_xsize, int chart_ysize);

private:
  int _thread_index;
  
  GtkStatsPianoRoll *_chart;
};


#endif

