/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file boundingHexahedron.cxx
 * @author drose
 * @date 1999-10-03
 */

#include "boundingHexahedron.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "boundingPlane.h"
#include "config_mathutil.h"

#include <math.h>
#include <algorithm>

using std::max;
using std::min;

TypeHandle BoundingHexahedron::_type_handle;

/**
 *
 */
BoundingHexahedron::
BoundingHexahedron(const LFrustum &frustum, bool is_ortho,
                   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  PN_stdfloat fs = 1.0f;
  if (!is_ortho) {
    fs = frustum._ffar / frustum._fnear;
  }

  // We build the points based on a Z-up right-handed frustum.  If the
  // requested coordinate system is otherwise, we'll convert it in a second
  // pass.
  _points[0].set(frustum._l * fs, frustum._ffar, frustum._b * fs);
  _points[1].set(frustum._r * fs, frustum._ffar, frustum._b * fs);
  _points[2].set(frustum._r * fs, frustum._ffar, frustum._t * fs);
  _points[3].set(frustum._l * fs, frustum._ffar, frustum._t * fs);
  _points[4].set(frustum._l, frustum._fnear, frustum._b);
  _points[5].set(frustum._r, frustum._fnear, frustum._b);
  _points[6].set(frustum._r, frustum._fnear, frustum._t);
  _points[7].set(frustum._l, frustum._fnear, frustum._t);

  _flags = 0;

  // Now fix the coordinate system, if necessary.
  if (cs == CS_zup_right) {
    set_centroid();
    set_planes();
  } else {
    xform(LMatrix4::convert_mat(CS_zup_right, cs));
  }
}

/**
 *
 */
BoundingHexahedron::
BoundingHexahedron(const LPoint3 &fll, const LPoint3 &flr,
                   const LPoint3 &fur, const LPoint3 &ful,
                   const LPoint3 &nll, const LPoint3 &nlr,
                   const LPoint3 &nur, const LPoint3 &nul) {
  _points[0] = fll;
  _points[1] = flr;
  _points[2] = fur;
  _points[3] = ful;
  _points[4] = nll;
  _points[5] = nlr;
  _points[6] = nur;
  _points[7] = nul;

  _flags = 0;
  set_centroid();
  set_planes();
}

/**
 *
 */
BoundingVolume *BoundingHexahedron::
make_copy() const {
  return new BoundingHexahedron(*this);
}

/**
 *
 */
LPoint3 BoundingHexahedron::
get_min() const {
  nassertr(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3(0.0f, 0.0f, 0.0f));
  int i;
  LPoint3 m = _points[0];
  for (i = 1; i < num_points; i++) {
    m.set(min(m[0], _points[i][0]),
          min(m[1], _points[i][1]),
          min(m[2], _points[i][2]));
  }
  return m;
}

/**
 *
 */
LPoint3 BoundingHexahedron::
get_max() const {
  nassertr(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3(0.0f, 0.0f, 0.0f));
  int i;
  LPoint3 m = _points[0];
  for (i = 1; i < num_points; i++) {
    m.set(max(m[0], _points[i][0]),
          max(m[1], _points[i][1]),
          max(m[2], _points[i][2]));
  }
  return m;
}

/**
 *
 */
LPoint3 BoundingHexahedron::
get_approx_center() const {
  nassertr(!is_empty(), LPoint3(0.0f, 0.0f, 0.0f));
  nassertr(!is_infinite(), LPoint3(0.0f, 0.0f, 0.0f));
  return _centroid;
}

/**
 *
 */
void BoundingHexahedron::
xform(const LMatrix4 &mat) {
  if (!is_empty() && !is_infinite()) {
    for (int i = 0; i < num_points; i++) {
      _points[i] = _points[i] * mat;
    }
    set_centroid();
    set_planes();
  }
}

/**
 *
 */
void BoundingHexahedron::
output(std::ostream &out) const {
  if (is_empty()) {
    out << "bhexahedron, empty";
  } else if (is_infinite()) {
    out << "bhexahedron, infinite";
  } else {
    out << "bhexahedron, min " << get_min() << " max " << get_max();
  }
}

/**
 *
 */
