// Filename: mesherFanMaker.h
// Created by:  drose (21Sep97)
//
////////////////////////////////////////////////////////////////////

#ifndef MESHERFANMAKER_H
#define MESHERFANMAKER_H

#ifdef SUPPORT_FANS

#include <pandabase.h>

#include "builderAttrib.h"
#include "builderVertex.h"
#include "builderBucket.h"
#include "mesherEdge.h"
#include "mesherStrip.h"

#include <list>


template <class PrimType>
class MesherTempl;

template <class PrimType>
class MesherFanMaker {
public:
  typedef PrimType Prim;
  typedef TYPENAME PrimType::Vertex Vertex;
  typedef TYPENAME PrimType::DAttrib FAttrib;
  typedef MesherEdge<PrimType> Edge;
  typedef MesherStrip<PrimType> Strip;
  typedef MesherTempl<PrimType> Mesher;

  typedef list<const Edge *> Edges;
  typedef list<Strip *> Strips;

  MesherFanMaker() {}
  MesherFanMaker(const Vertex *vertex, Strip *tri, Mesher *mesher);

  INLINE bool operator < (const MesherFanMaker &other) const;
  INLINE bool operator != (const MesherFanMaker &other) const;
  INLINE bool operator == (const MesherFanMaker &other) const;

  INLINE bool is_empty() const;
  INLINE bool is_valid() const;
  INLINE bool is_coplanar_with(const MesherFanMaker &other) const;

  bool join(MesherFanMaker &other);
  float compute_angle() const;

  int build();
  int unroll(Strips::iterator strip_begin, Strips::iterator strip_end,
	     Edges::iterator edge_begin, Edges::iterator edge_end);

  ostream &output(ostream &out) const;
 

  const Vertex *_vertex;
  Edges _edges;
  Strips _strips;
  int _planar;
  BuilderBucket *_bucket;
  Mesher *_mesher;
};

template <class PrimType>
ostream &operator << (ostream &out, const MesherFanMaker<PrimType> &fe) {
  return fe.output(out);
}

#include "mesherFanMaker.I"

#endif // SUPPORT_FANS

#endif
