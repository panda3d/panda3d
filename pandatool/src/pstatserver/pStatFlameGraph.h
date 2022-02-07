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
  PStatFlameGraph(PStatMonitor *monitor, PStatView &view,
                  int thread_index, int collector_index,
                  int xsize, int ysize);
  virtual ~PStatFlameGraph();

  void update();

  INLINE PStatView &get_view() const;
  INLINE int get_collector_index() const;
  void set_collector_index(int collector_index);

  INLINE double get_horizontal_scale() const;

  INLINE void set_average_mode(bool average_mode);
  INLINE bool get_average_mode() const;

  INLINE int height_to_pixel(double value) const;
  INLINE double pixel_to_height(int y) const;

  INLINE bool is_title_unknown() const;
  std::string get_title_text();
  std::string get_label_tooltip(int collector_index) const;

protected:
  static const size_t _num_average_frames = 200;

  struct CollectorData {
    double _offset;
    double _net_value;
    int _depth;
    // This is updated like a ring buffer, initialized with all the same value
    // at first, then always at _average_cursor.
    double _values[_num_average_frames];
  };
  typedef pmap<int, CollectorData> Data;

  void update_data();
  void update_data(const PStatViewLevel *level, int depth, double &offset);
  void changed_size(int xsize, int ysize);
  void force_redraw();
  virtual void update_labels();
  virtual void update_label(int collector_index, int row, int x, int width)=0;
  virtual void normal_guide_bars();

  virtual void begin_draw();
  virtual void end_draw();
  virtual void idle();

private:
  void compute_page(const PStatFrameData &frame_data);

protected:
  int _thread_index;

private:
  PStatView &_view;
  int _collector_index;
  bool _average_mode;
  size_t _average_cursor;

  Data _data;

  double _time_width;
  int _current_frame;
  bool _title_unknown;
};

#include "pStatFlameGraph.I"

#endif
