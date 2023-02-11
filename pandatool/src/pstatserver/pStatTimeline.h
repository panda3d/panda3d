/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatTimeline.h
 * @author rdb
 * @date 2022-02-11
 */

#ifndef PSTATTIMELINE_H
#define PSTATTIMELINE_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"
#include "pdeque.h"

class PStatFrameData;

/**
 * This is an abstract class that presents the interface for drawing a piano-
 * roll type chart: it shows the time spent in each of a number of collectors
 * as a horizontal bar of color, with time as the horizontal axis.
 *
 * This class just pnages all the piano-roll logic; the actual nuts and bolts
 * of drawing pixels is left to a user-derived class.
 */
class PStatTimeline : public PStatGraph {
public:
  PStatTimeline(PStatMonitor *monitor, int xsize, int ysize);
  virtual ~PStatTimeline();

  void new_data(int thread_index, int frame_number);
  bool update_bars(int thread_index, int frame_number);

  INLINE void set_horizontal_scale(double time_width);
  INLINE double get_horizontal_scale() const;
  INLINE void set_horizontal_scroll(double time_start);
  INLINE double get_horizontal_scroll() const;

  INLINE void zoom_to(double time_width, double pivot);
  INLINE void zoom_by(double amount, double center);
  INLINE void scroll_to(double time_start);
  INLINE void scroll_by(double time_start);

  INLINE int timestamp_to_pixel(double time) const;
  INLINE double pixel_to_timestamp(int x) const;
  INLINE int height_to_pixel(double value) const;
  INLINE double pixel_to_height(int y) const;
  INLINE int get_num_rows() const;

  std::string get_bar_tooltip(int row, int x) const;

  virtual void write_datagram(Datagram &dg) const final;
  virtual void read_datagram(DatagramIterator &scan) final;

protected:
  void changed_size(int xsize, int ysize);
  void force_redraw();
  void force_redraw(int row, int from_x, int to_x);
  void normal_guide_bars();

  virtual void clear_region();
  virtual void begin_draw();
  void draw_thread(int thread_index, double start_time, double end_time);
  void draw_row(int thread_index, int row_index, double start_time, double end_time);
  virtual void draw_separator(int row);
  virtual void draw_guide_bar(int x, GuideBarStyle style);
  virtual void draw_bar(int row, int from_x, int to_x, int collector_index,
                        const std::string &collector_name);
  virtual void end_draw();
  virtual void idle();

  bool animate(double time, double dt);

  class ColorBar {
  public:
    double _start, _end;
    int _collector_index;
    int _thread_index;
    int _frame_number;
    bool _open_begin : 8, _open_end : 8;

    bool operator < (const ColorBar &other) const {
      return _end < other._end;
    }
  };
  typedef pvector<ColorBar> Row;
  typedef pvector<Row> Rows;

  bool find_bar(int row, int x, ColorBar &bar) const;

  class ThreadRow {
  public:
    std::string _label;
    Rows _rows;
    size_t _row_offset = 0;
    int _last_frame = -1;
    bool _visible = false;
  };
  typedef pvector<ThreadRow> ThreadRows;
  ThreadRows _threads;
  bool _threads_changed = true;
  double _clock_skew = 0.0;
  int _app_collector_index = -1;

  enum KeyFlag {
    F_left = 1,
    F_right = 2,
    F_w = 4,
    F_a = 8,
    F_s = 16,
    F_d = 32,
  };
  int _keys_held = 0;
  double _scroll_speed = 0.0;
  double _zoom_speed = 0.0;
  double _zoom_center = 0.0;

private:
  double _time_scale;
  double _start_time = 0.0;
  double _lowest_start_time = 0.0;
  double _highest_end_time = 0.0;
  bool _have_start_time = false;
  double _target_start_time = 0.0;
  double _target_time_scale;
};

#include "pStatTimeline.I"

#endif
