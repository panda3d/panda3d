/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file triangulator.cxx
 * @author drose
 * @date 2007-01-18
 */

#include "triangulator.h"
#include "randomizer.h"

/**
 *
 */
Triangulator::
Triangulator() {
}

/**
 * Removes all vertices and polygon specifications from the Triangulator, and
 * prepares it to start over.
 */
void Triangulator::
clear() {
  _vertices.clear();
  clear_polygon();
}

/**
 * Adds a new vertex to the vertex pool.  Returns the vertex index number.
 */
int Triangulator::
add_vertex(const LPoint2d &point) {
  int index = (int)_vertices.size();
  _vertices.push_back(point);
  return index;
}

/**
 * Removes the current polygon definition (and its set of holes), but does not
 * clear the vertex pool.
 */
void Triangulator::
clear_polygon() {
  _polygon.clear();
  _holes.clear();
}

/**
 * Adds the next consecutive vertex of the polygon.  This vertex should index
 * into the vertex pool established by repeated calls to add_vertex().
 *
 * The vertices may be listed in either clockwise or counterclockwise order.
 * Vertices should not be repeated.  In particular, do not repeat the first
 * vertex at the end.
 */
void Triangulator::
add_polygon_vertex(int index) {
  _polygon.push_back(index);
}

/**
 * Finishes the previous hole, if any, and prepares to add a new hole.
 */
void Triangulator::
begin_hole() {
  _holes.push_back(vector_int());
}

/**
 * Adds the next consecutive vertex of the current hole.  This vertex should
 * index into the vertex pool established by repeated calls to add_vertex().
 *
 * The vertices may be listed in either clockwise or counterclockwise order.
 * Vertices should not be repeated.
 */
void Triangulator::
add_hole_vertex(int index) {
  nassertv(!_holes.empty());
  _holes.back().push_back(index);
}

/**
 * Does the work of triangulating the specified polygon.  After this call, you
 * may retrieve the new triangles one at a time by iterating through
 * get_triangle_v0/1/2().
 */
void Triangulator::
triangulate() {
  _result.clear();

  // Make sure our index numbers are reasonable.
  cleanup_polygon_indices(_polygon);
  Holes::iterator hi;
  for (hi = _holes.begin(); hi != _holes.end(); ++hi) {
    cleanup_polygon_indices(*hi);
  }

  if (_polygon.size() < 3) {
    // Degenerate case.
    return;
  }

  // Set up the list of segments.
  seg.clear();
  seg.push_back(segment_t());  // we don't use the first entry.
  make_segment(_polygon, true);

  for (hi = _holes.begin(); hi != _holes.end(); ++hi) {
    if ((*hi).size() >= 3) {
      make_segment(*hi, false);
    }
  }

  // Shuffle the segment index.
  int num_segments = (int)seg.size() - 1;
  permute.reserve(num_segments);
  int i;
  for (i = 0; i < num_segments; ++i) {
    permute.push_back(i + 1);
  }

  // Actually, I'm not sure why we should shuffle the index.  That makes the
  // result non-deterministic, and isn't one order--for instance, the initial
  // order--as good as any other?
  /*
  Randomizer randomizer;
  for (i = 0; i < num_segments; ++i) {
    int j = randomizer.random_int(num_segments);
    nassertv(j >= 0 && j < num_segments);
    int t = permute[i];
    permute[i] = permute[j];
    permute[j] = t;
  }
  */
  choose_idx = 0;

  /*
  // cerr << "got " << num_segments << " segments\n";
  for (i = 1; i < (int)seg.size(); ++i) {
    segment_t &s = seg[i];
    printf("  %d. (%g %g), (%g %g)\n", i, s.v0.x, s.v0.y, s.v1.x, s.v1.y);
    printf("    root0 = %d, root1 = %d\n", s.root0, s.root1);
    printf("    next = %d, prev = %d\n", s.next, s.prev);
  }
  */

  while (construct_trapezoids(num_segments) != 0) {
    // If there's an error, re-shuffle the index and try again.
    Randomizer randomizer;
    for (i = 0; i < num_segments; ++i) {
      int j = randomizer.random_int(num_segments);
      nassertv(j >= 0 && j < num_segments);
      int t = permute[i];
      permute[i] = permute[j];
      permute[j] = t;
    }
    choose_idx = 0;

    /*
    // cerr << "got " << num_segments << " segments\n";
    for (i = 1; i < (int)seg.size(); ++i) {
      segment_t &s = seg[i];
      printf("  %d. (%g %g), (%g %g)\n", i, s.v0.x, s.v0.y, s.v1.x, s.v1.y);
      printf("    root0 = %d, root1 = %d\n", s.root0, s.root1);
      printf("    next = %d, prev = %d\n", s.next, s.prev);
    }
    */
  }

  /*
  // cerr << "got " << tr.size() - 1 << " trapezoids\n";
  for (i = 1; i < (int)tr.size(); ++i) {
    trap_t &t = tr[i];
    // cerr << "  " << i << ". state = " << t.state << "\n"; cerr << "    lseg
    // = " << t.lseg << " rseg = " << t.rseg << "\n"; cerr << "    hi = " <<
    // t.hi.x << " " << t.hi.y << " lo = " << t.lo.x << " " << t.lo.y << "\n";
  }
  */

  int nmonpoly = monotonate_trapezoids(num_segments);

  // cerr << "got " << nmonpoly << " monotone polygons\n";

  triangulate_monotone_polygons(num_segments, nmonpoly);

  /*
  Result::iterator ri;
  for (ri = _result.begin(); ri != _result.end(); ++ri) {
    // cerr << "tri: " << (*ri)._v0 << " " << (*ri)._v1 << " " << (*ri)._v2 <<
    // "\n";
  }
  */
}

/**
 * Returns the number of triangles generated by the previous call to
 * triangulate().
 */
int Triangulator::
get_num_triangles() const {
  return _result.size();
}

/**
 * Returns vertex 0 of the nth triangle generated by the previous call to
 * triangulate().
 *
 * This is a zero-based index into the vertices added by repeated calls to
 * add_vertex().
 */
int Triangulator::
get_triangle_v0(int n) const {
  nassertr(n >= 0 && n < (int)_result.size(), -1);
  return _result[n]._v0;
}

/**
 * Returns vertex 1 of the nth triangle generated by the previous call to
 * triangulate().
 *
 * This is a zero-based index into the vertices added by repeated calls to
 * add_vertex().
 */
int Triangulator::
get_triangle_v1(int n) const {
  nassertr(n >= 0 && n < (int)_result.size(), -1);
  return _result[n]._v1;
}

/**
 * Returns vertex 2 of the nth triangle generated by the previous call to
 * triangulate().
 *
 * This is a zero-based index into the vertices added by repeated calls to
 * add_vertex().
 */
int Triangulator::
get_triangle_v2(int n) const {
  nassertr(n >= 0 && n < (int)_result.size(), -1);
  return _result[n]._v2;
}

/**
 * Removes any invalid index numbers from the list.
 */
void Triangulator::
cleanup_polygon_indices(vector_int &polygon) {
  // First, check for index bounds.
  size_t pi = 0;
  while (pi < polygon.size()) {
    if (polygon[pi] >= 0 && (size_t)polygon[pi] < _vertices.size()) {
      // This vertex is OK.
      ++pi;
    } else {
      // This index is out-of-bounds; remove it.
      polygon.erase(_polygon.begin() + pi);
    }
  }

  // Now, remove any consecutive repeated vertices.
  pi = 1;
  while (pi < polygon.size()) {
    if (_vertices[polygon[pi]] != _vertices[polygon[pi - 1]]) {
      // This vertex is OK.
      ++pi;
    } else {
      // This vertex repeats the previous one; remove it.
      polygon.erase(_polygon.begin() + pi);
    }
  }

  if (polygon.size() > 1 && _vertices[polygon.back()] == _vertices[_polygon.front()]) {
    // The last vertex repeats the first one; remove it.
    polygon.pop_back();
  }
}


// The remainder of the code in this file is adapted more or less from the C
// code published with the referenced paper.

#define T_X     1
#define T_Y     2
#define T_SINK  3


