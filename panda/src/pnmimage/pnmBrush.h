// Filename: pnmBrush.h
// Created by:  drose (01Feb07)
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

#ifndef PNMBRUSH_H
#define PNMBRUSH_H

#include "pandabase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "luse.h"

class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : PNMBrush
// Description : This class is used to control the shape and color of
//               the drawing operations performed by a PNMPainter
//               object.
//
//               Normally, you don't create a PNMBrush directly;
//               instead, use one of the static PNMBrush::make_*()
//               methods provided here.
//
//               A PNMBrush is used to draw the border of a polygon or
//               rectangle, as well as for filling its interior.  When
//               it is used to draw a border, the brush is "smeared"
//               over the border; when it is used to fill the
//               interior, it is tiled through the interior.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PNMIMAGE PNMBrush : public ReferenceCount {
protected:
  INLINE PNMBrush(double xc, double yc);

PUBLISHED:
  virtual ~PNMBrush();

  enum BrushEffect {
    BE_set,
    BE_blend,
    BE_darken,
    BE_lighten,
  };

  static PT(PNMBrush) make_transparent();
  static PT(PNMBrush) make_pixel(const LColord &color, BrushEffect effect = BE_blend);
  static PT(PNMBrush) make_spot(const LColord &color, double radius, bool fuzzy,
                                BrushEffect effect = BE_blend);
  static PT(PNMBrush) make_image(const PNMImage &image, double xc, double yc,
                                 BrushEffect effect = BE_blend);

public:
  INLINE double get_xc() const;
  INLINE double get_yc() const;

  virtual void draw(PNMImage &image, int x, int y, double pixel_scale)=0;
  virtual void fill(PNMImage &image, int xfrom, int xto, int y,
                    int xo, int yo)=0;

protected:
  double _xc, _yc;
};

#include "pnmBrush.I"

#endif
