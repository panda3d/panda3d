// Filename: pStatGraph.h
// Created by:  drose (19Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATGRAPH_H
#define PSTATGRAPH_H

#include <pandatoolbase.h>

#include "pStatMonitor.h"
#include "pStatClientData.h"

#include <luse.h>
#include <vector_int.h>

#include <map>

class PStatView;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatGraph
// Description : This is an abstract base class for several different
//               kinds of graphs that have a few things in common,
//               like labels and guide bars.
////////////////////////////////////////////////////////////////////
class PStatGraph {
public:
  PStatGraph(PStatMonitor *monitor, int xsize, int ysize);
  virtual ~PStatGraph();

  INLINE PStatMonitor *get_monitor() const;

  INLINE int get_num_labels() const;
  INLINE int get_label_collector(int n) const;
  INLINE string get_label_name(int n) const;
  INLINE RGBColorf get_label_color(int n) const;

  INLINE void set_target_frame_rate(double frame_rate);
  INLINE double get_target_frame_rate() const;

  INLINE int get_xsize() const;
  INLINE int get_ysize() const;

  class GuideBar {
  public:
    double _height;
    string _label;
    bool _is_target;
  };

  enum GuideBarUnits {
    GBU_hz         = 0x0001,
    GBU_ms         = 0x0002,
    GBU_show_units = 0x0004,
  };

  int get_num_guide_bars() const;
  const GuideBar &get_guide_bar(int n) const;

  INLINE void set_guide_bar_units(int unit_mask);
  INLINE int get_guide_bar_units() const;

protected:
  virtual void normal_guide_bars()=0;
  void update_guide_bars(int num_bars, double scale);
  GuideBar make_guide_bar(double time) const;

  bool _labels_changed;
  bool _guide_bars_changed;

  PT(PStatMonitor) _monitor;

  double _target_frame_rate;

  int _xsize;
  int _ysize;

  // Table of the collectors that should be drawn as labels, in order
  // from bottom to top.
  typedef vector_int Labels;
  Labels _labels;

  typedef vector<GuideBar> GuideBars;
  GuideBars _guide_bars;
  int _guide_bar_units;
};

#include "pStatGraph.I"

#endif