#define FIRSTPT 1               /* checking whether pt. is inserted */
#define LASTPT  2


#define REALLY_BIG 1<<30
#define C_EPS 1.0e-7            /* tolerance value: Used for making */
                                /* all decisions about collinearity or */
                                /* left/right of segment. Decrease */
                                /* this value if the input points are */
                                /* spaced very close together */


#define S_LEFT 1                /* for merge-direction */
#define S_RIGHT 2


#define ST_VALID 1              /* for trapezium state */
#define ST_INVALID 2


#define SP_SIMPLE_LRUP 1        /* for splitting trapezoids */
#define SP_SIMPLE_LRDN 2
#define SP_2UP_2DN     3
#define SP_2UP_LEFT    4
#define SP_2UP_RIGHT   5
#define SP_2DN_LEFT    6
#define SP_2DN_RIGHT   7
#define SP_NOSPLIT    -1

#define TR_FROM_UP 1            /* for traverse-direction */
#define TR_FROM_DN 2

#define TRI_LHS 1
#define TRI_RHS 2


#define CROSS(v0, v1, v2) (((v1).x - (v0).x)*((v2).y - (v0).y) - \
                           ((v1).y - (v0).y)*((v2).x - (v0).x))

#define DOT(v0, v1) ((v0).x * (v1).x + (v0).y * (v1).y)

#define FP_EQUAL(s, t) (fabs(s - t) <= C_EPS)


#define CROSS_SINE(v0, v1) ((v0).x * (v1).y - (v1).x * (v0).y)
#define LENGTH(v0) (sqrt((v0).x * (v0).x + (v0).y * (v0).y))


/**
 * Returns true if the list of vertices is counter-clockwise, false if it is
 * clockwise.
 */
bool Triangulator::
check_left_winding(const vector_int &range) const {
  // We do this by computing the polygon's signed area.  If it comes out
  // negative, the polygon is right-winding.

  double area = 0.0;
  size_t j = range.size() - 1;
  for (size_t i = 0; i < range.size(); ++i) {
    const LPoint2d &p0 = _vertices[range[j]];
    const LPoint2d &p1 = _vertices[range[i]];
    area += p0[0] * p1[1] - p0[1] * p1[0];
    j = i;
  }

  return area >= 0.0;
}

/**
 * Converts a linear list of integer vertices to a list of segment_t.  If
 * want_left_winding is true, the list is reversed if necessary to make it
 * left-winding; otherwise, it is reversed to make it right-winding.
 */
void Triangulator::
make_segment(const vector_int &range, bool want_left_winding) {
  int num_points = (int)range.size();
  nassertv(num_points >= 2);

  int first = (int)seg.size();
  int last = first + num_points - 1;

  if (want_left_winding == check_left_winding(range)) {
    // Keep it in its natural order.
    int first = (int)seg.size();
    int last = first + num_points - 1;

    seg.push_back(segment_t(this, range[0], range[1],
                            last, first + 1));

    for (int i = 1; i < num_points - 1; ++i) {
      seg.push_back(segment_t(this, range[i], range[i + 1],
                              first + i - 1, first + i + 1));
    }

    seg.push_back(segment_t(this, range[num_points - 1], range[0],
                            last - 1, first));

  } else {
    // Reverse it.
    seg.push_back(segment_t(this, range[0], range[num_points - 1],
                            last, first + 1));

    for (int i = 1; i < num_points - 1; ++i) {
      seg.push_back(segment_t(this, range[num_points - i], range[num_points - i - 1],
                              first + i - 1, first + i + 1));
    }

    seg.push_back(segment_t(this, range[1], range[0],
                            last - 1, first));

  }
}

/* Return the next segment in the generated random ordering of all the */
/* segments in S */
int Triangulator::
choose_segment() {
  nassertr(choose_idx < (int)permute.size(), 0);
  // segment_t &s = seg[permute[choose_idx]]; cerr << "choose_segment " <<
  // permute[choose_idx] << ": " << s.v0.x << ", " << s.v0.y << " to " <<
  // s.v1.x << ", " << s.v1.y << "\n";
  return permute[choose_idx++];
}

double Triangulator::
math_log2(double v) {
  static const double log2 = log(2.0);
  return log(v) / log2;
}

/* Get log*n for given n */
int Triangulator::
math_logstar_n(int n) {
  int i;
  double v;

  for (i = 0, v = (double) n; v >= 1; i++)
    v = math_log2(v);

  return (i - 1);
}


int Triangulator::
math_N(int n, int h) {
  int i;
  double v;

  for (i = 0, v = (int) n; i < h; i++)
    v = math_log2(v);

  return (int) ceil((double) 1.0*n/v);
}


/* Return a new node to be added into the query tree */
int Triangulator::newnode() {
  int index = (int)qs.size();
  qs.push_back(node_t());
  // cerr << "creating new node " << index << "\n";
  return index;
}

/* Return a free trapezoid */
int Triangulator::newtrap() {
  int tr_idx = (int)tr.size();
  tr.push_back(trap_t());
  tr[tr_idx].lseg = -1;
  tr[tr_idx].rseg = -1;
  tr[tr_idx].state = ST_VALID;
  // cerr << "creating new trapezoid " << tr_idx << "\n";
  return tr_idx;
}


/* Return the maximum of the two points into the yval structure */
int Triangulator::_max(point_t *yval, point_t *v0, point_t *v1) {
  if (v0->y > v1->y + C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x > v1->x + C_EPS)
        *yval = *v0;
      else
        *yval = *v1;
    }
  else
    *yval = *v1;

  return 0;
}


/* Return the minimum of the two points into the yval structure */
int Triangulator::_min(point_t *yval, point_t *v0, point_t *v1) {
  if (v0->y < v1->y - C_EPS)
    *yval = *v0;
  else if (FP_EQUAL(v0->y, v1->y))
    {
      if (v0->x < v1->x)
        *yval = *v0;
      else
        *yval = *v1;
    }
  else
    *yval = *v1;

  return 0;
}


int Triangulator::
_greater_than(point_t *v0, point_t *v1) {
  if (v0->y > v1->y + C_EPS)
    return true;
  else if (v0->y < v1->y - C_EPS)
    return false;
  else
    return (v0->x > v1->x);
}


int Triangulator::
_equal_to(point_t *v0, point_t *v1) {
  return (FP_EQUAL(v0->y, v1->y) && FP_EQUAL(v0->x, v1->x));
}

int Triangulator::
_greater_than_equal_to(point_t *v0, point_t *v1) {
  if (v0->y > v1->y + C_EPS)
    return true;
  else if (v0->y < v1->y - C_EPS)
    return false;
  else
    return (v0->x >= v1->x);
}

int Triangulator::
_less_than(point_t *v0, point_t *v1) {
  if (v0->y < v1->y - C_EPS)
    return true;
  else if (v0->y > v1->y + C_EPS)
    return false;
  else
    return (v0->x < v1->x);
}


/* Initilialise the query structure (Q) and the trapezoid table (T)
 * when the first segment is added to start the trapezoidation. The
 * query-tree starts out with 4 trapezoids, one S-node and 2 Y-nodes
 *
 *                4
 *   -----------------------------------
 *                \
 *      1          \        2
 *                  \
 *   -----------------------------------
 *                3
 */

