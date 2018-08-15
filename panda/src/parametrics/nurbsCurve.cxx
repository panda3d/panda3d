/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsCurve.cxx
 * @author drose
 * @date 1998-02-27
 */

#include "nurbsCurve.h"
#include "config_parametrics.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "epvector.h"

TypeHandle NurbsCurve::_type_handle;

/**
 *
 */
NurbsCurve::
NurbsCurve() {
  _order = 4;
}

/**
 * Constructs a NURBS curve equivalent to the indicated (possibly non-NURBS)
 * curve.
 */
NurbsCurve::
NurbsCurve(const ParametricCurve &pc) {
  _order = 4;

  if (!pc.convert_to_nurbs(this)) {
    parametrics_cat->warning()
      << "Cannot make a NURBS from the indicated curve.\n";
  }
}

/**
 * Constructs a NURBS curve according to the indicated NURBS parameters.
 */
NurbsCurve::
NurbsCurve(int order, int num_cvs,
           const PN_stdfloat knots[], const LVecBase4 cvs[]) {
  _order = order;

  int i;
  _cvs.reserve(num_cvs);
  for (i = 0; i < num_cvs; i++) {
    append_cv(cvs[i]);
  }

  int num_knots = num_cvs + order;
  for (i = 0; i < num_knots; i++) {
    set_knot(i, knots[i]);
  }

  recompute();
}

/**
 *
 */
NurbsCurve::
~NurbsCurve() {
}

/**
 * Returns a newly-allocated PandaNode that is a shallow copy of this one.  It
 * will be a different pointer, but its internal data may or may not be shared
 * with that of the original PandaNode.  No children will be copied.
 */
PandaNode *NurbsCurve::
make_copy() const {
  return new NurbsCurve(*this);
}

/**
 * Changes the order of the curve.  Must be a value from 1 to 4.  Can only be
 * done when there are no cv's.
 */
void NurbsCurve::
set_order(int order) {
  nassertv(order >= 1 && order <= 4);
  nassertv(_cvs.empty());

  _order = order;
}

/**
 *
 */
int NurbsCurve::
get_order() const {
  return _order;
}

/**
 *
 */
int NurbsCurve::
get_num_cvs() const {
  return _cvs.size();
}

/**
 * Returns the number of knots on the curve.
 */
int NurbsCurve::
get_num_knots() const {
  return _cvs.size() + _order;
}



/**
 * Inserts a new CV into the middle of the curve at the indicated parametric
 * value.  This doesn't change the shape or timing of the curve; however, it
 * is irreversible: if the new CV is immediately removed, the curve will be
 * changed.  Returns true if successful, false otherwise.
 */
bool NurbsCurve::
insert_cv(PN_stdfloat t) {
  if (_cvs.empty()) {
    append_cv(0.0f, 0.0f, 0.0f);
    return true;
  }

  if (t <= 0) {
    t = 0.0f;
  }

  int k = find_cv(t);
  if (k < 0) {
    append_cv(_cvs.back()._p);
    return true;
  }

  // Now we are inserting a knot between k-1 and k.  We'll adjust the CV's
  // according to Bohm's rule.

  // First, get the new values of all the CV's that will change.  These are
  // the CV's in the range [k - (_order-1), k-1].

  LVecBase4 new_cvs[3];
  int i;
  for (i = 0; i < _order-1; i++) {
    int nk = i + k - (_order-1);
    PN_stdfloat ti = get_knot(nk);
    PN_stdfloat d = get_knot(nk + _order-1) - ti;
    if (d == 0.0f) {
      new_cvs[i] = _cvs[nk-1]._p;
    } else {
      PN_stdfloat a = (t - ti) / d;
      new_cvs[i] = (1.0f-a)*_cvs[nk-1]._p + a*_cvs[nk]._p;
    }
  }

  // Now insert the new CV
  _cvs.insert(_cvs.begin() + k-1, CV());

  // Set all the new position values
  for (i = 0; i < _order-1; i++) {
    int nk = i + k - (_order-1);
    _cvs[nk]._p = new_cvs[i];
  }

  // And set the new knot value.
  _cvs[k-1]._t = t;

  return true;
}

/**
 * Removes the indicated CV from the curve.  Returns true if the CV index was
 * valid, false otherwise.
 */
bool NurbsCurve::
remove_cv(int n) {
  if (n < 0 || n >= (int)_cvs.size()) {
    return false;
  }

  _cvs.erase(_cvs.begin() + n);
  return true;
}

/**
 * Removes all CV's from the curve.
 */
void NurbsCurve::
remove_all_cvs() {
  _cvs.erase(_cvs.begin(), _cvs.end());
}


/**
 * Repositions the indicated CV.  Returns true if successful, false otherwise.
 */
