// Filename: triangleRasterizer.h
// Created by:  drose (06Nov99)
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

#ifndef TRIANGLERASTERIZER_H
#define TRIANGLERASTERIZER_H

#include "fixedPoint.h"

#include "luse.h"
#include <pnmImage.h>

class StitchImage;

class RasterizerVertex {
public:
  LPoint2d _p;
  LPoint2d _uv;
  LVector3d _space;
  double _alpha;
  int _visibility;
};

class TriangleRasterizer {
public:
  TriangleRasterizer();

  void draw_triangle(const RasterizerVertex *v0,
                     const RasterizerVertex *v1,
                     const RasterizerVertex *v2);
  void draw_pixel(const RasterizerVertex *v0, double radius);

  PNMImage *_output;
  StitchImage *_input;
  bool _read_input;
  const PNMImage *_texture;
  bool _filter_output;
  Colord _untextured_color;

private:
  class Edge {
  public:
    Edge(const RasterizerVertex *v0, const RasterizerVertex *v1);

    const RasterizerVertex *_v0;  //  Y(v0) < Y(v1)
    const RasterizerVertex *_v1;
    double _dx;                   // X(v1) - X(v0)
    double _dy;                   // Y(v1) - Y(v0)
    FixedPoint _fdxdy;            // dx/dy in fixed-point
    FixedPoint _fsx;              // first sample point x coord
    FixedPoint _fsy;
    double _adjy;                 // adjust from v[0]->fy to fsy, scaled
    int _lines;                   // number of lines to be sampled on this edge
    FixedPoint _fx0;              // fixed pt X of lower endpoint
  };

  inline void filter_pixel(RGBColord &rgb, double &alpha,
                           double s, double t,
                           double dsdx, double dtdx, double dsdy, double dtdy);
  void filter_pixel(RGBColord &rgb, double &alpha,
                    double s, double t, double radius);

  void read_input();
};

#endif

