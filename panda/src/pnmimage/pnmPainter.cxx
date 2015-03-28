// Filename: pnmPainter.cxx
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

#include "pnmPainter.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::Constructor
//       Access: Published
//  Description: The constructor stores a pointer to the PNMImage you
//               pass it, but it does not take ownership of the
//               object; you are responsible for ensuring that the
//               PNMImage does not destruct during the lifetime of the
//               PNMPainter object.
//
//               The xo, yo coordinates specify an optional offset for
//               fill coordinates.  If you are painting with a pattern
//               fill, these specify the virtual coordinates of the
//               upper-left corner of the image, which can allow you
//               to adjust the pattern to line up with nested images,
//               if necessary.
////////////////////////////////////////////////////////////////////
PNMPainter::
PNMPainter(PNMImage &image, int xo, int yo) :
  _image(image),
  _xo(xo), _yo(yo)
{
  _pen = PNMBrush::make_pixel(LColorf(0, 0, 0, 1));
  _fill = PNMBrush::make_pixel(LColorf(1, 1, 1, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::draw_line
//       Access: Published
//  Description: Draws an antialiased line on the PNMImage, using the
//               current pen.
////////////////////////////////////////////////////////////////////
void PNMPainter::
draw_line(float xa, float ya, float xb, float yb) {
  // Shift the line coordinates to position the center of the pen on
  // the line.
  xa -= (_pen->get_xc() - 0.5);
  xb -= (_pen->get_xc() - 0.5);
  ya -= (_pen->get_yc() - 0.5);
  yb -= (_pen->get_yc() - 0.5);

  // Compute the line delta.
  float xd = xb - xa;
  float yd = yb - ya;

  if (xa == xb && ya == yb) {
    // Just a single point.  Treat it as a very short horizontal line.
    xd = 1.0;
  }

  if (cabs(xd) > cabs(yd)) {
    // This line is more horizontal than vertical.
    if (xa < xb) {
      // Draw the line from left to right.
      int x_min = (int)cfloor(xa);
      int x_max = (int)cceil(xb);

      // The first point.
      draw_hline_point(x_min, xa, ya, xd, yd, 1.0 - (xa - x_min));

      // The middle points.
      for (int x = x_min + 1; x < x_max; ++x) {
        draw_hline_point(x, xa, ya, xd, yd, 1.0);
      }

      if (x_max != x_min) {
        // The last point.
        draw_hline_point(x_max, xa, ya, xd, yd, 1.0 - (x_max - xb));
      }

    } else {
      // Draw the line from right to left.
      int x_min = (int)cfloor(xb);
      int x_max = (int)cceil(xa);

      // The first point.
      draw_hline_point(x_max, xa, ya, xd, yd, 1.0 - (x_max - xa));

      // The middle points.
      for (int x = x_max - 1; x > x_min; --x) {
        draw_hline_point(x, xa, ya, xd, yd, 1.0);
      }

      if (x_max != x_min) {
        // The last point.
        draw_hline_point(x_min, xa, ya, xd, yd, 1.0 - (xb - x_min));
      }
    }

  } else {
    // This line is more vertical than horizontal.
    if (ya < yb) {
      // Draw the line from top to bottom.
      int y_min = (int)cfloor(ya);
      int y_max = (int)cceil(yb);

      // The first point.
      draw_vline_point(y_min, xa, ya, xd, yd, 1.0 - (ya - y_min));

      // The middle points.
      for (int y = y_min + 1; y < y_max; ++y) {
        draw_vline_point(y, xa, ya, xd, yd, 1.0);
      }

      if (y_max != y_min) {
        // The last point.
        draw_vline_point(y_max, xa, ya, xd, yd, 1.0 - (y_max - yb));
      }

    } else {
      // Draw the line from bottom to top.
      int y_min = (int)cfloor(yb);
      int y_max = (int)cceil(ya);

      // The first point.
      draw_vline_point(y_max, xa, ya, xd, yd, 1.0 - (y_max - ya));

      // The middle points.
      for (int y = y_max - 1; y > y_min; --y) {
        draw_vline_point(y, xa, ya, xd, yd, 1.0);
      }

      if (y_max != y_min) {
        // The last point.
        draw_vline_point(y_min, xa, ya, xd, yd, 1.0 - (yb - y_min));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMPainter::draw_rectangle
//       Access: Published
//  Description: Draws a filled rectangule on the PNMImage, using the
//               current pen for the outline, and the current fill
//               brush for the interior.
//
//               The two coordinates specify any two diagonally
//               opposite corners.
////////////////////////////////////////////////////////////////////
void PNMPainter::
draw_rectangle(float xa, float ya, float xb, float yb) {
  // Make (xa, ya) be the upper-left corner, and (xb, yb) the
  // lower-right.
  if (xa > xb) {
    float t = xa;
    xa = xb;
    xb = t;
  }
  if (ya > yb) {
    float t = ya;
    ya = yb;
    yb = t;
  }

  // First, fill the interior.
  int x_min = (int)cceil(xa);
  int x_max = (int)cfloor(xb);
  int y_min = (int)cceil(ya);
  int y_max = (int)cfloor(yb);
  for (int y = y_min; y <= y_max; ++y) {
    _fill->fill(_image, x_min, x_max, y, _xo, _yo);
  }

  // Then, draw the outline.
  draw_line(xa, ya, xa, yb);
  draw_line(xa, yb, xb, yb);
  draw_line(xb, yb, xb, ya);
  draw_line(xb, ya, xa, ya);
}
