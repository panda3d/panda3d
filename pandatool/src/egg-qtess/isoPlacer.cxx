/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file isoPlacer.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "isoPlacer.h"
#include "qtessSurface.h"
#include "subdivSegment.h"
#include "nurbsSurfaceResult.h"
#include "pvector.h"


/**
 *
 */
void IsoPlacer::
get_scores(int subdiv, int across, double ratio,
           NurbsSurfaceResult *surf, bool s) {
  _maxi = subdiv - 1;

  _cscore.clear();
  _sscore.clear();

  _cscore.reserve(_maxi);
  _sscore.reserve(_maxi);

  // First, tally up the curvature and stretch scores across the surface.
  int i = 0;
  for (i = 0; i < _maxi; i++) {
    _cscore.push_back(0.0);
    _sscore.push_back(0.0);
  }

  int a;
  for (a = 0; a <= across; a++) {
    double v = (double)a / (double)across;

    LVecBase3 p1, p2, p3, pnext;
    LVecBase3 v1, v2;
    if (s) {
      surf->eval_point(0.0, v, p3);
    } else {
      surf->eval_point(v, 0.0, p3);
    }
    int num_points = 1;

    for (i = -1; i < _maxi; i++) {
      double u = (double)(i+1) / (double)(_maxi+1);
      if (s) {
        surf->eval_point(u, v, pnext);
      } else {
        surf->eval_point(v, u, pnext);
      }

      // We'll ignore consecutive equal points.  They don't contribute to
      // curvature or size.
      if (!pnext.almost_equal(p3)) {
        num_points++;
        p1 = p2;
        p2 = p3;
        p3 = pnext;

        v1 = v2;
        v2 = p3 - p2;
        double vlength = length(v2);
        v2 /= vlength;

        if (i >= 0) {
          _sscore[i] += vlength;
        }

        if (num_points >= 3) {
          // We only have a meaningful v1, v2 when we've read at least three
          // non-equal points.
          double d = v1.dot(v2);

          _cscore[i] += acos(std::max(std::min(d, 1.0), -1.0));
        }
      }
    }
  }

  // Now integrate.
  _cint.clear();
  _cint.reserve(_maxi + 1);

  double net = 0.0;
  double ad = (double)(across+1);
  _cint.push_back(0.0);
  for (i = 0; i < _maxi; i++) {
    net += _cscore[i]/ad + ratio * _sscore[i]/ad;
    _cint.push_back(net);
  }
}

/**
 *
 */
void IsoPlacer::
place(int count, pvector<double> &iso_points) {
  int i;

  // Count up the average curvature.
  double avg_curve = 0.0;
  for (i = 0; i < _maxi; i++) {
    avg_curve += _cscore[i];
  }
  avg_curve /= (double)_maxi;

  // Find all the local maxima in the curvature table.  These are bend points.
  typedef pvector<int> BendPoints;
  BendPoints bpoints;
  BendPoints::iterator bi, bnext;

  typedef pvector<SubdivSegment> Segments;
  Segments segments;
  Segments::iterator si;

  /*
  // Having problems with bend points right now.  Maybe this is just a bad
  // idea.  It seems to work pretty well without them, anyway.
  for (i = 1; i < _maxi-1; i++) {
    // A point must be measurably higher than both its neighbors, as well as
    // at least 50% more curvy than the average curvature, to qualify as a
    // bend point.
    if (_cscore[i] > _cscore[i-1]+0.001 &&
        _cscore[i] > _cscore[i+1]+0.001 &&
        _cscore[i] > 1.5 * avg_curve) {
      bpoints.push_back(i);
    }
  }
  */

  // Now make sure there aren't any two bend points closer together than
  // maxicount.  If there are, remove the smaller of the two.
  bi = bpoints.begin();
  int min_separation = _maxi/count;
  while (bi != bpoints.end()) {
    bnext = bi;
    ++bnext;

    if (bnext != bpoints.end() && (*bnext) - (*bi) < min_separation) {
      // Too close.  Remove one.
      if (_cscore[*bnext] > _cscore[*bi]) {
        *bi = *bnext;
      }
      bpoints.erase(bnext);
    } else {
      // Not too close; keep going;
      bi = bnext;
    }
  }

  // Now, if we have fewer total subdivisions than bend points, then remove
  // the smallest bend points.
  while (count - 1 < (int)bpoints.size()) {
    bi = bpoints.begin();
    BendPoints::iterator mi = bi;
    for (++bi; bi != bpoints.end(); ++bi) {
      if (_cscore[*bi] < _cscore[*mi]) {
        mi = bi;
      }
    }
    bpoints.erase(mi);
  }

  // Now all the remaining bend points are valid.
  bi = bpoints.begin();
  int last = 0;
  for (bi = bpoints.begin(); bi != bpoints.end(); ++bi) {
    segments.push_back(SubdivSegment(&_cint[0], last, *bi));
    last = *bi;
  }
  segments.push_back(SubdivSegment(&_cint[0], last, _maxi));

  int nr = count - segments.size();

  // Now we have subdivided the curve into a number of smaller curves at the
  // bend points.  We still have nr remaining cuts to make; distribute these
  // cuts among the curves evenly according to score.

  // Divvy out the extra cuts.  First, each segment gets an amount
  // proportional to its score.
  double net_score = _cint[_maxi];
  nassertv(net_score > 0.0);
  int ns = 0;
  for (si = segments.begin(); si != segments.end(); ++si) {
    (*si)._num_cuts = (int)floor(nr * (*si).get_score() / net_score);
    nassertv((*si)._num_cuts <= nr);  // This fails if net_score is nan.
    ns += (*si)._num_cuts;
  }

  // Then, assign the remaining cuts to the neediest segments.
  nr -= ns;
  while (nr > 0) {
    si = min_element(segments.begin(), segments.end());
    (*si)._num_cuts++;
    nr--;
  }

  // Now cut up the segments as indicated.
  for (si = segments.begin(); si != segments.end(); ++si) {
    (*si).cut();
  }

  // Finally, return the result.
  iso_points.erase(iso_points.begin(), iso_points.end());

  iso_points.push_back(0.0);
  for (si = segments.begin(); si != segments.end(); ++si) {
    pvector<int>::iterator ci;
    for (ci = (*si)._cuts.begin(); ci != (*si)._cuts.end(); ++ci) {
      iso_points.push_back((*ci+1) / (double)(_maxi+1));
    }
    iso_points.push_back(((*si)._t+1) / (double)(_maxi+1));
  }

  // Oh, wait.  The last segment is actually drawn all the way to 1.
  iso_points.back() = 1.0;
}
