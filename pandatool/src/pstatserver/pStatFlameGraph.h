/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatFlameGraph.h
 * @author rdb
 * @date 2022-01-28
 */

#ifndef PSTATFLAMEGRAPH_H
#define PSTATFLAMEGRAPH_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include "pmap.h"
#include "pdeque.h"

class PStatFrameData;

/**
 * This is an abstract class that presents the interface for drawing a flame
 * chart: it shows the time spent in each of a number of collectors
 * as a horizontal bar of color, with time as the horizontal axis.
 *
 * This class just manages all the flame chart logic; the actual nuts and bolts
 * of drawing pixels is left to a user-derived class.
 */
class PStatFlameGraph : public PStatGraph {
public:
  PStatFlameGraph(PStatMonitor *monitor,
                  int thread_index, int collector_index, int frame_number,
                  int xsize, int ysize);
  virtual ~PStatFlameGraph();

  void update();

  INLINE int get_thread_index() const;
  INLINE int get_collector_index() const;
  void set_collector_index(int collector_index);
  void push_collector_index(int collector_index);
  bool pop_collector_index();
  INLINE void clear_history();
  INLINE size_t get_history_depth() const;

  INLINE int get_frame_number() const;
  void set_frame_number(int collector_index);
  bool first_frame();
  bool next_frame();
  bool prev_frame();
  bool last_frame();

  INLINE double get_horizontal_scale() const;

  INLINE void set_average_mode(bool average_mode);
  INLINE bool get_average_mode() const;

  INLINE int height_to_pixel(double value) const;
  INLINE double pixel_to_height(int y) const;

  INLINE bool is_title_unknown() const;
  std::string get_title_text();

  std::string get_bar_tooltip(int depth, int x) const;
  int get_bar_collector(int depth, int x) const;

  virtual void write_datagram(Datagram &dg) const final;
  virtual void read_datagram(DatagramIterator &scan) final;

protected:
  void update_data();
  void changed_size(int xsize, int ysize);
  void force_redraw();
  virtual void normal_guide_bars();

  virtual void begin_draw();
  virtual void draw_bar(int depth, int from_x, int to_x,
                        int collector_index, int parent_index);
  virtual void end_draw();
  virtual void idle();

  bool animate(double time, double dt);

private:
  static const size_t _num_average_frames = 150;

  class StackLevel {
  public:
    void reset();

    StackLevel *start(int collector_index, double time);
    StackLevel *stop(int collector_index, double time);
    StackLevel *stop_all(double time);

    INLINE double get_net_value(bool average) const;
    void reset_averages();
    bool update_averages(size_t cursor);

    const StackLevel *locate(int depth, double time, bool average) const;

    void clear();

  private:
    StackLevel *r_stop(int collector_index, double time);

    double _net_value = 0.0;
    double _avg_net_value = 0.0;

    // This is updated like a ring buffer, initialized with all the same value
    // at first, then always at _average_cursor.
    double _values[_num_average_frames] = {0.0};

    double _start_time = 0.0;
    int _count = 0;
    bool _started = false;

    int _collector_index = -1;
    StackLevel *_parent = nullptr;
    pmap<int, StackLevel> _children;

    friend class PStatFlameGraph;
  };

  void r_draw_level(const StackLevel &level, int depth = 0, double offset = 0.0);

  StackLevel _stack;
  int _thread_index;
  int _collector_index;
  int _orig_collector_index;
  int _frame_number;
  bool _average_mode;
  size_t _average_cursor;

  double _time_width;
  int _current_frame;
  bool _title_unknown;

  std::vector<int> _history;
};

#include "pStatFlameGraph.I"

#endif