void BoundingHexahedron::
write(std::ostream &out, int indent_level) const {
  if (is_empty()) {
    indent(out, indent_level) << "bhexahedron, empty\n";
  } else if (is_infinite()) {
    out << "bhexahedron, infinite\n";
  } else {
    indent(out, indent_level)
      << "bhexahedron, min " << get_min() << " max " << get_max() << ":\n";
    int i;
    for (i = 0; i < num_points; i++) {
      indent(out, indent_level + 2) << _points[i] << "\n";
    }
    indent(out, indent_level + 2) << "centroid is " << _centroid << "\n";
  }
}

/**
 * Virtual downcast method.  Returns this object as a pointer of the indicated
 * type, if it is in fact that type.  Returns NULL if it is not that type.
 */
const BoundingHexahedron *BoundingHexahedron::
as_bounding_hexahedron() const {
  return this;
}

/**
 *
 */
bool BoundingHexahedron::
extend_other(BoundingVolume *other) const {
  return other->extend_by_hexahedron(this);
}

/**
 *
 */
bool BoundingHexahedron::
around_other(BoundingVolume *other,
             const BoundingVolume **first,
             const BoundingVolume **last) const {
  return other->around_hexahedrons(first, last);
}

/**
 *
 */
int BoundingHexahedron::
contains_other(const BoundingVolume *other) const {
  return other->contains_hexahedron(this);
}

/**
 *
 */
int BoundingHexahedron::
contains_point(const LPoint3 &point) const {
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    // The hexahedron contains the point iff the point is behind all of the
    // planes.
    for (int i = 0; i < num_planes; i++) {
      const LPlane &p = _planes[i];
      if (p.dist_to_plane(point) > 0.0f) {
        return IF_no_intersection;
      }
    }
    return IF_possible | IF_some | IF_all;
  }
}

/**
 *
 */
int BoundingHexahedron::
contains_lineseg(const LPoint3 &a, const LPoint3 &b) const {
  if (is_empty()) {
    return IF_no_intersection;

  } else if (is_infinite()) {
    return IF_possible | IF_some | IF_all;

  } else {
    // The hexahedron does not contains the line segment if both points are in
    // front of any one plane.
    for (int i = 0; i < num_planes; i++) {
      const LPlane &p = _planes[i];
      if (p.dist_to_plane(a) > 0.0f ||
          p.dist_to_plane(b) > 0.0f) {
        return IF_no_intersection;
      }
    }

    // If there is no plane that both points are in front of, the hexahedron
    // may or may not contain the line segment.  For the moment, we won't
    // bother to check that more thoroughly, though.
    return IF_possible;
  }
}

/**
 *
 */
int BoundingHexahedron::
contains_sphere(const BoundingSphere *sphere) const {
  nassertr(!is_empty(), 0);

  // The hexahedron contains the sphere iff the sphere is at least partly
  // behind all of the planes.
  const LPoint3 &center = sphere->get_center();
  PN_stdfloat radius = sphere->get_radius();

  int result = IF_possible | IF_some | IF_all;

  for (int i = 0; i < num_planes; i++) {
    const LPlane &p = _planes[i];
    PN_stdfloat dist = p.dist_to_plane(center);

    if (dist > radius) {
      // The sphere is completely in front of this plane; it's thus completely
      // outside of the hexahedron.
      return IF_no_intersection;

    } else if (dist > -radius) {
      // The sphere is not completely behind this plane, but some of it is.
      result &= ~IF_all;
    }
  }

  return result;
}

/**
 *
 */
int BoundingHexahedron::
contains_box(const BoundingBox *box) const {
  nassertr(!is_empty(), 0);
  nassertr(!box->is_empty(), 0);

  // Put the box inside a sphere for the purpose of this test.
  const LPoint3 &min = box->get_minq();
  const LPoint3 &max = box->get_maxq();
  LPoint3 center = (min + max) * 0.5f;
  PN_stdfloat radius2 = (max - center).length_squared();

  int result = IF_possible | IF_some | IF_all;

  for (int i = 0; i < num_planes; i++) {
    const LPlane &p = _planes[i];
    PN_stdfloat dist = p.dist_to_plane(center);
    PN_stdfloat dist2 = dist * dist;

    if (dist2 <= radius2) {
      // The sphere is not completely behind this plane, but some of it is.

      // Look a little closer.
      bool all_in = true;
      bool all_out = true;
      for (int i = 0; i < 8 && (all_in || all_out) ; ++i) {
        if (p.dist_to_plane(box->get_point(i)) < 0.0f) {
          // This point is inside the plane.
          all_out = false;
        } else {
          // This point is outside the plane.
          all_in = false;
        }
      }

      if (all_out) {
        return IF_no_intersection;
      } else if (!all_in) {
        result &= ~IF_all;
      }

    } else if (dist >= 0.0f) {
      // The sphere is completely in front of this plane.
      return IF_no_intersection;
    }
  }

  return result;
}

