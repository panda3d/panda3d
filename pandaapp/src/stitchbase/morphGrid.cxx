// Filename: morphGrid.cxx
// Created by:  drose (08Nov99)
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

#include "morphGrid.h"
#include "triangle.h"

#include "mathNumbers.h"

#include <math.h>
#include <assert.h>

MorphGrid::Vertex::
Vertex(const LPoint2d &p) {
  for (int i = 0; i < (int)TT_num; i++) {
    _p[i] = p;
  }
  _alpha = 1.0;
  _over_another = false;

  // -1 on the distance counter is a flag that the value hasn't yet
  // been computed.
  _dist_from_interior = -1;
}

MorphGrid::Triangle::
Triangle(Vertex *v0, Vertex *v1, Vertex *v2) {
  _v[0] = v0;
  _v[1] = v1;
  _v[2] = v2;
}

bool MorphGrid::Triangle::
contains_point(const LPoint2d &p, TableType from) const {
  if ((p[0] < _min_p[from][0] || p[0] > _max_p[from][0]) ||
      (p[1] < _min_p[from][1] || p[1] > _max_p[from][1])) {
    // Doesn't pass the minmax test.
    return false;
  }

  return triangle_contains_point(p, _v[0]->_p[from], _v[1]->_p[from],
                                 _v[2]->_p[from]);
}

LPoint2d MorphGrid::Triangle::
morph_point(const LPoint2d &p, TableType from, TableType to) const {
  return (p * _inv[from]) * _mat[to];
}

double MorphGrid::Triangle::
get_alpha(const LPoint2d &p, TableType from) const {
  LPoint2d q = p * _inv[from];

  // Now q is a point in a right triangle, where (0,1) is v0, (0,0) is
  // v1, and (1,0) is v2.  Interpolate the appropriate alpha value
  // based on this coordinate system.

  double alpha01 = (_v[0]->_alpha + q[1] * (_v[1]->_alpha - _v[0]->_alpha));
  return (alpha01 + q[0] * (_v[2]->_alpha - alpha01));
}

void MorphGrid::Triangle::
recompute() {
  for (int i = 0; i < (int)TT_num; i++) {
    for (int a = 0; a < 2; a++) {
      _min_p[i][a] = min(min(_v[0]->_p[i][a], _v[1]->_p[i][a]),
                         _v[2]->_p[i][a]);
      _max_p[i][a] = max(max(_v[0]->_p[i][a], _v[1]->_p[i][a]),
                         _v[2]->_p[i][a]);
    }

    LPoint2d origin = _v[1]->_p[i];
    LVector2d yaxis = _v[0]->_p[i] - origin;
    LVector2d xaxis = _v[2]->_p[i] - origin;

    _mat[i] = LMatrix3d(xaxis[0], xaxis[1], 0.0,
                        yaxis[0], yaxis[1], 0.0,
                        origin[0], origin[1], 1.0);

    _inv[i] = invert(_mat[i]);
  }
}

MorphGrid::TriangleTree::
TriangleTree(Triangle *a, Triangle *b) {
  _has_tris = true;
  _u._tri[0] = a;
  _u._tri[1] = b;
}

MorphGrid::TriangleTree::
TriangleTree(TriangleTree *a, TriangleTree *b) {
  _has_tris = false;
  _u._tree[0] = a;
  _u._tree[1] = b;
}

MorphGrid::TriangleTree::
~TriangleTree() {
  if (!_has_tris) {
    delete _u._tree[0];
    delete _u._tree[1];
  }
}

void MorphGrid::TriangleTree::
recompute() {
  if (_has_tris) {
    _u._tri[0]->recompute();
    _u._tri[1]->recompute();

    for (int i = 0; i < (int)TT_num; i++) {
      for (int a = 0; a < 2; a++) {
        _min_p[i][a] =
          min(_u._tri[0]->_min_p[i][a], _u._tri[1]->_min_p[i][a]);
        _max_p[i][a] =
          max(_u._tri[0]->_max_p[i][a], _u._tri[1]->_max_p[i][a]);
      }
    }
  } else {
    _u._tree[0]->recompute();
    _u._tree[1]->recompute();

    for (int i = 0; i < (int)TT_num; i++) {
      for (int a = 0; a < 2; a++) {
        _min_p[i][a] =
          min(_u._tree[0]->_min_p[i][a], _u._tree[1]->_min_p[i][a]);
        _max_p[i][a] =
          max(_u._tree[0]->_max_p[i][a], _u._tree[1]->_max_p[i][a]);
      }
    }
  }
}

