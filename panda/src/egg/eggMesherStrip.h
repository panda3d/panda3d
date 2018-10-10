/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMesherStrip.h
 * @author drose
 * @date 2005-03-13
 */

#ifndef EGGMESHERSTRIP_H
#define EGGMESHERSTRIP_H

#include "pandabase.h"
#include "eggVertexPool.h"
#include "eggPrimitive.h"
#include "eggMesherEdge.h"
#include "plist.h"

class EggMesherEdge;

/**
 * Represents a triangle strip or quad strip in progress, as assembled by the
 * mesher.  It might also represent a single polygon such as a triangle or
 * quad, since that's how strips generally start out.
 */
class EXPCL_PANDA_EGG EggMesherStrip {
public:
  enum PrimType {
    PT_poly,
    PT_point,
    PT_line,
    PT_tri,
    PT_tristrip,
    PT_trifan,
    PT_quad,
    PT_quadstrip,
    PT_linestrip,
  };

  enum MesherOrigin {
    MO_unknown,
    MO_user,
    MO_firstquad,
    MO_fanpoly,
    MO_mate
  };

  EggMesherStrip(PrimType prim_type, MesherOrigin origin);
  EggMesherStrip(const EggPrimitive *prim, int index, const EggVertexPool *vertex_pool,
                 bool flat_shaded);
  INLINE EggMesherStrip(const EggMesherStrip &copy);

  PT(EggPrimitive) make_prim(const EggVertexPool *vertex_pool);

  void measure_sheet(const EggMesherEdge *edge, int new_row,
                    int &num_prims, int &num_rows,
                    int first_row_id, int this_row_id,
                    int this_row_distance);
  void cut_sheet(int first_row_id, int do_mate,
                 const EggVertexPool *vertex_pool);

  bool mate(const EggVertexPool *vertex_pool);
  bool find_ideal_mate(EggMesherStrip *&mate, EggMesherEdge *&common_edge,
                       const EggVertexPool *vertex_pool);
  static bool mate_pieces(EggMesherEdge *common_edge, EggMesherStrip &front,
                          EggMesherStrip &back, const EggVertexPool *vertex_pool);
  static bool mate_strips(EggMesherEdge *common_edge, EggMesherStrip &front,
                          EggMesherStrip &back, PrimType type);
  static bool must_invert(const EggMesherStrip &front, const EggMesherStrip &back,
                          bool will_reverse_back, PrimType type);
  static bool convex_quad(EggMesherEdge *common_edge, EggMesherStrip &front,
                          EggMesherStrip &back, const EggVertexPool *vertex_pool);

  int count_neighbors() const;
  void output_neighbors(std::ostream &out) const;

  INLINE bool is_coplanar_with(const EggMesherStrip &other, PN_stdfloat threshold) const;
  INLINE PN_stdfloat coplanarity(const EggMesherStrip &other) const;
  INLINE int type_category() const;

  int find_uncommon_vertex(const EggMesherEdge *edge) const;
  const EggMesherEdge *find_opposite_edge(int vi) const;
  const EggMesherEdge *find_opposite_edge(const EggMesherEdge *edge) const;
  const EggMesherEdge *find_adjacent_edge(const EggMesherEdge *edge) const;

  INLINE void rotate_forward();
  INLINE void rotate_back();
  void rotate_to_front(const EggMesherEdge *edge);
  void rotate_to_back(const EggMesherEdge *edge);
  bool can_invert() const;
  bool invert();

  INLINE EggMesherEdge get_head_edge() const;
  INLINE EggMesherEdge get_tail_edge() const;

  bool is_odd() const;
  bool would_reverse_tail(PrimType want_type) const;
  void convert_to_type(PrimType want_type);

  void combine_edges(EggMesherStrip &other, int remove_sides);
  void remove_all_edges();

  // ptr equality
  INLINE bool operator == (const EggMesherStrip &other) const;
  INLINE bool operator != (const EggMesherStrip &other) const;

  bool pick_mate(const EggMesherStrip &a_strip, const EggMesherStrip &b_strip,
                 const EggMesherEdge &a_edge, const EggMesherEdge &b_edge,
                 const EggVertexPool *vertex_pool) const;

  bool pick_sheet_mate(const EggMesherStrip &a_strip,
                       const EggMesherStrip &b_strip) const;

  void output(std::ostream &out) const;

  typedef plist<CPT(EggPrimitive) > Prims;
  typedef plist<EggMesherEdge *> Edges;
  typedef plist<int> Verts;

  Prims _prims;
  Edges _edges;
  Verts _verts;

  enum MesherStatus {
    MS_alive,
    MS_dead,
    MS_done,
    MS_paired
  };

  PrimType _type;
  int _index;
  MesherStatus _status;

  bool _planar;
  LNormald _plane_normal;
  PN_stdfloat _plane_offset;
  int _row_id, _row_distance;
  MesherOrigin _origin;
  bool _flat_shaded;
};

INLINE std::ostream &
operator << (std::ostream &out, const EggMesherStrip &strip) {
  strip.output(out);
  return out;
}

#include "eggMesherStrip.I"

#endif
