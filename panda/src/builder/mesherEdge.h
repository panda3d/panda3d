// Filename: mesherEdge.h
// Created by:  drose (15Sep97)
//
////////////////////////////////////////////////////////////////////

#ifndef MESHEREDGE_H
#define MESHEREDGE_H

#include <pandabase.h>

#include "mesherConfig.h"
#include "builderBucket.h"

#include <list>
#include <math.h>


template <class PrimType>
class MesherStrip;

template <class PrimType>
class MesherEdge {
public:
  typedef PrimType Prim;
  typedef TYPENAME PrimType::Vertex Vertex;
  typedef MesherStrip<PrimType> Strip;

  INLINE MesherEdge(const Vertex *a, const Vertex *b);
  INLINE MesherEdge(const MesherEdge &copy);

  void remove(Strip *strip);
  void change_strip(Strip *from, Strip *to);

  INLINE bool contains_vertex(const Vertex *v) const;

  INLINE bool matches(const MesherEdge &other) const;

  INLINE MesherEdge *common_ptr();

  INLINE bool operator == (const MesherEdge &other) const;
  INLINE bool operator != (const MesherEdge &other) const;
  INLINE bool operator < (const MesherEdge &other) const;

  INLINE float compute_length(const BuilderBucket &bucket) const;
  INLINE Vertexf compute_box(const BuilderBucket &bucket) const;

  ostream &output(ostream &out) const;

  const Vertex *_a, *_b;

  typedef list<Strip *> Strips;
  Strips _strips;
  MesherEdge *_opposite;
};

template <class PrimType>
INLINE ostream &operator << (ostream &out,
			     const MesherEdge<PrimType> &edge) {
  return edge.output(out);
}

#include "mesherEdge.I"

#endif
