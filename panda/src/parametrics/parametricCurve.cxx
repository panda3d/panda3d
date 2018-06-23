/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parametricCurve.cxx
 * @author drose
 * @date 2001-03-04
 */

#include "parametricCurve.h"
#include "config_parametrics.h"
#include "hermiteCurve.h"
#include "nurbsCurve.h"

#include "datagram.h"
#include "datagramIterator.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "omniBoundingVolume.h"

static const PN_stdfloat tolerance_divisor = 100000.0f;

TypeHandle ParametricCurve::_type_handle;


/**
 * This is a virtual base class.  Don't try to construct one from Scheme.
 */
ParametricCurve::
ParametricCurve() : PandaNode("curve") {
  _curve_type = PCT_NONE;
  _num_dimensions = 3;
}

/**
 *
 */
ParametricCurve::
~ParametricCurve() {
  // Our drawer list must be empty by the time we destruct, since our drawers
  // all maintain reference-counting pointers to us!  If this is not so, we
  // have lost a reference count somewhere, or we have gotten confused about
  // which drawers we're registered to.
  nassertv(_drawers.empty());
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * PandaNode by duplicating instances, false otherwise (for instance, a Camera
 * cannot be safely flattened, because the Camera pointer itself is
 * meaningful).
 */
bool ParametricCurve::
safe_to_flatten() const {
  return false;
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * PandaNode by calling the xform() method, false otherwise.  For instance,
 * it's usually a bad idea to attempt to xform a Character.
 */
bool ParametricCurve::
safe_to_transform() const {
  return false;
}

/**
 * Returns true if the curve is defined.  This base class function always
 * returns true; derived classes might override this to sometimes return
 * false.
 */
bool ParametricCurve::
is_valid() const {
  return true;
}


/**
 * Returns the upper bound of t for the entire curve.  The curve is defined in
 * the range 0.0f <= t <= get_max_t().  This base class function always
 * returns 1.0f; derived classes might override this to return something else.
 */
PN_stdfloat ParametricCurve::
get_max_t() const {
  return 1.0f;
}


/**
 * Sets the flag indicating the use to which the curve is intended to be put.
 * This flag is optional and only serves to provide a hint to the egg reader
 * and writer code; it has no effect on the curve's behavior.
 *
 * Setting the curve type also sets the num_dimensions to 3 or 1 according to
 * the type.
 *
 * THis flag may have one of the values PCT_XYZ, PCT_HPR, or PCT_T.
 */
void ParametricCurve::
set_curve_type(int type) {
  _curve_type = type;
  switch (_curve_type) {
  case PCT_XYZ:
  case PCT_HPR:
  case PCT_NONE:
    _num_dimensions = 3;
    break;

  case PCT_T:
    _num_dimensions = 1;
    break;

  default:
    assert(0);
  }
}

/**
 * Returns the flag indicating the use to which the curve is intended to be
 * put.
 */
int ParametricCurve::
get_curve_type() const {
  return _curve_type;
}

/**
 * Specifies the number of significant dimensions in the curve's vertices.
 * This should be one of 1, 2, or 3. Normally, XYZ and HPR curves have three
 * dimensions; time curves should always have one dimension.  This only serves
 * as a hint to the mopath editor, and also controls how the curve is written
 * out.
 */
void ParametricCurve::
set_num_dimensions(int num) {
  _num_dimensions = num;
}

/**
 * Returns the number of significant dimensions in the curve's vertices, as
 * set by a previous call to set_num_dimensions().  This is only a hint as to
 * how the curve is intended to be used; the actual number of dimensions of
 * any curve is always three.
 */
int ParametricCurve::
get_num_dimensions() const {
  return _num_dimensions;
}


/**
 * Approximates the length of the entire curve to within a few decimal places.
 */
PN_stdfloat ParametricCurve::
calc_length() const {
  return calc_length(0.0f, get_max_t());
}

/**
 * Approximates the length of the curve segment from parametric time 'from' to
 * time 'to'.
 */
PN_stdfloat ParametricCurve::
calc_length(PN_stdfloat from, PN_stdfloat to) const {
  PN_stdfloat t1, t2;
  LPoint3 p1, p2;

  // Normally we expect from < to.  If they came in backwards, reverse them.
  PN_stdfloat to_minus_from = to - from;

  if (to_minus_from < 0.0f) {
    PN_stdfloat temp = to;
    to = from;
    from = temp;
    to_minus_from=-to_minus_from;
  }

  // Start with a segment for each unit of t.
  int num_segs = (int)(to_minus_from) + 1;
  t2 = from;
  get_point(t2, p2);
  PN_stdfloat net = 0.0f;

  for (int i = 1; i <= num_segs; i++) {
    t1 = t2;
    p1 = p2;

    t2 = (to - from) * (PN_stdfloat)i / (PN_stdfloat)num_segs + from;
    get_point(t2, p2);

    net += r_calc_length(t1, t2, p1, p2, (p1 - p2).length());
  }
  return net;
}

/**
 * Returns the parametric value corresponding to the indicated distance along
 * the curve from the starting parametric value.
 *
 * This is the inverse of calc_length(): rather than determining the length
 * along the curve between two parametric points, it determines the position
 * in parametric time of a point n units along the curve.
 *
 * The search distance must not be negative.
 */
PN_stdfloat ParametricCurve::
find_length(PN_stdfloat start_t, PN_stdfloat length_offset) const {
  nassertr(length_offset >= 0.0f, start_t);
  nassertr(start_t >= 0.0f && start_t <= get_max_t(), start_t);

  PN_stdfloat t1, t2;
  LPoint3 p1, p2;

  // Start with a segment for each unit of t.
  PN_stdfloat max_t = get_max_t();
  int num_segs = (int)cfloor(max_t - start_t + 1);
  t2 = start_t;
  get_point(t2, p2);
  PN_stdfloat net = 0.0f;

  for (int i = 1; i <= num_segs; i++) {
    assert(net <= length_offset);

    t1 = t2;
    p1 = p2;

    t2 = start_t + (((max_t - start_t) * (PN_stdfloat)i) / (PN_stdfloat)num_segs);
    get_point(t2, p2);

    PN_stdfloat seglength = (p1 - p2).length();
    PN_stdfloat result;

    if (r_find_length(length_offset - net, result,
                      t1, t2, p1, p2, seglength)) {
      // Found it!
      return result;
    }

    net += seglength;
  }

  // Not on the curve?  Huh.
  return max_t;
}

/**
 * Recomputes the curve such that it passes through the point (px, py, pz) at
 * time t, but keeps the same tangent value at that point.
 */
bool ParametricCurve::
adjust_point(PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat) {
  return false;
}

/**
 * Recomputes the curve such that it has the tangent (tx, ty, tz) at time t,
 * but keeps the same position at the point.
 */
bool ParametricCurve::
adjust_tangent(PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat) {
  return false;
}

/**
 * Recomputes the curve such that it passes through the point (px, py, pz)
 * with the tangent (tx, ty, tz).
 */
bool ParametricCurve::
adjust_pt(PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat, PN_stdfloat) {
  return false;
}

/**
 * Recalculates the curve, if necessary.  Returns true if the resulting curve
 * is valid, false otherwise.
 */
bool ParametricCurve::
recompute() {
  return is_valid();
}

/**
 * Regenerates this curve as one long curve: the first curve connected end-to-
 * end with the second one.  Either a or b may be the same as 'this'.
 *
 * Returns true if successful, false on failure or if the curve type does not
 * support stitching.
 */
bool ParametricCurve::
stitch(const ParametricCurve *, const ParametricCurve *) {
  parametrics_cat.error()
    << get_type() << " does not support stitching.\n";
  return false;
}


/**
 * Writes an egg description of the nurbs curve to the specified output file.
 * Returns true if the file is successfully written.
 */
bool ParametricCurve::
write_egg(Filename filename, CoordinateSystem cs) {
  pofstream out;
  filename.set_text();

  if (!filename.open_write(out)) {
    parametrics_cat.error()
      << "Unable to write to " << filename << "\n";
    return false;
  }
  return write_egg(out, filename, cs);
}

/**
 * Writes an egg description of the nurbs curve to the specified output
 * stream.  Returns true if the file is successfully written.
 */
bool ParametricCurve::
write_egg(std::ostream &out, const Filename &filename, CoordinateSystem cs) {
  std::string curve_type;
  switch (get_curve_type()) {
  case PCT_XYZ:
    curve_type = "xyz";
    break;

  case PCT_HPR:
    curve_type = "hpr";
    break;

  case PCT_T:
    curve_type = "t";
    break;
  }

  if (!has_name()) {
    // If we don't have a name, come up with one.
    std::string name = filename.get_basename_wo_extension();

    if (!curve_type.empty()) {
      name += "_";
      name += curve_type;
    }

    set_name(name);
  }

  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (cs != CS_invalid) {
    out << "<CoordinateSystem> { ";
    switch (cs) {
    case CS_zup_right:
      out << "Z-Up";
      break;

    case CS_yup_right:
      out << "Y-Up";
      break;

    case CS_zup_left:
      out << "Z-Up-Left";
      break;

    case CS_yup_left:
      out << "Y-Up-Left";
      break;

    default:
      break;
    }
    out << " }\n\n";
  }


  if (!format_egg(out, get_name(), curve_type, 0)) {
    return false;
  }

  if (out) {
    return true;
  } else {
    return false;
  }
}



/**
 * Fills up the indicated vector with a list of BezierSeg structs that
 * describe the curve.  This assumes the curve is a PiecewiseCurve of
 * CubicCurvesegs.  Returns true if successful, false otherwise.
 */
bool ParametricCurve::
get_bezier_segs(ParametricCurve::BezierSegs &) const {
  return false;
}

/**
 * Fills the BezierSeg structure with a description of the curve segment as a
 * Bezier, if possible, but does not change the _t member of the structure.
 * Returns true if successful, false otherwise.
 */
bool ParametricCurve::
get_bezier_seg(ParametricCurve::BezierSeg &) const {
  return false;
}

/**
 * Returns a pointer to the object as a NurbsCurveInterface object if it
 * happens to be a NURBS-style curve; otherwise, returns NULL.
 */
NurbsCurveInterface *ParametricCurve::
get_nurbs_interface() {
  return nullptr;
}

/**
 * Stores an equivalent curve representation in the indicated Hermite curve,
 * if possible.  Returns true if successful, false otherwise.
 */
bool ParametricCurve::
convert_to_hermite(HermiteCurve *hc) const {
  BezierSegs bz_segs;
  if (!get_bezier_segs(bz_segs)) {
    return false;
  }

  hc->set_curve_type(_curve_type);

  // Now convert the Bezier segments to a Hermite.  Normally, the Beziers will
  // match up head-to-tail, but if they don't, that's a cut.
  hc->remove_all_cvs();

  int i, n;
  if (!bz_segs.empty()) {
    PN_stdfloat scale_in = 0.0f;
    PN_stdfloat scale_out = bz_segs[0]._t;
    n = hc->append_cv(HC_SMOOTH, bz_segs[0]._v[0]);
    hc->set_cv_out(n, 3.0f * (bz_segs[0]._v[1] - bz_segs[0]._v[0]) / scale_out);

    for (i = 0; i < (int)bz_segs.size()-1; i++) {
      scale_in = scale_out;
      scale_out = bz_segs[i+1]._t - bz_segs[i]._t;

      if (!bz_segs[i]._v[3].almost_equal(bz_segs[i+1]._v[0], 0.0001f)) {
        // Oops, we have a cut.
        hc->set_cv_type(n, HC_CUT);
      }

      n = hc->append_cv(HC_FREE, bz_segs[i+1]._v[0]);
      hc->set_cv_in(n, 3.0f * (bz_segs[i]._v[3] - bz_segs[i]._v[2]) / scale_in);
      hc->set_cv_tstart(n, bz_segs[i]._t);

      hc->set_cv_out(n, 3.0f * (bz_segs[i+1]._v[1] - bz_segs[i+1]._v[0]) / scale_out);
    }

    // Now the last CV.
    scale_in = scale_out;
    i = bz_segs.size()-1;
    n = hc->append_cv(HC_SMOOTH, bz_segs[i]._v[3]);
    hc->set_cv_in(n, 3.0f * (bz_segs[i]._v[3] - bz_segs[i]._v[2]) / scale_in);
    hc->set_cv_tstart(n, bz_segs[i]._t);
  }

  // Finally, go through and figure out which CV's are smooth or G1.
  int num_cvs = hc->get_num_cvs();
  for (n = 1; n < num_cvs-1; n++) {
    if (hc->get_cv_type(n)!=HC_CUT) {
      LVector3 in = hc->get_cv_in(n);
      LVector3 out = hc->get_cv_out(n);

      if (in.almost_equal(out, 0.0001f)) {
        hc->set_cv_type(n, HC_SMOOTH);
      } else {
        in.normalize();
        out.normalize();
        if (in.almost_equal(out, 0.0001f)) {
          hc->set_cv_type(n, HC_G1);
        }
      }
    }
  }
  return true;
}

/**
 * Stores in the indicated NurbsCurve a NURBS representation of an equivalent
 * curve.  Returns true if successful, false otherwise.
 */
bool ParametricCurve::
convert_to_nurbs(ParametricCurve *nc) const {
  NurbsCurveInterface *nurbs = nc->get_nurbs_interface();
  nassertr(nurbs != nullptr, false);

  BezierSegs bz_segs;
  if (!get_bezier_segs(bz_segs)) {
    return false;
  }

  nc->set_curve_type(_curve_type);

  nurbs->remove_all_cvs();
  nurbs->set_order(4);
  if (!bz_segs.empty()) {
    int i;
    for (i = 0; i < (int)bz_segs.size(); i++) {
      nurbs->append_cv(bz_segs[i]._v[0]);
      nurbs->append_cv(bz_segs[i]._v[1]);
      nurbs->append_cv(bz_segs[i]._v[2]);
      if (i == (int)bz_segs.size()-1 ||
          !bz_segs[i]._v[3].almost_equal(bz_segs[i+1]._v[0], 0.0001f)) {
        nurbs->append_cv(bz_segs[i]._v[3]);
      }
    }

    PN_stdfloat t;
    int ki = 4;
    nurbs->set_knot(0, 0.0f);
    nurbs->set_knot(1, 0.0f);
    nurbs->set_knot(2, 0.0f);
    nurbs->set_knot(3, 0.0f);

    for (i = 0; i < (int)bz_segs.size(); i++) {
      t = bz_segs[i]._t;

      nurbs->set_knot(ki, t);
      nurbs->set_knot(ki+1, t);
      nurbs->set_knot(ki+2, t);
      ki += 3;
      if (i == ((int)bz_segs.size())-1 ||
          !bz_segs[i]._v[3].almost_equal(bz_segs[i+1]._v[0], 0.0001f)) {
        nurbs->set_knot(ki, t);
        ki++;
      }
    }
  }

  return nc->recompute();
}


/**
 * Registers a Drawer with this curve that will automatically be updated
 * whenever the curve is modified, so that the visible representation of the
 * curve is kept up to date.  This is called automatically by the
 * ParametricCurveDrawer.
 *
 * Any number of Drawers may be registered with a particular curve.
 */
void ParametricCurve::
register_drawer(ParametricCurveDrawer *drawer) {
  _drawers.push_back(drawer);
}

/**
 * Removes a previously registered drawer from the list of automatically-
 * refreshed drawers.  This is called automatically by the
 * ParametricCurveDrawer.
 */
void ParametricCurve::
unregister_drawer(ParametricCurveDrawer *drawer) {
  _drawers.remove(drawer);
}




/**
 * Called from a base class to mark a section of the curve that has been
 * modified and must be redrawn or recomputed in some way.
 */
void ParametricCurve::
invalidate(PN_stdfloat, PN_stdfloat) {
  invalidate_all();
}

/**
 * Called from a base class to indicate that the curve has changed in some
 * substantial way and must be entirely redrawn.
 */
void ParametricCurve::
invalidate_all() {
  /*
  DrawerList::iterator n;
  for (n = _drawers.begin();
       n != _drawers.end();
       ++n) {
    (*n)->redraw();
  }
  */
}

/**
 * Formats the curve as an egg structure to write to the indicated stream.
 * Returns true on success, false on failure.
 */
bool ParametricCurve::
format_egg(std::ostream &, const std::string &, const std::string &, int) const {
  return false;
}


/**
 * The recursive implementation of calc_length.  This function calculates the
 * length of a segment of the curve between points t1 and t2, which presumably
 * evaluate to the endpoints p1 and p2, and the segment has the length
 * seglength.
 */
PN_stdfloat ParametricCurve::
r_calc_length(PN_stdfloat t1, PN_stdfloat t2, const LPoint3 &p1, const LPoint3 &p2,
              PN_stdfloat seglength) const {
  static const PN_stdfloat length_tolerance = 0.0000001f;
  static const PN_stdfloat t_tolerance = 0.000001f;

  if (t2 - t1 < t_tolerance) {
    // Stop recursing--we've just walked off the limit for representing
    // smaller values of t.
    return 0.0f;
  }

  PN_stdfloat tmid;
  LPoint3 pmid;
  PN_stdfloat left, right;

  // Calculate the point on the curve midway between the two endpoints.
  tmid = (t1+t2)*0.5f;
  get_point(tmid, pmid);

  // Did we increase the length of the segment measurably?
  left = (p1 - pmid).length();
  right = (pmid - p2).length();

  if ((left + right) - seglength < length_tolerance) {
    // No.  We're done.
    return seglength;
  } else {
    // Yes.  Keep going.
    return r_calc_length(t1, tmid, p1, pmid, left) +
      r_calc_length(tmid, t2, pmid, p2, right);
  }
}

/**
 * The recursive implementation of find_length.  This is similar to
 * r_calc_length, above.  target_length is the length along the curve past t1
 * that we hope to find.  If the indicated target_length falls within this
 * segment, returns true and sets found_t to the point along the segment.
 * Otherwise, updates seglength with the accurate calculated length of the
 * segment and returns false.
 */
bool ParametricCurve::
r_find_length(PN_stdfloat target_length, PN_stdfloat &found_t,
              PN_stdfloat t1, PN_stdfloat t2,
              const LPoint3 &p1, const LPoint3 &p2,
              PN_stdfloat &seglength) const {
  static const PN_stdfloat length_tolerance = 0.0000001f;
  static const PN_stdfloat t_tolerance = 0.000001f;

  if (target_length < t_tolerance) {
    // Stop recursing--we've just walked off the limit for representing
    // smaller values of t.
    found_t = t1;
    return true;

  }

  PN_stdfloat tmid;
  LPoint3 pmid;
  PN_stdfloat left, right;

  // Calculate the point on the curve midway between the two endpoints.
  tmid = (t1+t2)*0.5f;
  get_point(tmid, pmid);

  // Did we increase the length of the segment measurably?
  left = (p1 - pmid).length();
  right = (pmid - p2).length();

  if ((left + right) - seglength < length_tolerance) {
    // No.  Curve is relatively straight over this interval.
    return find_t_linear(target_length, found_t, t1, t2, p1, p2);
    /*
    if (target_length <= seglength) {
      // Compute t value that corresponds to target_length Maybe the point is
      // in the left half of the segment?
      if (r_find_t(target_length, found_t, t1, tmid, p1, pmid)) {
        return true;
      }
      // Maybe it's on the right half?
      if (r_find_t(target_length - left, found_t, tmid, t2, pmid, p2)) {
        return true;
      }
    }
    return false;
    */
  } else {
    // Yes.  Keep going.

    // Maybe the point is in the left half of the segment?
    if (r_find_length(target_length, found_t, t1, tmid, p1, pmid, left)) {
      return true;
    }

    // Maybe it's on the right half?
    if (r_find_length(target_length - left, found_t, tmid, t2, pmid, p2, right)) {
      return true;
    }

    // Neither.  Keep going.
    seglength = left + right;
    return false;
  }
}



/**
 * computes the t value in the parametric domain of a target point along a
 * straight section of a curve.  This is similar to r_calc_length, above.
 * target_length is the length along the curve past t1 that we hope to find.
 * If the indicated target_length falls within this segment, returns true and
 * sets found_t to the point along the segment.
 */
bool ParametricCurve::
r_find_t(PN_stdfloat target_length, PN_stdfloat &found_t,
         PN_stdfloat t1, PN_stdfloat t2,
         const LPoint3 &p1, const LPoint3 &p2) const {
  static const PN_stdfloat length_tolerance = 0.0001f;
  static const PN_stdfloat t_tolerance = 0.0001f;

  if (parametrics_cat.is_spam()) {
    parametrics_cat.spam()
      << "target_length " << target_length << " t1 " << t1 << " t2 " << t2 << "\n";
  }

  // Is the target point close to the near endpoint
  if (target_length < length_tolerance) {
    found_t = t1;
    return true;
  }

  // No, compute distance between two endpoints
  PN_stdfloat point_dist;
  point_dist = (p2 - p1).length();

  // Is the target point past the far endpoint?
  if (point_dist < target_length) {
    return false;
  }

  // Is the target point close to far endpoint?
  if ( (point_dist - target_length ) < length_tolerance ) {
    found_t = t2;
    return true;
  }

  // are we running out of parametric precision?
  if ((t2 - t1) < t_tolerance) {
    found_t = t1;
    return true;
  }

  // No, subdivide and continue
  PN_stdfloat tmid;
  LPoint3 pmid;
  PN_stdfloat left;

  // Calculate the point on the curve midway between the two endpoints.
  tmid = (t1+t2)*0.5f;
  get_point(tmid, pmid);

  // Maybe the point is in the left half of the segment?
  if (r_find_t(target_length, found_t, t1, tmid, p1, pmid)) {
    return true;
  }
  // Nope, must be in the right half
  left = (p1 - pmid).length();
  if (r_find_t(target_length - left, found_t, tmid, t2, pmid, p2)) {
    return true;
  }

  // not found in either half, keep looking
  return false;
}


/**
 * non-recursive version of r_find_t (see above)
 */
bool ParametricCurve::
find_t_linear(PN_stdfloat target_length, PN_stdfloat &found_t,
              PN_stdfloat t1, PN_stdfloat t2,
              const LPoint3 &p1, const LPoint3 &p2) const {
  const PN_stdfloat length_tolerance = (p1-p2).length()/tolerance_divisor;
  const PN_stdfloat t_tolerance = (t1+t2)/tolerance_divisor;

  if (parametrics_cat.is_spam()) {
    parametrics_cat.spam()
      << "target_length " << target_length << " t1 " << t1 << " t2 " << t2 << "\n";
  }

  // first, check to make sure this segment contains the point we're looking
  // for
  if (target_length > (p1 - p2).length()) {
    // segment is too short
    return false;
  }

  PN_stdfloat tleft = t1;
  PN_stdfloat tright = t2;
  PN_stdfloat tmid;
  LPoint3 pmid;
  PN_stdfloat len;

  while (1) {
    tmid = (tleft + tright) * 0.5f;
    get_point(tmid, pmid);
    len = (pmid - p1).length();

    /*
    if (parametrics_cat.is_spam()) {
      parametrics_cat.spam()
        << "tleft " << tleft << " tright " << tright <<
        " tmid " << tmid << " len " << len << endl;
    }
    */

    // is our midpoint at the right distance?
    if (fabs(len - target_length) < length_tolerance) {
      found_t = tmid;
      return true;
    }

    /*
    if (parametrics_cat.is_spam()) {
      parametrics_cat.spam()
        << "tright-tleft " << tright-tleft << " t_tolerance " << t_tolerance << endl;
    }
    */

    // are we out of parametric precision?
    if ((tright - tleft) < t_tolerance) {
      // unfortunately, we can't get any closer in parametric space
      found_t = tmid;
      return true;
    }

    // should we look closer or farther?
    if (len > target_length) {
      // look closer
      tright = tmid;
    } else {
      // look farther
      tleft = tmid;
    }
  }
}


/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void ParametricCurve::
write_datagram(BamWriter *manager, Datagram &me) {
  PandaNode::write_datagram(manager, me);

  me.add_int8(_curve_type);
  me.add_int8(_num_dimensions);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void ParametricCurve::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  _curve_type = scan.get_int8();
  _num_dimensions = scan.get_int8();
}
