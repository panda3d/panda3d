// Filename: mesher.h
// Created by:  drose (15Sep97)
//
////////////////////////////////////////////////////////////////////
#ifndef MESHERTEMPL_H
#define MESHERTEMPL_H

#include <pandabase.h>

#include "mesherConfig.h"
#include "builderBucket.h"
#include "mesherEdge.h"
#include "mesherStrip.h"

#include <vector>
#include <list>
#include <set>
#include <map>

template <class PrimType>
class MesherFanMaker;

template <class PrimType>
class MesherTempl {
public:
  typedef PrimType Prim;
  typedef TYPENAME PrimType::Vertex Vertex;
  typedef TYPENAME PrimType::DAttrib MAttrib;
  typedef MesherEdge<PrimType> Edge;
  typedef MesherStrip<PrimType> Strip;
  typedef MesherFanMaker<PrimType> FanMaker;
 
  MesherTempl(BuilderBucket *bucket);

  int add_prim(const Prim &prim, MesherStripOrigin origin = MO_user);
  void mesh();
  Prim getPrim();

  void finalize();

  void show(ostream &out);

protected:
  typedef list<Strip> Strips;
  typedef set<Edge, less<Edge> > Edges;
  typedef set<Edge *, less<Edge *> > EdgePtrs;
  typedef map<Vertex, EdgePtrs, less<Vertex> > Verts;

  // This is used for show-tstrips.
  typedef vector<BuilderC> Colors;
  // And this is used for show-qsheets.
  typedef map<int, int, less<int> > ColorSheetMap;

  int count_vert_edges(const EdgePtrs &edges) const;
  list<Strip> &choose_strip_list(const Strip &strip);

  void build_sheets();
  void find_fans();
  void make_quads();
  void meshList(Strips &strips);

  Strips _tris, _quads, _strips;
  Strips _dead, _done;
  Verts _verts;
  Edges _edges;
  int _stripIndex;
  BuilderBucket *_bucket;
  Strips::iterator _next_strip;
  Colors _colors;
  ColorSheetMap _color_sheets;

  friend class MesherStrip<PrimType>;
  friend class MesherFanMaker<PrimType>;
};

#include "mesherTempl.I"

#endif
