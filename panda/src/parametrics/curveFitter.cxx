// Filename: curveFitter.C
// Created by:  drose (17Sep98)
// 
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97  Walt Disney Imagineering, Inc.
// 
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
#include "pandabase.h"
#include "pointerTo.h"

#include "curveFitter.h"
#include "config_parametrics.h"
#include "curve.h"
#include "nurbsCurve.h"
#include "hermiteCurve.h"
#include <algorithm>

TypeHandle CurveFitter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::reset
//       Access: Public
//  Description: Removes all the data points previously added to the
//               CurveFitter, and initializes it for a new curve.
////////////////////////////////////////////////////////////////////
void CurveFitter::
reset() {
  _data.erase(_data.begin(), _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::add_point
//       Access: Public
//  Description: Adds a single sample point.
////////////////////////////////////////////////////////////////////
void CurveFitter::
add_point(double t, const LVector3f &point) {
  DataPoint dp;
  dp._t = t;
  dp._point = point;
  _data.push_back(dp);
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::sample
//       Access: Public
//  Description: Generates a series of data points by sampling the
//               given curve the indicated number of times.  If even
//               is true, the sampled t value is taken from the actual
//               curve length, as opposed to the parametric length.
////////////////////////////////////////////////////////////////////
void CurveFitter::
sample(ParametricCurve *curve, int count, bool even) {
  double max_t = curve->get_max_t();
  double t, last_t, d;
  DataPoint dp;

  last_t = 0.0;
  d = 0.0;
  int i;
  for (i = 0; i < count; i++) {
    t = max_t * (double)i / (double)(count-1);
    curve->get_point(t, dp._point);

    if (even) {
      d += curve->calc_length(last_t, t);
      dp._t = d;
    } else {
      dp._t = t;
    }

    _data.push_back(dp);
    last_t = t;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::generate_even
//       Access: Public
//  Description: Generates a set of data points whose x coordinate is
//               evenly distributed across the indicated distance and
//               parametric time.  Useful before a call to
//               compute_timewarp().
////////////////////////////////////////////////////////////////////
void CurveFitter::
generate_even(int count, double net_distance, double net_time) {
  double t, d;
  DataPoint dp;
  int i;
  for (i = 0; i < count; i++) {
    t = net_time * (double)i / (double)(count-1);
    d = net_distance * (double)i / (double)(count-1);
    
    dp._point.set(d, 0.0, 0.0);
    dp._t = t;
    _data.push_back(dp);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::wrap_hpr
//       Access: Public
//  Description: Assumes the data points collected represent a set of
//               HPR coordinates.  Resets each data point so that the
//               maximum delta between any two consecutive points is
//               180 degrees, which should prevent incorrect HPR
//               wrapping.
////////////////////////////////////////////////////////////////////
void CurveFitter::
wrap_hpr() {
  Data::iterator di;
  LVector3f last(0.0, 0.0, 0.0);
  LVector3f net(0.0, 0.0, 0.0);

  for (di = _data.begin(); di != _data.end(); ++di) {
    int i;
    for (i = 0; i < 3; i++) {
      (*di)._point[i] += net[i];
  
      while (((*di)._point[i] - last[i]) > 180.0) {
        (*di)._point[i] -= 360.0;
        net[i] -= 360.0;
      }
      
      while (((*di)._point[i] - last[i]) < -180.0) {
        (*di)._point[i] += 360.0;
        net[i] += 360.0;
      }
      
      last[i] = (*di)._point[i];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::compute_timewarp
//       Access: Public
//  Description: Assumes the data points already collected represent
//               the distance along a given curve at each unit of
//               parametric time (only the X coordinate is used).
//               Computes a new set of data points based on the
//               indicated curve that will serve as a timewarp to
//               produce this effect.
////////////////////////////////////////////////////////////////////
void CurveFitter::
compute_timewarp(const ParametricCurve *xyz) {
  Data::iterator di;
  double last_t = 0.0;
  double last_d = 0.0;
  double last_ratio = 1.0;

  for (di = _data.begin(); di != _data.end(); ++di) {
    double d = (*di)._point[0];
    double t = xyz->compute_t(last_t, d - last_d, 
                              last_t + last_ratio * (d - last_d),
                              0.001);
    (*di)._point.set(t, 0.0, 0.0);

    /*
    // Special HPR computation
    {
      LVector3f tangent;
      LMatrix4f mat;
      pfCoord c;
      static double last_h = 0.0;
      static double h_net = 0.0;

      xyz->get_tangent(t, tangent);
      look_at(mat, tangent, LVector3f(0.0, 0.0, 1.0));
      mat.getOrthoCoord(&c);
      cerr << "Replacing R " << c.hpr[2] << " with " << (*di)._point[1] << "\n";
      c.hpr[2] = (*di)._point[1];
      c.hpr[0] += h_net;

      // Check the wrap on the heading
      if ((c.hpr[0] - last_h) > 180.0) {
        c.hpr[0] -= 360.0;
        h_net -= 360.0;
      }

      if ((c.hpr[0] - last_h) < -180.0) {
        c.hpr[0] += 360.0;
        h_net += 360.0;
      }

      cerr << "H is " << c.hpr[0] << " h_net is " << h_net << "\n";
      last_h = c.hpr[0];
      
      (*di)._point = c.hpr;
      (*di)._t = t;
    }
    */

    if (d != last_d) {
      last_ratio = (t - last_t) / (d - last_d);
    }
    last_t = t;
    last_d = d;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::sort_points
//       Access: Public
//  Description: Sorts all the data points in order by parametric
//               time, in case they were added in an incorrect order.
////////////////////////////////////////////////////////////////////
void CurveFitter::
sort_points() {
  sort(_data.begin(), _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::desample
//       Access: Public
//  Description: Removes sample points in order to reduce the
//               complexity of a sampled curve.  Keeps one out of
//               every factor samples.  Also keeps the first and the
//               last samples.
////////////////////////////////////////////////////////////////////
void CurveFitter::
desample(double factor) {
  int in, out;
  double count = factor;

  out = 0;
  for (in = 0; in < _data.size()-1; in++) {
    if (count >= factor) {
      _data[out] = _data[in];
      out++;
      count -= factor;
    }
    count += 1.0;
  }

  _data[out] = _data.back();
  out++;

  _data.erase(_data.begin() + out, _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::compute_tangents
//       Access: Public
//  Description: Once a set of points has been built, and prior to
//               calling MakeHermite() or MakeNurbs(),
//               ComputeTangents() must be called to set up the
//               tangents correctly (unless the tangents were defined
//               as the points were added).
////////////////////////////////////////////////////////////////////
void CurveFitter::
compute_tangents(double scale) {
  // If the head and tail points match up, close the curve.
  bool closed =
    (_data.front()._point.almost_equal(_data.back()._point, 0.001));

  int i;
  int len = _data.size();

  // First, get all the points in the middle, excluding endpoints.
  // These are handled the same whether we are closing the curve or
  // not.
  for (i = 1; i < len-1; i++) {
    _data[i]._tangent = 
      (_data[i+1]._point - _data[i-1]._point) * scale /
      (_data[i+1]._t - _data[i-1]._t);
  }

  // Now handle the endpoints.
  if (closed) {
    _data[0]._tangent = _data[len-1]._tangent =
      (_data[1]._point - _data[len-2]._point) * scale /
      ((_data[1]._t - _data[0]._t) + (_data[len-1]._t - _data[len-2]._t));

  } else {
    _data[0]._tangent =
      (_data[1]._point - _data[0]._point) * scale /
      ((_data[1]._t - _data[0]._t) * 2.0);
    _data[len-1]._tangent =
      (_data[len-1]._point - _data[len-2]._point) * scale /
      ((_data[len-1]._t - _data[len-2]._t) * 2.0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::make_hermite
//       Access: Public
//  Description: Converts the current set of data points into a
//               Hermite curve.
////////////////////////////////////////////////////////////////////
HermiteCurve *CurveFitter::
make_hermite() const {
  HermiteCurve *hc = new HermiteCurve;

  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    int n = hc->insert_cv((*di)._t);
    hc->set_cv_type(n, HC_SMOOTH);
    hc->set_cv_point(n, (*di)._point);
    hc->set_cv_in(n, (*di)._tangent);
    hc->set_cv_out(n, (*di)._tangent);
  }

  return hc;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::make_nurbs
//       Access: Public
//  Description: Converts the current set of data points into a
//               NURBS curve.  This gives a smoother curve than
//               produced by MakeHermite().
////////////////////////////////////////////////////////////////////
NurbsCurve *CurveFitter::
make_nurbs() const {
  if (_data.size() < 2) {
    return NULL;
  }

#if 0
  NurbsCurve *nc = new NurbsCurve;
  nc->set_order(4);

  // First, we need four CV's to get started.
  nc->append_cv(LVector3f(0.0, 0.0, 0.0));
  nc->append_cv(LVector3f(0.0, 0.0, 0.0));
  nc->append_cv(LVector3f(0.0, 0.0, 0.0));
  nc->append_cv(LVector3f(0.0, 0.0, 0.0));
  nc->set_knot(4, _data[1]._t);

  nc->recompute();
  LVector3f junk;
  nc->get_point(nc->get_max_t(), junk);  // Reference the last segment.
  const LVector3f &p0 = _data[0]._point;
  LVector3f t0 = _data[0]._tangent * 2.0;
  LVector3f t1 = _data[1]._tangent * 2.0;
  const LVector3f &p1 = _data[1]._point;

  nc->rebuild_curveseg(RT_POINT, 0.0, pfVec4(p0[0], p0[1], p0[2], 1.0),
                       RT_TANGENT, 0.0, pfVec4(t0[0], t0[1], t0[2], 0.0),
                       RT_TANGENT, 1.0, pfVec4(t1[0], t1[1], t1[2], 0.0),
                       RT_POINT, 1.0, pfVec4(p1[0], p1[1], p1[2], 1.0));

  int i;
  for (i = 2; i < _data.size(); i++) {
    cerr << "Adding point " << i << "\n";
    nc->append_cv(LVector3f(0.0, 0.0, 0.0));
    nc->set_knot(i + 3, _data[i]._t);
    nc->recompute();
    nc->get_point(nc->get_max_t(), junk);

    /*
    const LVector3f &p0 = _data[i-1]._point;
    const LVector3f &t0 = _data[i-1]._tangent;
    const LVector3f &p1 = _data[i]._point;
    const LVector3f &t1 = _data[i]._tangent;
    
    nc->rebuild_curveseg(RT_POINT, 0.0, pfVec4(p0[0], p0[1], p0[2], 1.0),
                         RT_TANGENT, 0.0, pfVec4(t0[0], t0[1], t0[2], 0.0),
                         RT_TANGENT, 1.0, pfVec4(t1[0], t1[1], t1[2], 0.0),
                         RT_POINT, 1.0, pfVec4(p1[0], p1[1], p1[2], 1.0));
                         */

    const LVector3f &pi = _data[i]._point;
    nc->rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                         RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                         RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                         RT_POINT, 1.0, pfVec4(pi[0], pi[1], pi[2], 1.0));
  }

  /*
  nc->append_cv(LVector3f(0.0, 0.0, 0.0));
  nc->recompute();
  nc->get_point(nc->get_max_t(), junk);
  const LVector3f &pi = _data[_data.size()-1]._point;
  nc->rebuild_curveseg(RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                       RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                       RT_CV | RT_KEEP_ORIG, 0.0, pfVec4(),
                       RT_CV, 0.0, pfVec4(pi[0], pi[1], pi[2], 1.0));
                       */
  
  nc->recompute();
  return nc;

#else

  // We start with the HermiteCurve produced above, then convert it to
  // NURBS form.
  PT(HermiteCurve) hc = new HermiteCurve();
  NurbsCurve *nc = new NurbsCurve(*hc);

  // Now we even out the knots to smooth out the curve and make
  // everything c2 continuous.

  int num_knots = nc->get_num_knots();

  // We expect this to be a 4th order curve, since we just converted
  // it from a Hermite.
  assert(nc->get_order() == 4);
  assert(num_knots > 0);

  // Now the knot sequence goes something like this:
  //    0 0 0 0 1 1 1 2 2 2 3 3 3 4 4 4 4

  // We'll consider pairs of knot values beginning at position 3 and
  // every third position thereafter.  We just even out these values
  // between their two neighbors.

  int i;
  double k1, k2 = nc->get_knot(num_knots-1);
  for (i = 3; i < num_knots - 4; i += 3) {
    k1 = nc->get_knot(i-1);
    k2 = nc->get_knot(i+2);
    nc->set_knot(i, (k1 + k1 + k2) / 3.0);
    nc->set_knot(i+1, (k1 + k2 + k2) / 3.0);
  }

  // The last knot must have the terminal value.
  nc->set_knot(num_knots-4, k2);

  // Finally, recompute the curve.
  nc->recompute();

  return nc;
#endif
}
  
////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::print
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CurveFitter::
print() const {
  output(cerr);
}
  
////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CurveFitter::
output(ostream &out) const {
  Data::const_iterator di;

  for (di = _data.begin(); di != _data.end(); ++di) {
    out << (*di) << "\n";
  }
}