MorphGrid::Triangle *MorphGrid::TriangleTree::
find_triangle(const LPoint2d &p, TableType from) const {
  if ((p[0] < _min_p[from][0] || p[0] > _max_p[from][0]) ||
      (p[1] < _min_p[from][1] || p[1] > _max_p[from][1])) {
    // Doesn't pass the minmax test.
    return NULL;
  }

  if (_has_tris) {
    if (_u._tri[0]->contains_point(p, from)) {
      return _u._tri[0];
    }
    if (_u._tri[1]->contains_point(p, from)) {
      return _u._tri[1];
    }
    return NULL;
  } else {
    Triangle *t = _u._tree[0]->find_triangle(p, from);
    if (t == NULL) {
      t = _u._tree[1]->find_triangle(p, from);
    }
    return t;
  }
}

MorphGrid::
MorphGrid() {
  _x_verts = 0;
  _y_verts = 0;
  _last_triangle = NULL;
  _tree = NULL;
}

MorphGrid::
~MorphGrid() {
  if (_tree != NULL) {
    delete _tree;
  }
}

bool MorphGrid::
is_empty() const {
  return _x_verts <= 0 || _y_verts <= 0;
}

void MorphGrid::
clear() {
  init(0, 0);
}

void MorphGrid::
init(int x_verts, int y_verts) {
  _x_verts = x_verts;
  _y_verts = y_verts;

  if (_tree != NULL) {
    delete _tree;
    _tree = NULL;
  }
  _triangles.clear();
  _last_triangle = NULL;
  _table.clear();

  if (is_empty()) {
    return;
  }

  // Create a 2-d table of vertices.
  _table.reserve(_y_verts);
  int x, y;
  for (y = 0; y < _y_verts; y++) {
    _table.push_back(Row());
    _table[y].clear();
    _table[y].reserve(_x_verts);
    for (x = 0; x < _x_verts; x++) {
      LPoint2d p((double)x / (double)(_x_verts - 1),
                 1.0 - (double)y / (double)(_y_verts - 1));
      _table[y].push_back(Vertex(p));
    }
  }

  // Now create a bunch of triangles for these vertices.
  int num_tris = (_y_verts - 1) * (_x_verts - 1) * 2;

  _triangles.reserve(num_tris);
  for (y = 0; y + 1 < _y_verts; y++) {
    for (x = 0; x + 1 < _x_verts; x++) {
      _triangles.push_back(Triangle(&_table[y][x],
                                    &_table[y + 1][x],
                                    &_table[y + 1][x + 1]));
      _triangles.push_back(Triangle(&_table[y][x],
                                    &_table[y + 1][x + 1],
                                    &_table[y][x + 1]));
    }
  }
  assert((int)_triangles.size() == num_tris);

  // Now create a 2-d table of TriangleTree nodes, each of which
  // points to a pair of triangles.  We'll use this to build up the
  // TriangleTree structure.
  typedef vector<TriangleTree *> TRow;
  typedef vector<TRow> TTable;

  TTable tree;

  int x_tree = _x_verts - 1;
  int y_tree = _y_verts - 1;
  tree.reserve(y_tree);
  int i = 0;
  for (y = 0; y < y_tree; y++) {
    tree.push_back(TRow());
    tree[y].clear();
    tree[y].reserve(x_tree);
    for (x = 0; x < x_tree; x++) {
      tree[y].push_back(new TriangleTree(&_triangles[i],
                                         &_triangles[i + 1]));
      i += 2;
    }
  }
  assert(i == num_tris);

  // Now repeatedly pair up adjacent TriangleTree nodes, each time
  // making a new level with half the number of nodes, until we end up
  // with a single node.
  while (x_tree > 1 || y_tree > 1) {
    // Collapse horizontal pairs.
    int tx = 0;
    for (int y = 0; y < y_tree; y++) {
      tx = 0;
      int fx = 0;
      while (fx + 1 < x_tree) {
        tree[y][tx++] = new TriangleTree(tree[y][fx], tree[y][fx + 1]);
        fx += 2;
      }
      if (fx < x_tree) {
        // One more odd element remaining, just copy it up.
        tree[y][tx++] = tree[y][fx];
        fx++;
      }
      assert(fx == x_tree);
    }
    x_tree = tx;

    // Collapse vertical pairs.
    int ty = 0;
    for (int x = 0; x < x_tree; x++) {
      ty = 0;
      int fy = 0;
      while (fy + 1 < y_tree) {
        tree[ty++][x] = new TriangleTree(tree[fy][x], tree[fy + 1][x]);
        fy += 2;
      }
      if (fy < y_tree) {
        // One more odd element remaining, just copy it up.
        tree[ty++][x] = tree[fy][x];
        fy++;
      }
      assert(fy == y_tree);
    }
    y_tree = ty;
  }

  assert(x_tree == 1 && y_tree == 1);
  _tree = tree[0][0];
}

void MorphGrid::
recompute() {
  _tree->recompute();
}