bool NurbsCurve::
set_cv(int n, const LVecBase4 &v) {
  nassertr(n >= 0 && n < get_num_cvs(), false);

  _cvs[n]._p = v;
  return true;
}

/**
 * Returns the position in homogeneous space of the indicated CV.
 */
LVecBase4 NurbsCurve::
get_cv(int n) const {
  nassertr(n >= 0 && n < get_num_cvs(), LVecBase4::zero());

  return _cvs[n]._p;
}


/**
 * Sets the value of the indicated knot.  There are get_num_cvs() + _order
 * knot values, but the first _order - 1 and the last 1 knot values cannot be
 * changed.  It is also an error to set a knot value outside the range of its
 * neighbors.
 */
bool NurbsCurve::
set_knot(int n, PN_stdfloat t) {
  nassertr(n >= 0 && n < get_num_knots(), false);

  if (n < _order || n-1 >= (int)_cvs.size()) {
    return false;
  }
  _cvs[n-1]._t = t;
  return true;
}

/**
 * Retrieves the value of the indicated knot.
 */
PN_stdfloat NurbsCurve::
get_knot(int n) const {
  if (n < _order || _cvs.empty()) {
    return 0.0f;
  } else if (n-1 >= (int)_cvs.size()) {
    return _cvs.back()._t;
  } else {
    return _cvs[n-1]._t;
  }
}


/**
 * Recalculates the curve basis according to the latest position of the CV's,
 * knots, etc.  Until this function is called, adjusting the NURBS parameters
 * will have no visible effect on the curve.  Returns true if the resulting
 * curve is valid, false otherwise.
 */
bool NurbsCurve::
recompute() {
  _segs.erase(_segs.begin(), _segs.end());

  PN_stdfloat knots[8];
  LVecBase4 cvs[4];

  if ((int)_cvs.size() > _order-1) {
    for (int cv = 0; cv < (int)_cvs.size()-(_order-1); cv++) {
      if (get_knot(cv+_order-1) < get_knot(cv+_order)) {
        // There are _order consecutive CV's that define each segment,
        // beginning at cv.  Collect the CV's and knot values that define this
        // segment.
        int c;
        for (c = 0; c < _order; c++) {
          cvs[c] = _cvs[c+cv]._p;
        }
        for (c = 0; c < _order+_order; c++) {
          knots[c] = get_knot(c+cv);
        }

        insert_curveseg(_segs.size(), new CubicCurveseg(_order, knots, cvs),
                        knots[_order] - knots[_order-1]);
      }
    }
  }

  return !_segs.empty();
}

/**
 * Rebuilds the current curve segment (as selected by the most recent call to
 * find_curve()) according to the specified properties (see
 * CubicCurveseg::compute_seg).  Returns true if possible, false if something
 * goes horribly wrong.
 */
bool NurbsCurve::
rebuild_curveseg(int rtype0, PN_stdfloat t0, const LVecBase4 &v0,
                 int rtype1, PN_stdfloat t1, const LVecBase4 &v1,
                 int rtype2, PN_stdfloat t2, const LVecBase4 &v2,
                 int rtype3, PN_stdfloat t3, const LVecBase4 &v3) {
  // Figure out which CV's contributed to this segment.
  int seg = 0;

  nassertr((int)_cvs.size() > _order-1, false);

  int cv = 0;
  for (cv = 0; cv < (int)_cvs.size()-(_order-1); cv++) {
    if (get_knot(cv+_order-1) < get_knot(cv+_order)) {
      if (seg == _last_ti) {
        break;
      }
      seg++;
    }
  }

  // Now copy the cvs and knots in question.
  LMatrix4 G;
  PN_stdfloat knots[8];

  int c;

  // We only need to build the geometry matrix if at least one of the
  // properties depends on the original value.
  if ((rtype0 | rtype1 | rtype2 | rtype3) & RT_KEEP_ORIG) {
    for (c = 0; c < 4; c++) {
      const LVecBase4 &s = (c < _order) ? _cvs[c+cv]._p : LVecBase4::zero();

      G.set_col(c, s);
    }
  }

  // But we always need the knot vector to determine the basis matrix.
  for (c = 0; c < _order+_order; c++) {
    knots[c] = get_knot(c+cv);
  }

  LMatrix4 B;
  compute_nurbs_basis(_order, knots, B);

  LMatrix4 Bi;
  Bi = invert(B);

  if (!CubicCurveseg::compute_seg(rtype0, t0, v0,
                                  rtype1, t1, v1,
                                  rtype2, t2, v2,
                                  rtype3, t3, v3,
                                  B, Bi, G)) {
    return false;
  }

  // Now extract the new CV's from the new G matrix, and restore them to the
  // curve.
  for (c = 0; c < _order; c++) {
    _cvs[c+cv]._p = G.get_col(c);
  }

  return true;
}