int Triangulator::
init_query_structure(int segnum) {
  int i1, i2, i3, i4, i5, i6, i7, root;
  int t1, t2, t3, t4;
  segment_t *s = &seg[segnum];

  tr.clear();
  qs.clear();

  // We don't use the first elements.
  tr.push_back(trap_t());
  qs.push_back(node_t());

  i1 = newnode();
  qs[i1].nodetype = T_Y;
  _max(&qs[i1].yval, &s->v0, &s->v1); /* root */
  root = i1;

  i2 = newnode();
  qs[i1].right = i2;
  qs[i2].nodetype = T_SINK;
  qs[i2].parent = i1;

  i3 = newnode();
  qs[i1].left = i3;

  qs[i3].nodetype = T_Y;
  _min(&qs[i3].yval, &s->v0, &s->v1); /* root */
  qs[i3].parent = i1;

  i4 = newnode();
  qs[i3].left = i4;
  qs[i4].nodetype = T_SINK;
  qs[i4].parent = i3;

  i5 = newnode();
  qs[i3].right = i5;
  qs[i5].nodetype = T_X;
  qs[i5].segnum = segnum;
  qs[i5].parent = i3;

  i6 = newnode();
  qs[i5].left = i6;
  qs[i6].nodetype = T_SINK;
  qs[i6].parent = i5;

  i7 = newnode();
  qs[i5].right = i7;
  qs[i7].nodetype = T_SINK;
  qs[i7].parent = i5;

  t1 = newtrap();               /* middle left */
  t2 = newtrap();               /* middle right */
  t3 = newtrap();               /* bottom-most */
  t4 = newtrap();               /* topmost */

  tr[t4].lo = qs[i1].yval;
  tr[t2].hi = qs[i1].yval;
  tr[t1].hi = qs[i1].yval;
  tr[t3].hi = qs[i3].yval;
  tr[t2].lo = qs[i3].yval;
  tr[t1].lo = qs[i3].yval;
  tr[t4].hi.y = (double) (REALLY_BIG);
  tr[t4].hi.x = (double) (REALLY_BIG);
  tr[t3].lo.y = (double) -1* (REALLY_BIG);
  tr[t3].lo.x = (double) -1* (REALLY_BIG);
  tr[t2].lseg = segnum;
  tr[t1].rseg = segnum;
  tr[t2].u0 = t4;
  tr[t1].u0 = t4;
  tr[t2].d0 = t3;
  tr[t1].d0 = t3;
  tr[t3].u0 = t1;
  tr[t4].d0 = t1;
  tr[t3].u1 = t2;
  tr[t4].d1 = t2;

  tr[t1].sink = i6;
  tr[t2].sink = i7;
  tr[t3].sink = i4;
  tr[t4].sink = i2;

  tr[t2].state = ST_VALID;
  tr[t1].state = ST_VALID;
  tr[t4].state = ST_VALID;
  tr[t3].state = ST_VALID;

  qs[i2].trnum = t4;
  qs[i4].trnum = t3;
  qs[i6].trnum = t1;
  qs[i7].trnum = t2;

  s->is_inserted = true;
  return root;
}


/* Retun true if the vertex v is to the left of line segment no.
 * segnum. Takes care of the degenerate cases when both the vertices
 * have the same y--cood, etc.
 */

int Triangulator::
is_left_of(int segnum, point_t *v) {
  segment_t *s = &seg[segnum];
  double area;

  if (_greater_than(&s->v1, &s->v0)) /* seg. going upwards */
    {
      if (FP_EQUAL(s->v1.y, v->y))
        {
          if (v->x < s->v1.x)
            area = 1.0;
          else
            area = -1.0;
        }
      else if (FP_EQUAL(s->v0.y, v->y))
        {
          if (v->x < s->v0.x)
            area = 1.0;
          else
            area = -1.0;
        }
      else
        area = CROSS(s->v0, s->v1, (*v));
    }
  else                          /* v0 > v1 */
    {
      if (FP_EQUAL(s->v1.y, v->y))
        {
          if (v->x < s->v1.x)
            area = 1.0;
          else
            area = -1.0;
        }
      else if (FP_EQUAL(s->v0.y, v->y))
        {
          if (v->x < s->v0.x)
            area = 1.0;
          else
            area = -1.0;
        }
      else
        area = CROSS(s->v1, s->v0, (*v));
    }

  if (area > 0.0)
    return true;
  else
    return false;
}



/* Returns true if the corresponding endpoint of the given segment is */
/* already inserted into the segment tree. Use the simple test of */
/* whether the segment which shares this endpoint is already inserted */

int Triangulator::
inserted(int segnum, int whichpt) {
  if (whichpt == FIRSTPT)
    return seg[seg[segnum].prev].is_inserted;
  else
    return seg[seg[segnum].next].is_inserted;
}

/* This is query routine which determines which trapezoid does the
 * point v lie in. The return value is the trapezoid number.
 */

int Triangulator::
locate_endpoint(point_t *v, point_t *vo, int r) {
  // cerr << "locate_endpoint(" << v->x << " " << v->y << ", " << vo->x << " "
  // << vo->y << ", " << r << ")\n";
  node_t *rptr = &qs[r];

  switch (rptr->nodetype)
    {
    case T_SINK:
      return rptr->trnum;

    case T_Y:
      if (_greater_than(v, &rptr->yval)) /* above */
        return locate_endpoint(v, vo, rptr->right);
      else if (_equal_to(v, &rptr->yval)) /* the point is already */
        {                                 /* inserted. */
          if (_greater_than(vo, &rptr->yval)) /* above */
            return locate_endpoint(v, vo, rptr->right);
          else
            return locate_endpoint(v, vo, rptr->left); /* below */
        }
      else
        return locate_endpoint(v, vo, rptr->left); /* below */

    case T_X:
      if (_equal_to(v, &seg[rptr->segnum].v0) ||
               _equal_to(v, &seg[rptr->segnum].v1))
        {
          if (FP_EQUAL(v->y, vo->y)) /* horizontal segment */
            {
              if (vo->x < v->x)
                return locate_endpoint(v, vo, rptr->left); /* left */
              else
                return locate_endpoint(v, vo, rptr->right); /* right */
            }

          else if (is_left_of(rptr->segnum, vo))
            return locate_endpoint(v, vo, rptr->left); /* left */
          else
            return locate_endpoint(v, vo, rptr->right); /* right */
        }
      else if (is_left_of(rptr->segnum, v))
        return locate_endpoint(v, vo, rptr->left); /* left */
      else
        return locate_endpoint(v, vo, rptr->right); /* right */

    default:
      fprintf(stderr, "Haggu !!!!!\n");
      nassertr(false, -1);
      return -1;
    }
}


/* Thread in the segment into the existing trapezoidation. The
 * limiting trapezoids are given by tfirst and tlast (which are the
 * trapezoids containing the two endpoints of the segment. Merges all
 * possible trapezoids which flank this segment and have been recently
 * divided because of its insertion
 */

int Triangulator::
merge_trapezoids(int segnum, int tfirst, int tlast, int side) {
  int t, tnext, cond;
  int ptnext;

  // cerr << "merge_trapezoids(" << segnum << ", " << tfirst << ", " << tlast
  // << ", " << side << ")\n";

  /* First merge polys on the LHS */
  t = tfirst;
  while ((t > 0) && _greater_than_equal_to(&tr[t].lo, &tr[tlast].lo))
    {
      if (side == S_LEFT)
        cond = ((((tnext = tr[t].d0) > 0) && (tr[tnext].rseg == segnum)) ||
                (((tnext = tr[t].d1) > 0) && (tr[tnext].rseg == segnum)));
      else
        cond = ((((tnext = tr[t].d0) > 0) && (tr[tnext].lseg == segnum)) ||
                (((tnext = tr[t].d1) > 0) && (tr[tnext].lseg == segnum)));

      if (cond)
        {
          if ((tr[t].lseg == tr[tnext].lseg) &&
              (tr[t].rseg == tr[tnext].rseg)) /* good neighbours */
            {                                 /* merge them */
              /* Use the upper node as the new node i.e. t */

              ptnext = qs[tr[tnext].sink].parent;

              if (qs[ptnext].left == tr[tnext].sink)
                qs[ptnext].left = tr[t].sink;
              else
                qs[ptnext].right = tr[t].sink;  /* redirect parent */


              /* Change the upper neighbours of the lower trapezoids */

              if ((tr[t].d0 = tr[tnext].d0) > 0) {
                if (tr[tr[t].d0].u0 == tnext) {
                  tr[tr[t].d0].u0 = t;
                } else if (tr[tr[t].d0].u1 == tnext) {
                  tr[tr[t].d0].u1 = t;
                }
              }

              if ((tr[t].d1 = tr[tnext].d1) > 0) {
                if (tr[tr[t].d1].u0 == tnext) {
                  tr[tr[t].d1].u0 = t;
                } else if (tr[tr[t].d1].u1 == tnext) {
                  tr[tr[t].d1].u1 = t;
                }
              }

              tr[t].lo = tr[tnext].lo;
              tr[tnext].state = ST_INVALID; /* invalidate the lower */
                                            /* trapezium */
            }
          else              /* not good neighbours */
            t = tnext;
        }
      else                  /* do not satisfy the outer if */
        t = tnext;

    } /* end-while */

  return 0;
}


