// Filename: gtkStatsGuide.h
// Created by:  drose (16Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GTKSTATSGUIDE_H
#define GTKSTATSGUIDE_H

#include <pandatoolbase.h>

#include <gtk--.h>

class PStatStripChart;

////////////////////////////////////////////////////////////////////
// 	 Class : GtkStatsGuide
// Description : A widget designed to be drawn next to a
//               GtkStatsStripChart that shows the labels associated
//               with the strip chart's guide bars.
////////////////////////////////////////////////////////////////////
class GtkStatsGuide : public Gtk::DrawingArea {
public:
  GtkStatsGuide(PStatStripChart *chart);

private:
  virtual gint configure_event_impl(GdkEventConfigure *event);
  virtual gint expose_event_impl(GdkEventExpose *event);

private:
  PStatStripChart *_chart;
};

#endif

