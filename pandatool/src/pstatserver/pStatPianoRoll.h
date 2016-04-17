/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatPianoRoll.h
 * @author drose
 * @date 2000-07-18
 */

#ifndef PSTATPIANOROLL_H
#define PSTATPIANOROLL_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include "luse.h"
#include "vector_int.h"

#include "pmap.h"

class PStatFrameData;

/**
 * This is an abstract class that presents the interface for drawing a piano-
 * roll type chart: it shows the time spent in each of a number of collectors
 * as a horizontal bar of color, with time as the horizontal axis.
 *
 * This class just manages all the piano-roll logic; the actual nuts and bolts
 * of drawing pixels is left to a user-derived class.
 */
class PStatPianoRoll : public PStatGraph {
public:
  PStatPianoRoll(PStatMonitor *monitor, int thread_index,
                 int xsize, int ysize);
  virtual ~PStatPianoRoll();

  void update();

  INLINE void set_horizontal_scale(double time_width);
  INLINE double get_horizontal_scale() const;

  INLINE int timestamp_to_pixel(double time) const;
  INLINE double pixel_to_timestamp(int x) const;
  INLINE int height_to_pixel(double value) const;
  INLINE double pixel_to_height(int y) const;

protected:
  void changed_size(int xsize, int ysize);
  void force_redraw();
  virtual void normal_guide_bars();

  virtual void begin_draw();
  virtual void begin_row(int row);
  virtual void draw_bar(int row, int from_x, int to_x);
  virtual void end_row(int row);
  virtual void end_draw();
  virtual void idle();

private:
  void compute_page(const PStatFrameData &frame_data);

protected:
  int _thread_index;

private:
  double _time_width;
  double _start_time;

  class ColorBar {
  public:
    double _start;
    double _end;
  };
  typedef pvector<ColorBar> ColorBars;

  class BarBuilder {
  public:
    BarBuilder();
    void clear();
    void add_data_point(double time, bool is_start);
    void finish(double time);

    bool _is_new;
    ColorBars _color_bars;
  };

  typedef pmap<int, BarBuilder> PageData;
  PageData _page_data;
  int _current_frame;
};

#include "pStatPianoRoll.I"

#endif