/* Add in the new segment into the trapezoidation and update Q and T
 * structures. First locate the two endpoints of the segment in the
 * Q-structure. Then start from the topmost trapezoid and go down to
 * the  lower trapezoid dividing all the trapezoids in between .
 */

int Triangulator::
add_segment(int segnum) {
  // cerr << "add_segment(" << segnum << ")\n";

  segment_t s;
  // segment_t *so = &seg[segnum];
  int tu, tl, sk, tfirst, tlast; //, tnext;
  int tfirstr = 0, tlastr = 0, tfirstl = 0, tlastl = 0;
  int i1, i2, t, tn; // t1, t2,
  point_t tpt;
  int tribot = 0, is_swapped = 0;
  int tmptriseg;

  s = seg[segnum];
  if (_greater_than(&s.v1, &s.v0)) /* Get higher vertex in v0 */
    {
      int tmp;
      tpt = s.v0;
      s.v0 = s.v1;
      s.v1 = tpt;
      tmp = s.root0;
      s.root0 = s.root1;
      s.root1 = tmp;
      is_swapped = true;
    }

  if ((is_swapped) ? !inserted(segnum, LASTPT) :
       !inserted(segnum, FIRSTPT))     /* insert v0 in the tree */
    {
      int tmp_d;

      tu = locate_endpoint(&s.v0, &s.v1, s.root0);
      tl = newtrap();           /* tl is the new lower trapezoid */
      tr[tl].state = ST_VALID;
      tr[tl] = tr[tu];
      tr[tl].hi.y = s.v0.y;
      tr[tu].lo.y = s.v0.y;
      tr[tl].hi.x = s.v0.x;
      tr[tu].lo.x = s.v0.x;
      tr[tu].d0 = tl;
      tr[tu].d1 = 0;
      tr[tl].u0 = tu;
      tr[tl].u1 = 0;

      if (((tmp_d = tr[tl].d0) > 0) && (tr[tmp_d].u0 == tu))
        tr[tmp_d].u0 = tl;
      if (((tmp_d = tr[tl].d0) > 0) && (tr[tmp_d].u1 == tu))
        tr[tmp_d].u1 = tl;

      if (((tmp_d = tr[tl].d1) > 0) && (tr[tmp_d].u0 == tu))
        tr[tmp_d].u0 = tl;
      if (((tmp_d = tr[tl].d1) > 0) && (tr[tmp_d].u1 == tu))
        tr[tmp_d].u1 = tl;

      /* Now update the query structure and obtain the sinks for the */
      /* two trapezoids */

      i1 = newnode();           /* Upper trapezoid sink */
      i2 = newnode();           /* Lower trapezoid sink */
      sk = tr[tu].sink;

      qs[sk].nodetype = T_Y;
      qs[sk].yval = s.v0;
      qs[sk].segnum = segnum;   /* not really reqd ... maybe later */
      qs[sk].left = i2;
      qs[sk].right = i1;

      qs[i1].nodetype = T_SINK;
      qs[i1].trnum = tu;
      qs[i1].parent = sk;

      qs[i2].nodetype = T_SINK;
      qs[i2].trnum = tl;
      qs[i2].parent = sk;

      tr[tu].sink = i1;
      tr[tl].sink = i2;
      tfirst = tl;
    }
  else                          /* v0 already present */
    {       /* Get the topmost intersecting trapezoid */
      tfirst = locate_endpoint(&s.v0, &s.v1, s.root0);
    }


  if ((is_swapped) ? !inserted(segnum, FIRSTPT) :
       !inserted(segnum, LASTPT))     /* insert v1 in the tree */
    {
      int tmp_d;

      tu = locate_endpoint(&s.v1, &s.v0, s.root1);

      tl = newtrap();           /* tl is the new lower trapezoid */
      tr[tl].state = ST_VALID;
      tr[tl] = tr[tu];
      tr[tl].hi.y = s.v1.y;
      tr[tu].lo.y = s.v1.y;
      tr[tl].hi.x = s.v1.x;
      tr[tu].lo.x = s.v1.x;
      tr[tu].d0 = tl;
      tr[tu].d1 = 0;
      tr[tl].u0 = tu;
      tr[tl].u1 = 0;

      if (((tmp_d = tr[tl].d0) > 0) && (tr[tmp_d].u0 == tu))
        tr[tmp_d].u0 = tl;
      if (((tmp_d = tr[tl].d0) > 0) && (tr[tmp_d].u1 == tu))
        tr[tmp_d].u1 = tl;

      if (((tmp_d = tr[tl].d1) > 0) && (tr[tmp_d].u0 == tu))
        tr[tmp_d].u0 = tl;
      if (((tmp_d = tr[tl].d1) > 0) && (tr[tmp_d].u1 == tu))
        tr[tmp_d].u1 = tl;

      /* Now update the query structure and obtain the sinks for the */
      /* two trapezoids */

      i1 = newnode();           /* Upper trapezoid sink */
      i2 = newnode();           /* Lower trapezoid sink */
      sk = tr[tu].sink;

      qs[sk].nodetype = T_Y;
      qs[sk].yval = s.v1;
      qs[sk].segnum = segnum;   /* not really reqd ... maybe later */
      qs[sk].left = i2;
      qs[sk].right = i1;

      qs[i1].nodetype = T_SINK;
      qs[i1].trnum = tu;
      qs[i1].parent = sk;

      qs[i2].nodetype = T_SINK;
      qs[i2].trnum = tl;
      qs[i2].parent = sk;

      tr[tu].sink = i1;
      tr[tl].sink = i2;
      tlast = tu;
    }
  else                          /* v1 already present */
    {       /* Get the lowermost intersecting trapezoid */
      tlast = locate_endpoint(&s.v1, &s.v0, s.root1);
      tribot = 1;
    }

  /* Thread the segment into the query tree creating a new X-node */
  /* First, split all the trapezoids which are intersected by s into */
  /* two */

  t = tfirst;                   /* topmost trapezoid */

  while ((t > 0) &&
         _greater_than_equal_to(&tr[t].lo, &tr[tlast].lo))
                                /* traverse from top to bot */
    {
      int t_sav, tn_sav;
      sk = tr[t].sink;
      i1 = newnode();           /* left trapezoid sink */
      i2 = newnode();           /* right trapezoid sink */

      qs[sk].nodetype = T_X;
      qs[sk].segnum = segnum;
      qs[sk].left = i1;
      qs[sk].right = i2;

      qs[i1].nodetype = T_SINK; /* left trapezoid (use existing one) */
      qs[i1].trnum = t;
      qs[i1].parent = sk;

      qs[i2].nodetype = T_SINK; /* right trapezoid (allocate new) */
      tn = newtrap();
      qs[i2].trnum = tn;
      tr[tn].state = ST_VALID;
      qs[i2].parent = sk;

      if (t == tfirst)
        tfirstr = tn;
      if (_equal_to(&tr[t].lo, &tr[tlast].lo))
        tlastr = tn;

      tr[tn] = tr[t];
      tr[t].sink = i1;
      tr[tn].sink = i2;
      t_sav = t;
      tn_sav = tn;

      /* error */

      if ((tr[t].d0 <= 0) && (tr[t].d1 <= 0)) /* case cannot arise */
        {
          /* Actually, this case does sometimes arise.  Huh. */
          fprintf(stderr, "add_segment: error\n");
          return 1;
        }

      /* only one trapezoid below. partition t into two and make the */
      /* two resulting trapezoids t and tn as the upper neighbours of */
      /* the sole lower trapezoid */

      else if ((tr[t].d0 > 0) && (tr[t].d1 <= 0))
        {                       /* Only one trapezoid below */
          if ((tr[t].u0 > 0) && (tr[t].u1 > 0))
            {                   /* continuation of a chain from abv. */
              if (tr[t].usave > 0) /* three upper neighbours */
                {
                  if (tr[t].uside == S_LEFT)
                    {
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = -1;
                      tr[tn].u1 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                      tr[tr[tn].u1].d0 = tn;
                    }
                  else          /* intersects in the right */
                    {
                      tr[tn].u1 = -1;
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = tr[t].u0;
                      tr[t].u0 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[t].u1].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                    }
                  tr[tn].usave = 0;
                  tr[t].usave = 0;
                }
              else              /* No usave.... simple case */
                {
                  tr[tn].u0 = tr[t].u1;
                  tr[tn].u1 = -1;
                  tr[t].u1 = -1;
                  tr[tr[tn].u0].d0 = tn;
                }
            }
          else
            {                   /* fresh seg. or upward cusp */
              int tmp_u = tr[t].u0;
              int td0, td1;
              if (((td0 = tr[tmp_u].d0) > 0) &&
                  ((td1 = tr[tmp_u].d1) > 0))
                {               /* upward cusp */
                  if ((tr[td0].rseg > 0) &&
                      !is_left_of(tr[td0].rseg, &s.v1))
                    {
                      tr[tn].u1 = -1;
                      tr[t].u1 = -1;
                      tr[t].u0 = -1;
                      tr[tr[tn].u0].d1 = tn;
                    }
                  else          /* cusp going leftwards */
                    {
                      tr[t].u1 = -1;
                      tr[tn].u1 = -1;
                      tr[tn].u0 = -1;
                      tr[tr[t].u0].d0 = t;
                    }
                }
              else              /* fresh segment */
                {
                  tr[tr[t].u0].d0 = t;
                  tr[tr[t].u0].d1 = tn;
                }
            }

          if (FP_EQUAL(tr[t].lo.y, tr[tlast].lo.y) &&
              FP_EQUAL(tr[t].lo.x, tr[tlast].lo.x) && tribot)
            {           /* bottom forms a triangle */

              if (is_swapped)
                tmptriseg = seg[segnum].prev;
              else
                tmptriseg = seg[segnum].next;

              if ((tmptriseg > 0) && is_left_of(tmptriseg, &s.v0))
                {
                                /* L-R downward cusp */
                  tr[tr[t].d0].u0 = t;
                  tr[tn].d1 = -1;
                  tr[tn].d0 = -1;
                }
              else
                {
                                /* R-L downward cusp */
                  tr[tr[tn].d0].u1 = tn;
                  tr[t].d1 = -1;
                  tr[t].d0 = -1;
                }
            }
          else
            {
              if ((tr[tr[t].d0].u0 > 0) && (tr[tr[t].d0].u1 > 0))
                {
                  if (tr[tr[t].d0].u0 == t) /* passes thru LHS */
                    {
                      tr[tr[t].d0].usave = tr[tr[t].d0].u1;
                      tr[tr[t].d0].uside = S_LEFT;
                    }
                  else
                    {
                      tr[tr[t].d0].usave = tr[tr[t].d0].u0;
                      tr[tr[t].d0].uside = S_RIGHT;
                    }
                }
              tr[tr[t].d0].u0 = t;
              tr[tr[t].d0].u1 = tn;
            }

          t = tr[t].d0;
        }


      else if ((tr[t].d0 <= 0) && (tr[t].d1 > 0))
        {                       /* Only one trapezoid below */
          if ((tr[t].u0 > 0) && (tr[t].u1 > 0))
            {                   /* continuation of a chain from abv. */
              if (tr[t].usave > 0) /* three upper neighbours */
                {
                  if (tr[t].uside == S_LEFT)
                    {
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = -1;
                      tr[tn].u1 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                      tr[tr[tn].u1].d0 = tn;
                    }
                  else          /* intersects in the right */
                    {
                      tr[tn].u1 = -1;
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = tr[t].u0;
                      tr[t].u0 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[t].u1].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                    }

                  tr[tn].usave = 0;
                  tr[t].usave = 0;
                }
              else              /* No usave.... simple case */
                {
                  tr[tn].u0 = tr[t].u1;
                  tr[tn].u1 = -1;
                  tr[t].u1 = -1;
                  tr[tr[tn].u0].d0 = tn;
                }
            }
          else
            {                   /* fresh seg. or upward cusp */
              int tmp_u = tr[t].u0;
              int td0, td1;
              if (((td0 = tr[tmp_u].d0) > 0) &&
                  ((td1 = tr[tmp_u].d1) > 0))
                {               /* upward cusp */
                  if ((tr[td0].rseg > 0) &&
                      !is_left_of(tr[td0].rseg, &s.v1))
                    {
                      tr[tn].u1 = -1;
                      tr[t].u1 = -1;
                      tr[t].u0 = -1;
                      tr[tr[tn].u0].d1 = tn;
                    }
                  else
                    {
                      tr[t].u1 = -1;
                      tr[tn].u1 = -1;
                      tr[tn].u0 = -1;
                      tr[tr[t].u0].d0 = t;
                    }
                }
              else              /* fresh segment */
                {
                  tr[tr[t].u0].d0 = t;
                  tr[tr[t].u0].d1 = tn;
                }
            }

          if (FP_EQUAL(tr[t].lo.y, tr[tlast].lo.y) &&
              FP_EQUAL(tr[t].lo.x, tr[tlast].lo.x) && tribot)
            {           /* bottom forms a triangle */
              if (is_swapped)
                tmptriseg = seg[segnum].prev;
              else
                tmptriseg = seg[segnum].next;

              if ((tmptriseg > 0) && is_left_of(tmptriseg, &s.v0))
                {
                  /* L-R downward cusp */
                  tr[tr[t].d1].u0 = t;
                  tr[tn].d1 = -1;
                  tr[tn].d0 = -1;
                }
              else
                {
                  /* R-L downward cusp */
                  tr[tr[tn].d1].u1 = tn;
                  tr[t].d1 = -1;
                  tr[t].d0 = -1;
                }
            }
          else
            {
              if ((tr[tr[t].d1].u0 > 0) && (tr[tr[t].d1].u1 > 0))
                {
                  if (tr[tr[t].d1].u0 == t) /* passes thru LHS */
                    {
                      tr[tr[t].d1].usave = tr[tr[t].d1].u1;
                      tr[tr[t].d1].uside = S_LEFT;
                    }
                  else
                    {
                      tr[tr[t].d1].usave = tr[tr[t].d1].u0;
                      tr[tr[t].d1].uside = S_RIGHT;
                    }
                }
              tr[tr[t].d1].u0 = t;
              tr[tr[t].d1].u1 = tn;
            }

          t = tr[t].d1;
        }

      /* two trapezoids below. Find out which one is intersected by */
      /* this segment and proceed down that one */

      else
        {
          // int tmpseg = tr[tr[t].d0].rseg;
          double y0, yt;
          point_t tmppt;
          int tnext, i_d0;

          i_d0 = false;
          if (FP_EQUAL(tr[t].lo.y, s.v0.y))
            {
              if (tr[t].lo.x > s.v0.x)
                i_d0 = true;
            }
          else
            {
              y0 = tr[t].lo.y;
              tmppt.y = y0;
              yt = (y0 - s.v0.y)/(s.v1.y - s.v0.y);
              tmppt.x = s.v0.x + yt * (s.v1.x - s.v0.x);

              if (_less_than(&tmppt, &tr[t].lo))
                i_d0 = true;
            }

          /* check continuity from the top so that the lower-neighbour */
          /* values are properly filled for the upper trapezoid */

          if ((tr[t].u0 > 0) && (tr[t].u1 > 0))
            {                   /* continuation of a chain from abv. */
              if (tr[t].usave > 0) /* three upper neighbours */
                {
                  if (tr[t].uside == S_LEFT)
                    {
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = -1;
                      tr[tn].u1 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                      tr[tr[tn].u1].d0 = tn;
                    }
                  else          /* intersects in the right */
                    {
                      tr[tn].u1 = -1;
                      tr[tn].u0 = tr[t].u1;
                      tr[t].u1 = tr[t].u0;
                      tr[t].u0 = tr[t].usave;

                      tr[tr[t].u0].d0 = t;
                      tr[tr[t].u1].d0 = t;
                      tr[tr[tn].u0].d0 = tn;
                    }

                  tr[tn].usave = 0;
                  tr[t].usave = 0;
                }
              else              /* No usave.... simple case */
                {
                  tr[tn].u0 = tr[t].u1;
                  tr[tn].u1 = -1;
                  tr[t].u1 = -1;
                  tr[tr[tn].u0].d0 = tn;
                }
            }
          else
            {                   /* fresh seg. or upward cusp */
              int tmp_u = tr[t].u0;
              int td0, td1;
              if (((td0 = tr[tmp_u].d0) > 0) &&
                  ((td1 = tr[tmp_u].d1) > 0))
                {               /* upward cusp */
                  if ((tr[td0].rseg > 0) &&
                      !is_left_of(tr[td0].rseg, &s.v1))
                    {
                      tr[tn].u1 = -1;
                      tr[t].u1 = -1;
                      tr[t].u0 = -1;
                      tr[tr[tn].u0].d1 = tn;
                    }
                  else
                    {
                      tr[t].u1 = -1;
                      tr[tn].u1 = -1;
                      tr[tn].u0 = -1;
                      tr[tr[t].u0].d0 = t;
                    }
                }
              else              /* fresh segment */
                {
                  tr[tr[t].u0].d0 = t;
                  tr[tr[t].u0].d1 = tn;
                }
            }

          if (FP_EQUAL(tr[t].lo.y, tr[tlast].lo.y) &&
              FP_EQUAL(tr[t].lo.x, tr[tlast].lo.x) && tribot)
            {
              /* this case arises only at the lowest trapezoid.. i.e.
                 tlast, if the lower endpoint of the segment is
                 already inserted in the structure */

              tr[tr[t].d0].u0 = t;
              tr[tr[t].d0].u1 = -1;
              tr[tr[t].d1].u0 = tn;
              tr[tr[t].d1].u1 = -1;

              tr[tn].d0 = tr[t].d1;
              tr[tn].d1 = -1;
              tr[t].d1 = -1;

              tnext = tr[t].d1;
            }
          else if (i_d0)
                                /* intersecting d0 */
            {
              tr[tr[t].d0].u0 = t;
              tr[tr[t].d0].u1 = tn;
              tr[tr[t].d1].u0 = tn;
              tr[tr[t].d1].u1 = -1;

              /* new code to determine the bottom neighbours of the */
              /* newly partitioned trapezoid */

              tr[t].d1 = -1;

              tnext = tr[t].d0;
            }
          else                  /* intersecting d1 */
            {
              tr[tr[t].d0].u0 = t;
              tr[tr[t].d0].u1 = -1;
              tr[tr[t].d1].u0 = t;
              tr[tr[t].d1].u1 = tn;

              /* new code to determine the bottom neighbours of the */
              /* newly partitioned trapezoid */

              tr[tn].d0 = tr[t].d1;
              tr[tn].d1 = -1;

              tnext = tr[t].d1;
            }

          t = tnext;
        }

      tr[tn_sav].lseg  = segnum;
      tr[t_sav].rseg = segnum;
    } /* end-while */

  /* Now combine those trapezoids which share common segments. We can */
  /* use the pointers to the parent to connect these together. This */
  /* works only because all these new trapezoids have been formed */
  /* due to splitting by the segment, and hence have only one parent */

  tfirstl = tfirst;
  tlastl = tlast;
  merge_trapezoids(segnum, tfirstl, tlastl, S_LEFT);
  merge_trapezoids(segnum, tfirstr, tlastr, S_RIGHT);

  seg[segnum].is_inserted = true;
  return 0;
}


