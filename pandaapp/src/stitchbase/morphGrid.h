// Filename: morphGrid.h
// Created by:  drose (08Nov99)
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

#ifndef MORPHGRID_H
#define MORPHGRID_H

#include "luse.h"

class MorphGrid {
public:
  MorphGrid();
  ~MorphGrid();

  enum TableType {
    TT_in = 0,
    TT_out = 1,
    TT_num = 2
  };

  bool is_empty() const;
  void clear();

  void init(int x_verts, int y_verts);
  void recompute();
  void fill_alpha();

  LPoint2d morph_point(const LPoint2d &p, TableType from, TableType to);
  double get_alpha(const LPoint2d &p, TableType from);

  LPoint2d morph_in(const LPoint2d &p) const;
  LPoint2d morph_out(const LPoint2d &p) const;
  double get_alpha(const LPoint2d &p) const;

private:
  class Triangle;
public:

  class Vertex {
  public:
    Vertex(const LPoint2d &p);

    LPoint2d _p[TT_num];  // TT_in, TT_out

    // These members are used to feather the edges of the images where
    // they overlap other images.  Once the Stitcher sets the
    // _over_another flags appropriately, MorphGrid::fill_alpha() will
    // assign the alpha values to feather the edges.
    double _alpha;
    bool _over_another;
    int _dist_from_interior;
  };

  int _x_verts, _y_verts;
  typedef vector<Vertex> Row;
  typedef vector<Row> Table;
  Table _table;

private:
  void count_dist_from_interior(int x, int y, int dist);

  class Triangle {
  public:
    Triangle(Vertex *v0, Vertex *v1, Vertex *v2);
    bool contains_point(const LPoint2d &p, TableType from) const;
    LPoint2d morph_point(const LPoint2d &p, TableType from, TableType to) const;
    double get_alpha(const LPoint2d &p, TableType from) const;
    void recompute();

    Vertex *_v[3];
    LPoint2d _min_p[TT_num], _max_p[TT_num];
    LMatrix3d _mat[TT_num], _inv[TT_num];
  };

  typedef vector<Triangle> Triangles;
  Triangles _triangles;
  Triangle *_last_triangle;

  class TriangleTree {
  public:
    TriangleTree(Triangle *a, Triangle *b);
    TriangleTree(TriangleTree *a, TriangleTree *b);
    ~TriangleTree();
    void recompute();
    Triangle *find_triangle(const LPoint2d &p, TableType from) const;

    bool _has_tris;
    union {
      Triangle *_tri[2];
      TriangleTree *_tree[2];
    } _u;

    LPoint2d _min_p[TT_num], _max_p[TT_num];
  };

  TriangleTree *_tree;
  friend class TriangleTree;
};

#endif
