/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatStripChart.h
 * @author drose
 * @date 2000-07-15
 */

#ifndef PSTATSTRIPCHART_H
#define PSTATSTRIPCHART_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include "luse.h"
#include "vector_int.h"

#include "pmap.h"

class PStatView;

/**
 * This is an abstract class that presents the interface for drawing a basic
 * strip-chart, showing the relative value over an interval of time for
 * several different collectors, differentiated by bands of color.
 *
 * This class just manages all the strip-chart logic; the actual nuts and
 * bolts of drawing pixels is left to a user-derived class.
 */
class PStatStripChart : public PStatGraph {
public:
  PStatStripChart(PStatMonitor *monitor, PStatView &view,
                  int thread_index, int collector_index, int xsize, int ysize);
  virtual ~PStatStripChart();

  void new_data(int frame_number);
  void update();
  bool first_data() const;

  INLINE PStatView &get_view() const;
  INLINE int get_collector_index() const;
  void set_collector_index(int collector_index);

  INLINE void set_horizontal_scale(double time_width);
  INLINE double get_horizontal_scale() const;
  INLINE void set_vertical_scale(double value_height);
  void set_default_vertical_scale();
  void set_auto_vertical_scale();
  INLINE double get_vertical_scale() const;

  INLINE void set_scroll_mode(bool scroll_mode);
  INLINE bool get_scroll_mode() const;

  INLINE void set_average_mode(bool average_mode);
  INLINE bool get_average_mode() const;

  int get_collector_under_pixel(int xpoint, int ypoint);
  INLINE int timestamp_to_pixel(double time) const;
  INLINE double pixel_to_timestamp(int x) const;
  INLINE int height_to_pixel(double value) const;
  INLINE double pixel_to_height(int y) const;

  std::string get_title_text();
  bool is_title_unknown() const;

protected:
  class ColorData {
  public:
    unsigned short _collector_index;
    unsigned short _i;
    double _net_value;
  };
  typedef pvector<ColorData> FrameData;
  typedef pmap<int, FrameData> Data;

  static void accumulate_frame_data(FrameData &fdata,
                                    const FrameData &additional, double weight);
  static void scale_frame_data(FrameData &fdata, double factor);

  const FrameData &get_frame_data(int frame_number);
  void compute_average_pixel_data(PStatStripChart::FrameData &result,
                                  int &then_i, int &now_i, double now);
  double get_net_value(int frame_number) const;
  double get_average_net_value() const;

  void changed_size(int xsize, int ysize);
  void force_redraw();
  void force_reset();
  virtual void update_labels();
  virtual void normal_guide_bars();

  virtual void clear_region();
  virtual void copy_region(int start_x, int end_x, int dest_x);
  virtual void begin_draw(int from_x, int to_x);
  virtual void draw_slice(int x, int w, const FrameData &fdata);
  virtual void draw_empty(int x, int w);
  virtual void draw_cursor(int x);
  virtual void end_draw(int from_x, int to_x);
  virtual void idle();

  INLINE bool is_label_used(int collector_index) const;

private:
  void draw_frames(int first_frame, int last_frame);
  void draw_pixels(int first_pixel, int last_pixel);

  void clear_label_usage();
  void dec_label_usage(const FrameData &fdata);
  void inc_label_usage(const FrameData &fdata);

protected:
  int _thread_index;

private:
  PStatView &_view;
  int _collector_index;
  bool _scroll_mode;
  bool _average_mode;

  Data _data;

  int _next_frame;
  bool _first_data;
  int _cursor_pixel;

  int _level_index;

  double _time_width;
  double _start_time;
  double _value_height;
  bool _title_unknown;

  typedef vector_int LabelUsage;
  LabelUsage _label_usage;
};

#include "pStatStripChart.I"

#endif