/* Update the roots stored for each of the endpoints of the segment.
 * This is done to speed up the location-query for the endpoint when
 * the segment is inserted into the trapezoidation subsequently
 */
int Triangulator::
find_new_roots(int segnum) {
  // cerr << "find_new_roots(" << segnum << ")\n";
  segment_t *s = &seg[segnum];

  if (s->is_inserted)
    return 0;

  s->root0 = locate_endpoint(&s->v0, &s->v1, s->root0);
  s->root0 = tr[s->root0].sink;

  s->root1 = locate_endpoint(&s->v1, &s->v0, s->root1);
  s->root1 = tr[s->root1].sink;
  return 0;
}


/* Main routine to perform trapezoidation */
int Triangulator::
construct_trapezoids(int nseg) {
  // cerr << "construct_trapezoids(" << nseg << ")\n";
  int i;
  int root, h;

  /* Add the first segment and get the query structure and trapezoid */
  /* list initialised */

  root = init_query_structure(choose_segment());

  for (i = 1; i <= nseg; i++) {
    seg[i].root1 = root;
    seg[i].root0 = root;
  }

  for (h = 1; h <= math_logstar_n(nseg); h++)
    {
      for (i = math_N(nseg, h -1) + 1; i <= math_N(nseg, h); i++) {
        if (add_segment(choose_segment()) != 0) {
          // error in add_segment.
          return 1;
        }
      }

      /* Find a new root for each of the segment endpoints */
      for (i = 1; i <= nseg; i++)
        find_new_roots(i);
    }

  for (i = math_N(nseg, math_logstar_n(nseg)) + 1; i <= nseg; i++)
    add_segment(choose_segment());

  return 0;
}



