// Filename: builderProperties.h
// Created by:  drose (17Sep97)
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
#ifndef BUILDERPROPERTIES_H
#define BUILDERPROPERTIES_H

#include <pandabase.h>

#include "builderTypes.h"

#ifndef HAVE_IOSTREAM
// We assume if your C++ library defines <iostream.h>, then it also
// defines <stl_config.h>.
#include <stl_config.h>
#endif



////////////////////////////////////////////////////////////////////
//       Class : BuilderProperties
// Description : A class which defines several parameters used to
//               control specific behavior of the builder and mesher.
//               BuilderBucket inherits from this class, so each of
//               these properties may be set directly on a
//               BuilderBucket to control the geometry made with just
//               that bucket.  There may in this way be several
//               different sets of properties in effect at a given
//               time.
//
//               The initial values for these are set in the private
//               constructor to BuilderBucket, at the bottom of
//               builderBucket.cxx.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderProperties {
public:
  bool operator < (const BuilderProperties &other) const;
  void output(ostream &out) const;

  // If this is true, the mesher will be invoked to break up large
  // polygons and build triangles into tristrips wherever possible.
  bool _mesh;

  // If this is true, a pair of adjacent coplanar triangles that form
  // a quad may be replaced with a pair of triangles forming the same
  // quad but with the opposite diagonal, if this will help the
  // building of tristrips.
  bool _retesselate_coplanar;

  // If this is true, a coplanar fan may be treated as a single large
  // polygon, and then subdivided into a single tristrip, instead of
  // treating it as a fan.  This can sometimes help form longer
  // continuous tristrips.
  bool _unroll_fans;

  // The following three flags serve to illustrate the mesher's
  // effectiveness by coloring geometry.

  // This colors each created triangle strip with a random color.  The
  // first triangle of each strip is a little darker than the rest of
  // the strip.
  bool _show_tstrips;

  // This colors each rectangular group of quads--called a quadsheet
  // by the mesher--with a random color.  The mesher always creates
  // linear tristrips across whatever quadsheets it can identify.
  bool _show_qsheets;

  // This colors quads blue, and doesn't mesh them further.  Since a
  // large part of the mesher's algorithm is reassembling adjacent
  // triangles into quads, it's sometimes helpful to see where it
  // thinks the best quads lie.
  bool _show_quads;

  // This shows normals created by creating little colored line
  // segments to represent each normal.
  bool _show_normals;
  double _normal_scale;
  BuilderC _normal_color;

  // If this is true, large polygons will be subdivided into
  // triangles.  Otherwise, they will remain large polygons.
  bool _subdivide_polys;

  // This is the flatness tolerance below which two polygons will be
  // considered coplanar.  Making it larger makes it easier for the
  // mesher to reverse triangle diagonals to achieve a good mesh, at
  // the expense of precision of the surface.
  double _coplanar_threshold;

  // True if fans are to be considered at all.  Sometimes making fans
  // is more trouble than they're worth, since they tend to get in the
  // way of long tristrips.
  bool _consider_fans;

  // The maximum average angle of the apex of each triangle involved
  // in a fan.  If the triangles are more loosely packed than this,
  // don't consider putting them into a fan.
  double _max_tfan_angle;

  // The minimum number of triangles to be involved in a fan.  Setting
  // this number lower results in more fans, probably at the expense
  // of tristrips.  However, setting it to zero means no tfans will be
  // built (although fans may still be unrolled into tstrips if
  // _unroll_fans is true).
  int _min_tfan_tris;

protected:
  int sort_value() const;
};

INLINE ostream &operator << (ostream &out, const BuilderProperties &props) {
  props.output(out);
  return out;
}

#endif
