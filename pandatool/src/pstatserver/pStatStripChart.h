// Filename: pStatStripChart.h
// Created by:  drose (15Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATSTRIPCHART_H
#define PSTATSTRIPCHART_H

#include <pandatoolbase.h>

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include <luse.h>
#include <vector_int.h>

#include <map>

class PStatView;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatStripChart
// Description : This is an abstract class that presents the interface
//               for drawing a basic strip-chart, showing the relative
//               time elapsed over an interval of time for several
//               different collectors, differentiated by bands of
//               color.
//
//               This class just manages all the strip-chart logic;
//               the actual nuts and bolts of drawing pixels is left
//               to a user-derived class.
////////////////////////////////////////////////////////////////////
class PStatStripChart : public PStatGraph {
public:
  PStatStripChart(PStatMonitor *monitor, PStatView &view,
		  int collector_index, int xsize, int ysize);
  virtual ~PStatStripChart();

  void new_data(int frame_number);
  void update();
  bool first_data() const;

  INLINE PStatView &get_view() const;
  INLINE int get_collector_index() const;

  INLINE void set_horizontal_scale(double time_width);
  INLINE double get_horizontal_scale() const;
  INLINE void set_vertical_scale(double time_height);
  INLINE void set_default_vertical_scale();
  INLINE double get_vertical_scale() const;

  INLINE void set_scroll_mode(bool scroll_mode);
  INLINE bool get_scroll_mode() const;

  int get_collector_under_pixel(int xpoint, int ypoint);
  INLINE int timestamp_to_pixel(double time) const;
  INLINE double pixel_to_timestamp(int x) const;
  INLINE int height_to_pixel(double elapsed_time) const;
  INLINE double pixel_to_height(int y) const;

protected:
  class ColorData {
  public:
    int _collector_index;
    double _net_time;
  };
  typedef vector<ColorData> FrameData;
  typedef map<int, FrameData> Data;

  const FrameData &get_frame_data(int frame_number);

  void changed_size(int xsize, int ysize);
  void force_redraw();
  void force_reset();
  void update_labels();
  virtual void normal_guide_bars();

  virtual void clear_region();
  virtual void copy_region(int start_x, int end_x, int dest_x);
  virtual void begin_draw(int from_x, int to_x);
  virtual void draw_slice(int x, int frame_number);
  virtual void draw_empty(int x);
  virtual void draw_cursor(int x);
  virtual void end_draw(int from_x, int to_x);
  virtual void idle();
  
private:
  void draw_frames(int first_frame, int last_frame);
  void draw_pixels(int first_pixel, int last_pixel);

  PStatView &_view;
  int _collector_index;
  bool _scroll_mode;

  Data _data;

  int _next_frame;
  bool _first_data;
  int _cursor_pixel;

  int _level_index;

  double _time_width;
  double _start_time;
  double _time_height;
};

#include "pStatStripChart.I"

#endif