/* Function returns true if the trapezoid lies inside the polygon */
int Triangulator::
inside_polygon(trap_t *t) {
  int rseg = t->rseg;

  if (t->state == ST_INVALID)
    return 0;

  if ((t->lseg <= 0) || (t->rseg <= 0))
    return 0;

  if (((t->u0 <= 0) && (t->u1 <= 0)) ||
      ((t->d0 <= 0) && (t->d1 <= 0))) /* triangle */
    return (_greater_than(&seg[rseg].v1, &seg[rseg].v0));

  return 0;
}


/* return a new mon structure from the table */
int Triangulator::
newmon() {
  int index = (int)mon.size();
  mon.push_back(0);
  // cerr << "newmon " << index << "\n";
  return index;
}


/* return a new chain element from the table */
int Triangulator::
new_chain_element() {
  int index = (int)mchain.size();
  mchain.push_back(monchain_t());
  // cerr << "new_chain_element " << index << "\n";
  return index;
}


double Triangulator::
get_angle(point_t *vp0, point_t *vpnext, point_t *vp1) {
  point_t v0, v1;

  v0.x = vpnext->x - vp0->x;
  v0.y = vpnext->y - vp0->y;

  v1.x = vp1->x - vp0->x;
  v1.y = vp1->y - vp0->y;

  if (CROSS_SINE(v0, v1) >= 0)  /* sine is positive */
    return DOT(v0, v1)/LENGTH(v0)/LENGTH(v1);
  else
    return (-1.0 * DOT(v0, v1)/LENGTH(v0)/LENGTH(v1) - 2);
}


/* (v0, v1) is the new diagonal to be added to the polygon. Find which */
/* chain to use and return the positions of v0 and v1 in p and q */
int Triangulator::
get_vertex_positions(int v0, int v1, int *ip, int *iq) {
  vertexchain_t *vp0, *vp1;
  int i;
  double angle, temp;
  int tp = 0, tq = 0;

  vp0 = &vert[v0];
  vp1 = &vert[v1];

  /* p is identified as follows. Scan from (v0, v1) rightwards till */
  /* you hit the first segment starting from v0. That chain is the */
  /* chain of our interest */

  angle = -4.0;
  for (i = 0; i < 4; i++)
    {
      if (vp0->vnext[i] <= 0)
        continue;
      if ((temp = get_angle(&vp0->pt, &(vert[vp0->vnext[i]].pt),
                            &vp1->pt)) > angle)
        {
          angle = temp;
          tp = i;
        }
    }

  *ip = tp;

  /* Do similar actions for q */

  angle = -4.0;
  for (i = 0; i < 4; i++)
    {
      if (vp1->vnext[i] <= 0)
        continue;
      if ((temp = get_angle(&vp1->pt, &(vert[vp1->vnext[i]].pt),
                            &vp0->pt)) > angle)
        {
          angle = temp;
          tq = i;
        }
    }

  *iq = tq;

  return 0;
}


