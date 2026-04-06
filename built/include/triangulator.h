/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulator.h
 * @author drose
 * @date 2007-01-17
 */

#ifndef TRIANGULATOR_H
#define TRIANGULATOR_H

#include "pandabase.h"
#include "luse.h"
#include "memoryBase.h"
#include "vector_int.h"

/**
 * This class can triangulate a convex or concave polygon, even one with
 * holes.  It is adapted from an algorithm published as:
 *
 * Narkhede A. and Manocha D., Fast polygon triangulation algorithm based on
 * Seidel's Algorithm, UNC-CH, 1994.
 *
 * http://www.cs.unc.edu/~dm/CODE/GEM/chapter.html
 *
 * It works strictly on 2-d points.  See Triangulator3 for 3-d points.
 */
class EXPCL_PANDA_MATHUTIL Triangulator : public MemoryBase {
PUBLISHED:
  Triangulator();

  void clear();
  int add_vertex(const LPoint2d &point);
  INLINE int add_vertex(double x, double y);

  INLINE int get_num_vertices() const;
  INLINE const LPoint2d &get_vertex(int n) const;
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);
  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex);

  void clear_polygon();
  void add_polygon_vertex(int index);
  INLINE bool is_left_winding() const;

  void begin_hole();
  void add_hole_vertex(int index);

  void triangulate();

  int get_num_triangles() const;
  int get_triangle_v0(int n) const;
  int get_triangle_v1(int n) const;
  int get_triangle_v2(int n) const;

protected:
  void cleanup_polygon_indices(vector_int &polygon);

  typedef pvector<LPoint2d> Vertices;
  Vertices _vertices;

  vector_int _polygon;

  typedef pvector<vector_int> Holes;
  Holes _holes;

  class Triangle {
  public:
    INLINE Triangle(Triangulator *t, int v0, int v1, int v2);
    int _v0, _v1, _v2;
  };

  typedef pvector<Triangle> Result;
  Result _result;


  typedef struct {
    double x, y;
  } point_t, vector_t;


  struct segment_t {
    INLINE segment_t();
    INLINE segment_t(Triangulator *t, int v0_i, int v1_i, int prev, int next);

    point_t v0, v1;             /* two endpoints */
    int is_inserted;            /* inserted in trapezoidation yet ? */
    int root0, root1;           /* root nodes in Q */
    int next;                   /* Next logical segment */
    int prev;                   /* Previous segment */
    int v0_i; // index to user's vertex number
  };

  typedef pvector<segment_t> Segments;
  Segments seg;
  vector_int permute;
  int choose_idx;

  /* Trapezoid attributes */

  typedef struct {
    int lseg, rseg;             /* two adjoining segments */
    point_t hi, lo;             /* max/min y-values */
    int u0, u1;
    int d0, d1;
    int sink;                   /* pointer to corresponding in Q */
    int usave, uside;           /* I forgot what this means */
    int state;
  } trap_t;


  /* Node attributes for every node in the query structure */

  typedef struct {
    int nodetype;                       /* Y-node or S-node */
    int segnum;
    point_t yval;
    int trnum;
    int parent;                 /* doubly linked DAG */
    int left, right;            /* children */
  } node_t;


  typedef struct {
    int vnum;
    int next;                   /* Circularly linked list  */
    int prev;                   /* describing the monotone */
    int marked;                 /* polygon */
  } monchain_t;


  typedef struct {
    point_t pt;
    int vnext[4];                       /* next vertices for the 4 chains */
    int vpos[4];                        /* position of v in the 4 chains */
    int nextfree;
    int user_i;  // index to user's vertex number
  } vertexchain_t;


  typedef pvector<node_t> QueryStructure;
  QueryStructure qs;
  typedef pvector<trap_t> TrapezoidStructure;
  TrapezoidStructure tr;

  /* Table to hold all the monotone */
  /* polygons . Each monotone polygon */
  /* is a circularly linked list */
  pvector<monchain_t> mchain;

  /* chain init. information. This */
  /* is used to decide which */
  /* monotone polygon to split if */
  /* there are several other */
  /* polygons touching at the same */
  /* vertex  */
  pvector<vertexchain_t> vert;

  /* contains position of any vertex in */
  /* the monotone chain for the polygon */
  vector_int mon;

  vector_int visited;

  bool check_left_winding(const vector_int &range) const;
  void make_segment(const vector_int &range, bool want_left_winding);

  int choose_segment();
  static double math_log2(double v);
  static int math_logstar_n(int n);
  static int math_N(int n, int h);

  int newnode();
  int newtrap();
  int _max(point_t *yval, point_t *v0, point_t *v1);
  int _min(point_t *yval, point_t *v0, point_t *v1);
  int _greater_than(point_t *v0, point_t *v1);
  int _equal_to(point_t *v0, point_t *v1);
  int _greater_than_equal_to(point_t *v0, point_t *v1);
  int _less_than(point_t *v0, point_t *v1);
  int init_query_structure(int segnum);
  int is_left_of(int segnum, point_t *v);
  int inserted(int segnum, int whichpt);
  int locate_endpoint(point_t *v, point_t *vo, int r);
  int merge_trapezoids(int segnum, int tfirst, int tlast, int side);
  int add_segment(int segnum);
  int find_new_roots(int segnum);
  int construct_trapezoids(int nseg);

  int inside_polygon(trap_t *t);
  int newmon();
  int new_chain_element();
  double get_angle(point_t *vp0, point_t *vpnext, point_t *vp1);
  int get_vertex_positions(int v0, int v1, int *ip, int *iq);
  int make_new_monotone_poly(int mcur, int v0, int v1);
  int monotonate_trapezoids(int n);
  int traverse_polygon(int mcur, int trnum, int from, int dir);
  void triangulate_monotone_polygons(int nvert, int nmonpoly);
  void triangulate_single_polygon(int nvert, int posmax, int side);

  friend class Triangle;
  friend struct segment_t;
};

#include "triangulator.I"

#endif
