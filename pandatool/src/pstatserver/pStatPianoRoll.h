// Filename: pStatPianoRoll.h
// Created by:  drose (18Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PSTATPIANOROLL_H
#define PSTATPIANOROLL_H

#include <pandatoolbase.h>

#include "pStatGraph.h"
#include "pStatMonitor.h"
#include "pStatClientData.h"

#include <luse.h>
#include <vector_int.h>

#include <map>

class PStatFrameData;

////////////////////////////////////////////////////////////////////
// 	 Class : PStatPianoRoll
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

  INLINE void set_horizontal_scale(double time_width);
  INLINE double get_horizontal_scale() const;

  INLINE int timestamp_to_pixel(double time) const;
  INLINE double pixel_to_timestamp(int x) const;

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

  double _time_width;
  double _start_time;

  class ColorBar {
  public:
    double _start;
    double _end;
  };
  typedef vector<ColorBar> ColorBars;

  class BarBuilder {
  public:
    BarBuilder();
    void clear();
    void add_data_point(double time);
    void finish(double time);

    bool _is_new;
    ColorBars _color_bars;
  };
    
  typedef map<int, BarBuilder> PageData;
  PageData _page_data;
  int _current_frame;
};

#include "pStatPianoRoll.I"

#endif