/**
 *
 */
int BoundingHexahedron::
contains_plane(const BoundingPlane *plane) const {
  return plane->contains_hexahedron(this) & ~IF_all;
}

/**
 *
 */
int BoundingHexahedron::
contains_hexahedron(const BoundingHexahedron *hexahedron) const {
  nassertr(!is_empty(), 0);
  nassertr(!hexahedron->is_empty(), 0);

  // Put the hexahedron inside a sphere for the purposes of this test.
  LPoint3 min = hexahedron->get_min();
  LPoint3 max = hexahedron->get_max();
  LPoint3 center = (min + max) * 0.5f;
  PN_stdfloat radius2 = (max - center).length_squared();

  int result = IF_possible | IF_some | IF_all;

  for (int i = 0; i < num_planes; i++) {
    const LPlane &p = _planes[i];
    PN_stdfloat dist = p.dist_to_plane(center);
    PN_stdfloat dist2 = dist * dist;

    if (dist >= 0.0f && dist2 > radius2) {
      // The sphere is completely in front of this plane; it's thus completely
      // outside of the hexahedron.
      return IF_no_intersection;

    } else {/*if (dist < 0.0f && dist2 < radius2) {*/
      // The sphere is not completely behind this plane, but some of it is.

      // Look a little closer.
      unsigned points_out = 0;
      for (int i = 0; i < 8; ++i) {
        if (p.dist_to_plane(hexahedron->get_point(i)) > 0.0f) {
          // This point is outside the plane.
          ++points_out;
        }
      }

      if (points_out != 0) {
        if (points_out == 8) {
          return IF_no_intersection;
        }
        result &= ~IF_all;
      }
    }
  }

  return result;
}

/**
 *
 */
void BoundingHexahedron::
set_planes() {
  _planes[0] = LPlane(_points[0], _points[3], _points[2]);

  // Test to see if we have accidentally inverted our frustum by transforming
  // it with a -1 matrix.  We do this by ensuring that the centroid is in
  // front of all of the planes (actually, we only need to test the first
  // plane).
  if (_planes[0].dist_to_plane(_centroid) > 0) {
    // Oops!  We're flipped!  Rebuild the planes in the opposite direction.
    _planes[0] = LPlane(_points[0], _points[2], _points[3]);
    _planes[1] = LPlane(_points[0], _points[5], _points[1]);
    _planes[2] = LPlane(_points[1], _points[6], _points[2]);
    _planes[3] = LPlane(_points[2], _points[7], _points[3]);
    _planes[4] = LPlane(_points[3], _points[4], _points[0]);
    _planes[5] = LPlane(_points[4], _points[7], _points[6]);

  } else {
    // No, a perfectly sane universe.
    _planes[1] = LPlane(_points[0], _points[1], _points[5]);
    _planes[2] = LPlane(_points[1], _points[2], _points[6]);
    _planes[3] = LPlane(_points[2], _points[3], _points[7]);
    _planes[4] = LPlane(_points[3], _points[0], _points[4]);
    _planes[5] = LPlane(_points[4], _points[6], _points[7]);
  }

  // Still not entirely sure why some code keeps triggering these, but I'm
  // taking them out of the normal build for now.
#ifdef _DEBUG
  nassertv(_planes[0].dist_to_plane(_centroid) <= 0.001);
  nassertv(_planes[1].dist_to_plane(_centroid) <= 0.001);
  nassertv(_planes[2].dist_to_plane(_centroid) <= 0.001);
  nassertv(_planes[3].dist_to_plane(_centroid) <= 0.001);
  nassertv(_planes[4].dist_to_plane(_centroid) <= 0.001);
  nassertv(_planes[5].dist_to_plane(_centroid) <= 0.001);
#endif
}

/**
 *
 */
void BoundingHexahedron::
set_centroid() {
  LPoint3 net = _points[0];
  for (int i = 1; i < num_points; i++) {
    net += _points[i];
  }
  _centroid = net / (PN_stdfloat)num_points;
}
