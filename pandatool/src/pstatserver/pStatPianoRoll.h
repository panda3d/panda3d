// Filename: pStatPianoRoll.h
// Created by:  drose (18Jul00)
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

#ifndef PSTATPIANOROLL_H
#define PSTATPIANOROLL_H

#include "pandatoolbase.h"

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include "luse.h"
#include <vector_int.h>

#include "pmap.h"

class PStatFrameData;

////////////////////////////////////////////////////////////////////
//       Class : PStatPianoRoll
// Description : This is an abstract class that presents the interface
//               for drawing a piano-roll type chart: it shows the
//               time spent in each of a number of collectors as a
//               horizontal bar of color, with time as the horizontal
//               axis.
//
//               This class just manages all the piano-roll logic; the
//               actual nuts and bolts of drawing pixels is left to a
//               user-derived class.
////////////////////////////////////////////////////////////////////
class PStatPianoRoll : public PStatGraph {
public:
  PStatPianoRoll(PStatMonitor *monitor, int thread_index,
                 int xsize, int ysize);
  virtual ~PStatPianoRoll();

  void update();

  INLINE void set_horizontal_scale(float time_width);
  INLINE float get_horizontal_scale() const;

  INLINE int timestamp_to_pixel(float time) const;
  INLINE float pixel_to_timestamp(int x) const;

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

  int _thread_index;

  float _time_width;
  float _start_time;

  class ColorBar {
  public:
    float _start;
    float _end;
  };
  typedef pvector<ColorBar> ColorBars;

  class BarBuilder {
  public:
    BarBuilder();
    void clear();
    void add_data_point(float time);
    void finish(float time);

    bool _is_new;
    ColorBars _color_bars;
  };

  typedef pmap<int, BarBuilder> PageData;
  PageData _page_data;
  int _current_frame;
};

#include "pStatPianoRoll.I"

#endif