/* v0 and v1 are specified in anti-clockwise order with respect to
 * the current monotone polygon mcur. Split the current polygon into
 * two polygons using the diagonal (v0, v1)
 */
int Triangulator::
make_new_monotone_poly(int mcur, int v0, int v1) {
  int p, q, ip, iq;
  int mnew = newmon();
  int i, j, nf0, nf1;
  vertexchain_t *vp0, *vp1;

  if (v0 <= 0 || v1 <= 0) {
    return -1;
  }

  vp0 = &vert[v0];
  vp1 = &vert[v1];

  get_vertex_positions(v0, v1, &ip, &iq);

  p = vp0->vpos[ip];
  q = vp1->vpos[iq];

  /* At this stage, we have got the positions of v0 and v1 in the */
  /* desired chain. Now modify the linked lists */

  i = new_chain_element();      /* for the new list */
  j = new_chain_element();

  mchain[i].vnum = v0;
  mchain[j].vnum = v1;

  mchain[i].next = mchain[p].next;
  mchain[mchain[p].next].prev = i;
  mchain[i].prev = j;
  mchain[j].next = i;
  mchain[j].prev = mchain[q].prev;
  mchain[mchain[q].prev].next = j;

  mchain[p].next = q;
  mchain[q].prev = p;

  nf0 = vp0->nextfree;
  nf1 = vp1->nextfree;

  vp0->vnext[ip] = v1;

  vp0->vpos[nf0] = i;
  vp0->vnext[nf0] = mchain[mchain[i].next].vnum;
  vp1->vpos[nf1] = j;
  vp1->vnext[nf1] = v0;

  vp0->nextfree++;
  vp1->nextfree++;

#ifdef DEBUG
  fprintf(stderr, "make_poly: mcur = %d, (v0, v1) = (%d, %d)\n",
          mcur, v0, v1);
  fprintf(stderr, "next posns = (p, q) = (%d, %d)\n", p, q);
#endif

  mon[mcur] = p;
  mon[mnew] = i;
  return mnew;
}

/* Main routine to get monotone polygons from the trapezoidation of
 * the polygon.
 */

int Triangulator::
monotonate_trapezoids(int n) {
  int i;
  int tr_start;

  vert.clear();
  visited.clear();
  mchain.clear();
  mon.clear();

  vert.insert(vert.begin(), n + 1, vertexchain_t());
  mchain.insert(mchain.begin(), n + 1, monchain_t());

  visited.insert(visited.begin(), tr.size(), 0);

  /* First locate a trapezoid which lies inside the polygon */
  /* and which is triangular */
  for (i = 1; i < (int)tr.size(); i++)
    if (inside_polygon(&tr[i]))
      break;
  if (i >= (int)tr.size()) {
    // No valid trapezoids.
    return 0;
  }
  // printf("start = %d\n", i);
  tr_start = i;

  /* Initialise the mon data-structure and start spanning all the */
  /* trapezoids within the polygon */

#if 0
  for (i = 1; i <= n; i++)
    {
      mchain[i].prev = i - 1;
      mchain[i].next = i + 1;
      mchain[i].vnum = i;
      vert[i].pt = seg[i].v0;
      vert[i].vnext[0] = i + 1; /* next vertex */
      vert[i].vpos[0] = i;      /* locn. of next vertex */
      vert[i].nextfree = 1;
      vert[i].user_i = seg[i].v0_i;
    }
  mchain[1].prev = n;
  mchain[n].next = 1;
  vert[n].vnext[0] = 1;
  vert[n].vpos[0] = n;
  mon.push_back(1); /* position of any vertex in the first */
                    /* chain  */

#else

  for (i = 1; i <= n; i++)
    {
      mchain[i].prev = seg[i].prev;
      mchain[i].next = seg[i].next;
      mchain[i].vnum = i;
      vert[i].pt = seg[i].v0;
      vert[i].vnext[0] = seg[i].next; /* next vertex */
      vert[i].vpos[0] = i;      /* locn. of next vertex */
      vert[i].nextfree = 1;
      vert[i].user_i = seg[i].v0_i;
    }

  mon.push_back(1); /* position of any vertex in the first */
                    /* chain  */

#endif

  /* traverse the polygon */
  if (tr[tr_start].u0 > 0)
    traverse_polygon(0, tr_start, tr[tr_start].u0, TR_FROM_UP);
  else if (tr[tr_start].d0 > 0)
    traverse_polygon(0, tr_start, tr[tr_start].d0, TR_FROM_DN);

  /* return the number of polygons created */
  return newmon();
}


/* recursively visit all the trapezoids */
int Triangulator::
traverse_polygon(int mcur, int trnum, int from, int dir) {
  // printf("traverse_polygon(%d, %d, %d, %d)\n", mcur, trnum, from, dir);

  if (mcur < 0 || trnum <= 0)
    return 0;

  if (visited[trnum])
    return 0;

  trap_t *t = &tr[trnum];
  // int howsplit;
  int mnew;
  int v0, v1;  //, v0next, v1next;
  int retval = 0;  //, tmp;

  // printf("visited size = %d, visited[trnum] = %d\n", visited.size(),
  // visited[trnum]);

  visited[trnum] = true;

  /* We have much more information available here. */
  /* rseg: goes upwards   */
  /* lseg: goes downwards */

  /* Initially assume that dir = TR_FROM_DN (from the left) */
  /* Switch v0 and v1 if necessary afterwards */


  /* special cases for triangles with cusps at the opposite ends. */
  /* take care of this first */
  if ((t->u0 <= 0) && (t->u1 <= 0))
    {
      if ((t->d0 > 0) && (t->d1 > 0)) /* downward opening triangle */
        {
          v0 = tr[t->d1].lseg;
          v1 = t->lseg;
          if (from == t->d1)
            {
              mnew = make_new_monotone_poly(mcur, v1, v0);
              traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
              traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
            }
          else
            {
              mnew = make_new_monotone_poly(mcur, v0, v1);
              traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
              traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
            }
        }
      else
        {
          retval = SP_NOSPLIT;  /* Just traverse all neighbours */
          traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
          traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
          traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
          traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
        }
    }

  else if ((t->d0 <= 0) && (t->d1 <= 0))
    {
      if ((t->u0 > 0) && (t->u1 > 0)) /* upward opening triangle */
        {
          v0 = t->rseg;
          v1 = tr[t->u0].rseg;
          if (from == t->u1)
            {
              mnew = make_new_monotone_poly(mcur, v1, v0);
              traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
              traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
            }
          else
            {
              mnew = make_new_monotone_poly(mcur, v0, v1);
              traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
              traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
            }
        }
      else
        {
          retval = SP_NOSPLIT;  /* Just traverse all neighbours */
          traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
          traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
          traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
          traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
        }
    }

  else if ((t->u0 > 0) && (t->u1 > 0))
    {
      if ((t->d0 > 0) && (t->d1 > 0)) /* downward + upward cusps */
        {
          v0 = tr[t->d1].lseg;
          v1 = tr[t->u0].rseg;
          retval = SP_2UP_2DN;
          if (((dir == TR_FROM_DN) && (t->d1 == from)) ||
              ((dir == TR_FROM_UP) && (t->u1 == from)))
            {
              mnew = make_new_monotone_poly(mcur, v1, v0);
              traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
              traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
              traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
              traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
            }
          else
            {
              mnew = make_new_monotone_poly(mcur, v0, v1);
              traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
              traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
              traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
              traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
            }
        }
      else                      /* only downward cusp */
        {
          if (_equal_to(&t->lo, &seg[t->lseg].v1))
            {
              v0 = tr[t->u0].rseg;
              v1 = seg[t->lseg].next;

              retval = SP_2UP_LEFT;
              if ((dir == TR_FROM_UP) && (t->u0 == from))
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                }
            }
          else
            {
              v0 = t->rseg;
              v1 = tr[t->u0].rseg;
              retval = SP_2UP_RIGHT;
              if ((dir == TR_FROM_UP) && (t->u1 == from))
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                }
            }
        }
    }
  else if ((t->u0 > 0) || (t->u1 > 0)) /* no downward cusp */
    {
      if ((t->d0 > 0) && (t->d1 > 0)) /* only upward cusp */
        {
          if (_equal_to(&t->hi, &seg[t->lseg].v0))
            {
              v0 = tr[t->d1].lseg;
              v1 = t->lseg;
              retval = SP_2DN_LEFT;
              if (!((dir == TR_FROM_DN) && (t->d0 == from)))
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                }
            }
          else
            {
              v0 = tr[t->d1].lseg;
              v1 = seg[t->rseg].next;

              retval = SP_2DN_RIGHT;
              if ((dir == TR_FROM_DN) && (t->d1 == from))
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                }
            }
        }
      else                      /* no cusp */
        {
          if (_equal_to(&t->hi, &seg[t->lseg].v0) &&
              _equal_to(&t->lo, &seg[t->rseg].v0))
            {
              v0 = t->rseg;
              v1 = t->lseg;
              retval = SP_SIMPLE_LRDN;
              if (dir == TR_FROM_UP)
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                }
            }
          else if (_equal_to(&t->hi, &seg[t->rseg].v1) &&
                   _equal_to(&t->lo, &seg[t->lseg].v1))
            {
              v0 = seg[t->rseg].next;
              v1 = seg[t->lseg].next;

              retval = SP_SIMPLE_LRUP;
              if (dir == TR_FROM_UP)
                {
                  mnew = make_new_monotone_poly(mcur, v1, v0);
                  traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->d0, trnum, TR_FROM_UP);
                }
              else
                {
                  mnew = make_new_monotone_poly(mcur, v0, v1);
                  traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
                  traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
                  traverse_polygon(mnew, t->u0, trnum, TR_FROM_DN);
                  traverse_polygon(mnew, t->u1, trnum, TR_FROM_DN);
                }
            }
          else                  /* no split possible */
            {
              retval = SP_NOSPLIT;
              traverse_polygon(mcur, t->u0, trnum, TR_FROM_DN);
              traverse_polygon(mcur, t->d0, trnum, TR_FROM_UP);
              traverse_polygon(mcur, t->u1, trnum, TR_FROM_DN);
              traverse_polygon(mcur, t->d1, trnum, TR_FROM_UP);
            }
        }
    }

  return retval;
}


