// Filename: mesherStrip.h
// Created by:  drose (16Sep97)
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

#ifndef MESHERSTRIP_H
#define MESHERSTRIP_H

#include "pandabase.h"

#include "mesherConfig.h"
#include "builderTypes.h"
#include "builderBucket.h"
#include "config_builder.h"

#include "plist.h"
#include <math.h>


template <class PrimType>
class MesherEdge;

template <class PrimType>
class MesherTempl;

enum MesherStripStatus {
  MS_alive, MS_dead, MS_done, MS_paired
};

enum MesherStripOrigin {
  MO_unknown, MO_user, MO_firstquad, MO_fanpoly, MO_mate
};

template <class PrimType>
class MesherStrip {
public:
  typedef PrimType Prim;
  typedef TYPENAME PrimType::Vertex Vertex;
  typedef TYPENAME PrimType::DAttrib SAttrib;
  typedef MesherEdge<PrimType> Edge;

  MesherStrip() {}
  MesherStrip(const PrimType &prim, int index, const BuilderBucket &bucket);
  INLINE MesherStrip(const MesherStrip &copy);

  Prim make_prim(const BuilderBucket &bucket);

  void measure_sheet(const Edge *edge, int new_row,
                    int &num_prims, int &num_rows,
                    int first_row_id, int this_row_id,
                    int this_row_distance);
  void cut_sheet(int first_row_id, int do_mate,
                 const BuilderBucket &bucket);

  bool mate(const BuilderBucket &bucket);
  bool find_ideal_mate(MesherStrip *&mate, Edge *&common_edge,
                       const BuilderBucket &bucket);
  static bool mate_pieces(Edge *common_edge, MesherStrip &front,
                          MesherStrip &back, const BuilderBucket &bucket);
  static bool mate_strips(Edge *common_edge, MesherStrip &front,
                          MesherStrip &back, BuilderPrimType type);
  static bool must_invert(const MesherStrip &front, const MesherStrip &back,
                          bool will_reverse_back, BuilderPrimType type);
  static bool convex_quad(Edge *common_edge, MesherStrip &front,
                          MesherStrip &back, const BuilderBucket &bucket);

  int count_neighbors() const;
  ostream &show_neighbors(ostream &out) const;

  INLINE bool is_coplanar_with(const MesherStrip &other, float threshold) const;
  INLINE float coplanarity(const MesherStrip &other) const;
  INLINE int type_category() const;

  const Vertex *find_uncommon_vertex(const Edge *edge) const;
  const Edge *find_opposite_edge(const Vertex *vertex) const;
  const Edge *find_opposite_edge(const Edge *edge) const;
  const Edge *find_adjacent_edge(const Edge *edge) const;

  INLINE void rotate_forward();
  INLINE void rotate_back();
  void rotate_to_front(const Edge *edge);
  void rotate_to_back(const Edge *edge);
  bool can_invert() const;
  bool invert();

  INLINE Edge get_head_edge() const;
  INLINE Edge get_tail_edge() const;

  bool is_odd() const;
  bool would_reverse_tail(BuilderPrimType wantType) const;
  void convert_to_type(BuilderPrimType wantType);

  void combine_edges(MesherStrip<PrimType> &other, int removeSides);
  void remove_all_edges();

  // ptr equality
  INLINE bool operator == (const MesherStrip &other) const;
  INLINE bool operator != (const MesherStrip &other) const;

  bool pick_mate(const MesherStrip &a_strip, const MesherStrip &b_strip,
                 const Edge &a_edge, const Edge &b_edge,
                 const BuilderBucket &bucket) const;

  bool pick_sheet_mate(const MesherStrip &a_strip,
                       const MesherStrip &b_strip) const;

  ostream &output(ostream &out) const;

  typedef plist<SAttrib> Prims;
  typedef plist<Edge *> Edges;
  typedef plist<const Vertex *> Verts;

  Prims _prims;
  Edges _edges;
  Verts _verts;

  BuilderPrimType _type;
  int _index;
  MesherStripStatus _status;

  int _planar;
  LVector3f _plane_normal;
  float _plane_offset;
  int _row_id, _row_distance;
  int _origin;
};

template <class PrimType>
INLINE ostream &operator << (ostream &out,
                             const MesherStrip<PrimType> &strip) {
  return strip.output(out);
}

#include "mesherStrip.I"

#endif
