// Filename: pStatStripChart.h
// Created by:  drose (15Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PSTATSTRIPCHART_H
#define PSTATSTRIPCHART_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include "luse.h"
#include <vector_int.h>

#include "pmap.h"

class PStatView;

////////////////////////////////////////////////////////////////////
//       Class : PStatStripChart
// Description : This is an abstract class that presents the interface
//               for drawing a basic strip-chart, showing the relative
//               value over an interval of time for several different
//               collectors, differentiated by bands of color.
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

  INLINE void set_horizontal_scale(float time_width);
  INLINE float get_horizontal_scale() const;
  INLINE void set_vertical_scale(float value_height);
  void set_default_vertical_scale();
  void set_auto_vertical_scale();
  INLINE float get_vertical_scale() const;

  INLINE void set_scroll_mode(bool scroll_mode);
  INLINE bool get_scroll_mode() const;

  int get_collector_under_pixel(int xpoint, int ypoint);
  INLINE int timestamp_to_pixel(float time) const;
  INLINE float pixel_to_timestamp(int x) const;
  INLINE int height_to_pixel(float value) const;
  INLINE float pixel_to_height(int y) const;

  string get_title_text();
  bool is_title_unknown() const;

protected:
  class ColorData {
  public:
    int _collector_index;
    float _net_value;
  };
  typedef pvector<ColorData> FrameData;
  typedef pmap<int, FrameData> Data;

  const FrameData &get_frame_data(int frame_number);

  void changed_size(int xsize, int ysize);
  void force_redraw();
  void force_reset();
  virtual void update_labels();
  virtual void normal_guide_bars();

  virtual void clear_region();
  virtual void copy_region(int start_x, int end_x, int dest_x);
  virtual void begin_draw(int from_x, int to_x);
  virtual void draw_slice(int x, int w, int frame_number);
  virtual void draw_empty(int x, int w);
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

  float _time_width;
  float _start_time;
  float _value_height;
  bool _title_unknown;
};

#include "pStatStripChart.I"

#endif