/* For each monotone polygon, find the ymax and ymin (to determine the */
/* two y-monotone chains) and pass on this monotone polygon for greedy */
/* triangulation. */
/* Take care not to triangulate duplicate monotone polygons */

void Triangulator::
triangulate_monotone_polygons(int nvert, int nmonpoly) {
  int i;
  point_t ymax, ymin;
  int p, vfirst, posmax, posmin, v;
  int vcount, processed;

  #ifdef DEBUG
  for (i = 0; i < nmonpoly; i++)
    {
      fprintf(stderr, "\n\nPolygon %d: ", i);
      vfirst = mchain[mon[i]].vnum;
      p = mchain[mon[i]].next;
      fprintf (stderr, "%d ", mchain[mon[i]].vnum);
      while (mchain[p].vnum != vfirst)
        {
          fprintf(stderr, "%d ", mchain[p].vnum);
          if (mchain[p].vnum == -1) {
            fprintf(stderr, " xx");
            break;
          }
          p = mchain[p].next;
        }
    }
  fprintf(stderr, "\n");
  #endif

  for (i = 0; i < nmonpoly; i++)
    {
      vcount = 1;
      processed = false;
      vfirst = mchain[mon[i]].vnum;
      if (vfirst <= 0) {
        return;
      }
      ymin = vert[vfirst].pt;
      ymax = ymin;
      posmin = mon[i];
      posmax = posmin;
      mchain[mon[i]].marked = true;
      p = mchain[mon[i]].next;
      while ((v = mchain[p].vnum) != vfirst)
        {
          if (v <= 0) {
            return;
          }
          if (mchain[p].marked)
           {
             processed = true;
             break;             /* break from while */
           }
          else
            mchain[p].marked = true;

          if (_greater_than(&vert[v].pt, &ymax))
            {
              ymax = vert[v].pt;
              posmax = p;
            }
          if (_less_than(&vert[v].pt, &ymin))
            {
              ymin = vert[v].pt;
              posmin = p;
            }
          p = mchain[p].next;
          vcount++;
       }

      if (processed)            /* Go to next polygon */
        continue;

      if (vcount == 3)          /* already a triangle */
        {
          _result.push_back(Triangle(this, mchain[p].vnum,
                                     mchain[mchain[p].next].vnum,
                                     mchain[mchain[p].prev].vnum));
        }
      else                      /* triangulate the polygon */
        {
          v = mchain[mchain[posmax].next].vnum;
          if (_equal_to(&vert[v].pt, &ymin))
            {                   /* LHS is a single line */
              triangulate_single_polygon(nvert, posmax, TRI_LHS);
            }
          else
            triangulate_single_polygon(nvert, posmax, TRI_RHS);
        }
    }
}


/* A greedy corner-cutting algorithm to triangulate a y-monotone
 * polygon in O(n) time.
 * Joseph O-Rourke, Computational Geometry in C.
 */
void Triangulator::
triangulate_single_polygon(int nvert, int posmax, int side) {
  int v;
  vector_int rc;        /* reflex chain */
  int ri;
  int endv, tmp, vpos;

  // cerr << "triangulate_single_polygon(" << nvert << ", " << posmax << ", "
  // << side << ")\n";

  if (side == TRI_RHS)          /* RHS segment is a single segment */
    {
      rc.push_back(mchain[posmax].vnum);
      tmp = mchain[posmax].next;
      rc.push_back(mchain[tmp].vnum);
      ri = 1;

      vpos = mchain[tmp].next;
      v = mchain[vpos].vnum;

      if ((endv = mchain[mchain[posmax].prev].vnum) == 0)
        endv = nvert;
    }
  else                          /* LHS is a single segment */
    {
      tmp = mchain[posmax].next;
      rc.push_back(mchain[tmp].vnum);
      tmp = mchain[tmp].next;
      rc.push_back(mchain[tmp].vnum);
      ri = 1;

      vpos = mchain[tmp].next;
      v = mchain[vpos].vnum;

      endv = mchain[posmax].vnum;
    }

  int num_triangles = 0;

  while ((v != endv) || (ri > 1))
    {
      // cerr << " v = " << v << " ri = " << ri << " rc = " << rc.size() << "
      // _result = " << _result.size() << "\n";
      if (v <= 0) {
        // Something went wrong.
        return;
      }
      if (ri > 0)               /* reflex chain is non-empty */
        {
          PN_stdfloat crossResult = CROSS(vert[v].pt, vert[rc[ri - 1]].pt,
                                    vert[rc[ri]].pt);
          if ( crossResult >= 0 )  /* could be convex corner or straight */
            {
              if (crossResult > 0) {  /* convex corner: cut it off */
                _result.push_back(Triangle(this, rc[ri - 1], rc[ri], v));
                if (++num_triangles >= nvert - 2) {
                  // We can't generate more than this number of triangles.
                  return;
                }
              }
              /* else : perfectly straight, will be abandoned anyway */
              ri--;
              rc.pop_back();
              nassertv(ri + 1 == (int)rc.size());
            }
          else /* concave, add v to the chain */
            {
              ri++;
              rc.push_back(v);
              nassertv(ri + 1 == (int)rc.size());
              vpos = mchain[vpos].next;
              v = mchain[vpos].vnum;
            }
        }
      else                      /* reflex-chain empty: add v to the */
        {                       /* reflex chain and advance it  */
          ri++;
          rc.push_back(v);
          nassertv(ri + 1 == (int)rc.size());
          vpos = mchain[vpos].next;
          v = mchain[vpos].vnum;
        }
    } /* end-while */

  /* reached the bottom vertex. Add in the triangle formed */
  _result.push_back(Triangle(this, rc[ri - 1], rc[ri], v));
  ri--;
}