void MorphGrid::
fill_alpha() {
  // The stitcher has already made a distinction between interior
  // points (that is, points which are over no other image, and must
  // be 100% opaque) and exterior points (points which lay over
  // another image, and should be feathered).  We now need to
  // determine the distance each exterior point is from this
  // interior/exterior dividing line.

  // To do this, we first find an interior point.
  bool found_interior = false;
  int x, y;
  for (y = 0; y < _y_verts && !found_interior; y++) {
    for (x = 0; x < _x_verts && !found_interior; x++) {
      if (!_table[y][x]._over_another) {
        // Here's one!
        found_interior = true;
        count_dist_from_interior(x, y, 0);
      }
    }
  }

  if (!found_interior) {
    // There are no interior points in this image--it entirely covers
    // other images.  (Doesn't seem to be much point to it, does
    // there?)  We'll just feather the edges a little.
    for (y = 0; y < _y_verts; y++) {
      _table[y][0]._alpha = 0.0;
      _table[y][_x_verts - 1]._alpha = 0.0;
    }
    for (x = 0; x < _x_verts; x++) {
      _table[0][x]._alpha = 0.0;
      _table[_y_verts - 1][x]._alpha = 0.0;
    }
    return;
  }

  // Now go back through and assign the alpha based on the relative
  // distance of each point from the edge and from the interior.
  for (y = 0; y < _y_verts; y++) {
    for (x = 0; x < _x_verts; x++) {
      if (!_table[y][x]._over_another) {
        _table[y][x]._alpha = 1.0;

      } else {
        int dist_from_edge =
          min(min(x, y),
              min(_x_verts - 1 - x, _y_verts - 1 - y));

        assert(_table[y][x]._dist_from_interior >= 0);

        // We subtract one from dist_from_interior to give us a bit of
        // comfort zone around the interior edge--we're not precisely
        // sure where the actual edge is.
        int dist_from_interior =
          max(_table[y][x]._dist_from_interior - 1, 0);

        // Now if dist_from_edge is 0, it must be transparent; if
        // dist_from_interior is 0, it must be opaque.  Any other
        // combination should be some value in between.
        if (dist_from_interior == 0) {
          _table[y][x]._alpha = 1.0;

        } else if (dist_from_edge == 0) {
          _table[y][x]._alpha = 0.0;

        } else {
          double ratio = (double)dist_from_interior /
            (double)(dist_from_interior + dist_from_edge);

          _table[y][x]._alpha = (cos(ratio * MathNumbers::pi) + 1.0) / 2.0;
        }
      }
    }
  }
}

LPoint2d MorphGrid::
morph_point(const LPoint2d &p, TableType from, TableType to) {
  if (is_empty()) {
    return p;
  }

  if (_last_triangle != NULL) {
    // First, check to see if the point is within the same triangle as
    // the last point was.  This will save a bit of time if it is.
    if (_last_triangle->contains_point(p, from)) {
      return _last_triangle->morph_point(p, from, to);
    }
  }

  // Nope, we just blew cache.  We'll have to look for the containing
  // triangle the hard way.
  assert(_tree != NULL);
  _last_triangle = _tree->find_triangle(p, from);

  if (_last_triangle == NULL) {
    return p;
  } else {
    return _last_triangle->morph_point(p, from, to);
  }
}

double MorphGrid::
get_alpha(const LPoint2d &p, TableType from) {
  if (is_empty()) {
    return 1.0;
  }

  if (_last_triangle != NULL) {
    // First, check to see if the point is within the same triangle as
    // the last point was.  This will save a bit of time if it is.
    if (_last_triangle->contains_point(p, from)) {
      return _last_triangle->get_alpha(p, from);
    }
  }

  // Nope, we just blew cache.  We'll have to look for the containing
  // triangle the hard way.
  assert(_tree != NULL);
  _last_triangle = _tree->find_triangle(p, from);

  if (_last_triangle == NULL) {
    return 1.0;
  } else {
    return _last_triangle->get_alpha(p, from);
  }
}


LPoint2d MorphGrid::
morph_in(const LPoint2d &p) const {
  return ((MorphGrid *)this)->morph_point(p, TT_out, TT_in);
}

LPoint2d MorphGrid::
morph_out(const LPoint2d &p) const {
  return ((MorphGrid *)this)->morph_point(p, TT_in, TT_out);
}

double MorphGrid::
get_alpha(const LPoint2d &p) const {
  return ((MorphGrid *)this)->get_alpha(p, TT_in);
}

void MorphGrid::
count_dist_from_interior(int x, int y, int dist) {
  if (x >= 0 && x < _x_verts &&
      y >= 0 && y < _y_verts) {
    Vertex &v = _table[y][x];
    if (!v._over_another) {
      // Here we are in the interior.
      dist = 0;
    }

    if (v._dist_from_interior < 0 || dist < v._dist_from_interior) {
      // Update this point, and recurse to our neighbors.
      v._dist_from_interior = dist;

      count_dist_from_interior(x + 1, y, dist + 1);
      count_dist_from_interior(x - 1, y, dist + 1);
      count_dist_from_interior(x, y + 1, dist + 1);
      count_dist_from_interior(x, y - 1, dist + 1);
    }
  }
}
