// Filename: stitcher.h
// Created by:  drose (09Nov99)
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

#ifndef STITCHER_H
#define STITCHER_H

#include "luse.h"

#include <string>
#include <map>

class StitchPoint;
class StitchImage;

class Stitcher {
public:
  Stitcher();
  ~Stitcher();

  void add_image(StitchImage *image);
  void add_point(const string &name, const LVector3d &vec);
  void show_points(double radius, const Colord &color);
  void stitch();

  typedef vector<StitchImage *> Images;
  Images _placed;

  typedef vector<StitchPoint *> LoosePoints;
  LoosePoints _loose_points;
  bool _show_points;
  double _point_radius;
  Colord _point_color;

private:
  class MatchingPoint {
  public:
    MatchingPoint(StitchPoint *p, const LPoint2d &got_uv);

    StitchPoint *_p;
    LPoint2d _need_uv;
    LPoint2d _got_uv;
    LVector2d _diff;
  };
  typedef vector<MatchingPoint> MatchingPoints;

  int score_image(StitchImage *image);
  double stitch_image(StitchImage *image);
  void feather_image(StitchImage *image);

  double try_match(StitchImage *image, LMatrix3d &rot,
                   const MatchingPoints &mp, int zero, int one);

  Images _images;

  typedef map<string, StitchPoint *> Points;
  Points _points;
};


#endif
