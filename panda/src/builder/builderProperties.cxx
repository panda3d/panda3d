// Filename: builderProperties.cxx
// Created by:  drose (17Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "builderProperties.h"

////////////////////////////////////////////////////////////////////
//     Function: BuilderProperties::Ordering operator
//       Access: Public
//  Description: Defines an arbitrary ordering among different
//               properties, and groups identical sets of properties
//               together.  This operator is used for the more
//               important task of grouping BuilderBuckets together;
//               see the comments for the similar function in
//               builderBucket.cxx.
////////////////////////////////////////////////////////////////////
bool BuilderProperties::
operator < (const BuilderProperties &other) const {
  int sv1 = sort_value();
  int sv2 = other.sort_value();

  if (sv1 != sv2) {
    return sv1 < sv2;
  }

  if (_coplanar_threshold != other._coplanar_threshold) {
    return _coplanar_threshold < other._coplanar_threshold;
  }

  if (_max_tfan_angle != other._max_tfan_angle) {
    return _max_tfan_angle < other._max_tfan_angle;
  }

  if (_min_tfan_tris != other._min_tfan_tris) {
    return _min_tfan_tris < other._min_tfan_tris;
  }

  if (_show_normals) {
    if (_normal_scale != other._normal_scale) {
      return _normal_scale < other._normal_scale;
    }
    if (_normal_color != other._normal_color) {
      return _normal_color < other._normal_color;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::output
//       Access: Public
//  Description: Outputs the properties meaningfully.
////////////////////////////////////////////////////////////////////
void BuilderProperties::
output(ostream &out) const {
  if (_mesh) {
    out << "T-Mesh using Mesher\n";

    if (_show_tstrips) {
      out << "Color individual tristrips\n";
    } else if (_show_qsheets) {
      out << "Color individual quadsheets\n";
    } else if (_show_quads) {
      out << "Color individual quads\n";
    }
    if (_retesselate_coplanar) {
      out << "Retesselate coplanar triangles when needed; threshold = "
          << _coplanar_threshold << "\n";
    }
  }
  if (_subdivide_polys) {
    out << "Subdivide polygons into tris.\n";
  }
  if (_consider_fans) {
    out << "Look for possible triangle fans with max per-triangle angle of "
        << _max_tfan_angle << " degrees.\n";
    if (_min_tfan_tris==0) {
      out << "Do not create tfans";
    } else {
      out << "Do not create tfans smaller than " << _min_tfan_tris << " tris";
    }
    if (_unroll_fans) {
      out << "; retesselate to tstrips.\n";
    } else {
      out << ".\n";
    }
  }
}



////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimTempl::sort_value
//       Access: Protected
//  Description: Returns a number for grouping properties.  This is
//               used as a helper function to the ordering operator,
//               above.  It simply collects all the booleans together
//               into a single number.
////////////////////////////////////////////////////////////////////
int BuilderProperties::
sort_value() const {
  return
    ((_mesh!=0) << 8) |
    ((_show_tstrips!=0) << 7) |
    ((_show_qsheets!=0) << 6) |
    ((_show_quads!=0) << 5) |
    ((_show_normals!=0) << 4) |
    ((_retesselate_coplanar!=0) << 3) |
    ((_unroll_fans!=0) << 2) |
    ((_subdivide_polys!=0) << 1) |
    ((_consider_fans!=0) << 0);
}