/**
 * Regenerates this curve as one long curve: the first curve connected end-to-
 * end with the second one.  Either a or b may be the same as 'this'.
 *
 * Returns true if successful, false on failure or if the curve type does not
 * support stitching.
 */
bool NurbsCurve::
stitch(const ParametricCurve *a, const ParametricCurve *b) {
  // First, make a copy of both of our curves.  This ensures they are of the
  // correct type, and also protects us in case one of them is the same as
  // 'this'.
  PT(NurbsCurve) na = new NurbsCurve(*a);
  PT(NurbsCurve) nb = new NurbsCurve(*b);

  if (na->get_num_cvs() == 0 || nb->get_num_cvs() == 0) {
    return false;
  }

  if (na->get_order() != nb->get_order()) {
    parametrics_cat->error()
      << "Cannot stitch NURBS curves of different orders!\n";
    return false;
  }

  // First, translate curve B to move its first CV to curve A's last CV.
  LVecBase3 point_offset =
    na->get_cv_point(na->get_num_cvs() - 1) - nb->get_cv_point(0);
  int num_b_cvs = nb->get_num_cvs();
  for (int i = 0; i < num_b_cvs; i++) {
    nb->set_cv_point(i, nb->get_cv_point(i) + point_offset);
  }

  // Now define a vector of all of A's CV's except the last one.
  _cvs = na->_cvs;
  if (!_cvs.empty()) {
    _cvs.pop_back();
  }

  PN_stdfloat t = na->get_max_t();

  // Now add all the new CV's.
  epvector<CV>::iterator ci;
  for (ci = nb->_cvs.begin(); ci != nb->_cvs.end(); ++ci) {
    CV new_cv = (*ci);
    new_cv._t += t;
    _cvs.push_back(new_cv);
  }

  recompute();
  return true;
}


/**
 * Returns a pointer to the object as a NurbsCurveInterface object if it
 * happens to be a NURBS-style curve; otherwise, returns NULL.
 */
NurbsCurveInterface *NurbsCurve::
get_nurbs_interface() {
  return this;
}

/**
 * Stores in the indicated NurbsCurve a NURBS representation of an equivalent
 * curve.  Returns true if successful, false otherwise.
 */
bool NurbsCurve::
convert_to_nurbs(ParametricCurve *nc) const {
  nc->set_curve_type(_curve_type);
  return NurbsCurveInterface::convert_to_nurbs(nc);
}

/**
 *
 */
void NurbsCurve::
write(std::ostream &out, int indent_level) const {
  NurbsCurveInterface::write(out, indent_level);
}

/**
 * Adds a new CV to the end of the curve.  Creates a new knot value by adding
 * 1 to the last knot value.  Returns the index of the new CV.
 */
int NurbsCurve::
append_cv_impl(const LVecBase4 &v) {
  _cvs.push_back(CV(v, get_knot(_cvs.size())+1.0f));
  return _cvs.size()-1;
}

/**
 * Formats the curve as an egg structure to write to the indicated stream.
 * Returns true on success, false on failure.
 */
bool NurbsCurve::
format_egg(std::ostream &out, const std::string &name, const std::string &curve_type,
           int indent_level) const {
  return NurbsCurveInterface::format_egg(out, name, curve_type, indent_level);
}

/**
 * Finds the first knot whose value is >= t, or -1 if t is beyond the end of
 * the curve.
 */
int NurbsCurve::
find_cv(PN_stdfloat t) {
  int i;
  for (i = _order-1; i < (int)_cvs.size(); i++) {
    if (_cvs[i]._t >= t) {
      return i+1;
    }
  }

  return -1;
}

/**
 * Initializes the factory for reading these things from Bam files.
 */
void NurbsCurve::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_NurbsCurve);
}

/**
 * Factory method to generate an object of this type.
 */
TypedWritable *NurbsCurve::
make_NurbsCurve(const FactoryParams &params) {
  NurbsCurve *me = new NurbsCurve;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void NurbsCurve::
write_datagram(BamWriter *manager, Datagram &me) {
  PiecewiseCurve::write_datagram(manager, me);

  me.add_int8(_order);

  me.add_uint32(_cvs.size());
  size_t i;
  for (i = 0; i < _cvs.size(); i++) {
    const CV &cv = _cvs[i];
    cv._p.write_datagram(me);
    me.add_float64(cv._t);
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void NurbsCurve::
fillin(DatagramIterator &scan, BamReader *manager) {
  PiecewiseCurve::fillin(scan, manager);

  _order = scan.get_int8();

  size_t num_cvs = scan.get_uint32();

  _cvs.reserve(num_cvs);
  size_t i;
  for (i = 0; i < num_cvs; i++) {
    CV cv;
    cv._p.read_datagram(scan);
    cv._t = scan.get_float64();
    _cvs.push_back(cv);
  }
}
