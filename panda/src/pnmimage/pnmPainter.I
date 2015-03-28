// Filename: pnmPainter.I
// Created by:  drose (02Feb07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////
 

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMPainter::
~PNMPainter() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::set_pen
//       Access: Published
//  Description: Specifies a PNMBrush that will be used for drawing
//               lines and edges.  If the brush is a bitmap brush, its
//               image will be smeared pixelwise along the line.
//
//               Unlike the PNMImage passed to the constructor, the
//               PNMPainter will take ownership of the pen.  It is not
//               necessary to keep a separate pointer to it.
////////////////////////////////////////////////////////////////////
INLINE void PNMPainter::
set_pen(PNMBrush *pen) {
  _pen = pen;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::get_pen
//       Access: Published
//  Description: Returns the current pen.  See set_pen().
////////////////////////////////////////////////////////////////////
INLINE PNMBrush *PNMPainter::
get_pen() const {
  return _pen;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::set_fill
//       Access: Published
//  Description: Specifies a PNMBrush that will be used for filling
//               in the interiors of objects.  If the brush is a
//               bitmap brush, its image will be tiled throughout the
//               space.
//
//               Unlike the PNMImage passed to the constructor, the
//               PNMPainter will take ownership of the fill brush.  It
//               is not necessary to keep a separate pointer to it.
////////////////////////////////////////////////////////////////////
INLINE void PNMPainter::
set_fill(PNMBrush *fill) {
  _fill = fill;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::get_fill
//       Access: Published
//  Description: Returns the current fill brush.  See set_fill().
////////////////////////////////////////////////////////////////////
INLINE PNMBrush *PNMPainter::
get_fill() const {
  return _fill;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::draw_point
//       Access: Published
//  Description: Draws an antialiased point on the PNMImage, using the
//               current pen.
////////////////////////////////////////////////////////////////////
INLINE void PNMPainter::
draw_point(float x, float y) {
  draw_line(x, y, x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::draw_hline_point
//       Access: Private
//  Description: Called within draw_line() to draw a single point of a
//               mostly-horizontal line.
////////////////////////////////////////////////////////////////////
INLINE void PNMPainter::
draw_hline_point(int x, float xa, float ya, float xd, float yd,
                 float pixel_scale) {
  float y = (yd * (x - xa) / xd) + ya;
  int ymax = (int)cceil(y);
  int ymin = (int)cfloor(y);
  if (ymax == ymin) {
    _pen->draw(_image, x, ymin, pixel_scale);
  } else {
    _pen->draw(_image, x, ymax, (y - ymin) * pixel_scale);
    _pen->draw(_image, x, ymin, (ymax - y) * pixel_scale);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::draw_vline_point
//       Access: Private
//  Description: Called within draw_line() to draw a single point of a
//               mostly-vertical line.
////////////////////////////////////////////////////////////////////
INLINE void PNMPainter::
draw_vline_point(int y, float xa, float ya, float xd, float yd,
                 float pixel_scale) {
  float x = (xd * (y - ya) / yd) + xa;
  int xmax = (int)cceil(x);
  int xmin = (int)cfloor(x);
  if (xmax == xmin) {
    _pen->draw(_image, xmin, y, pixel_scale);
  } else {
    _pen->draw(_image, xmax, y, (x - xmin) * pixel_scale);
    _pen->draw(_image, xmin, y, (xmax - x) * pixel_scale);
  }
}
