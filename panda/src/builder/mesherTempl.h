// Filename: mesherTempl.h
// Created by:  drose (15Sep97)
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
#ifndef MESHERTEMPL_H
#define MESHERTEMPL_H

#include "pandabase.h"

#include "mesherConfig.h"
#include "builderBucket.h"
#include "mesherEdge.h"
#include "mesherStrip.h"

#include "pvector.h"
#include "plist.h"
#include "pset.h"
#include "pmap.h"

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
  typedef plist<Strip> Strips;
  typedef pset<Edge, less<Edge> > Edges;
  typedef pset<Edge *, less<Edge *> > EdgePtrs;
  typedef pmap<Vertex, EdgePtrs, less<Vertex> > Verts;

  // This is used for show-tstrips.
  typedef pvector<BuilderC> Colors;
  // And this is used for show-qsheets.
  typedef pmap<int, int, less<int> > ColorSheetMap;

  int count_vert_edges(const EdgePtrs &edges) const;
  plist<Strip> &choose_strip_list(const Strip &strip);

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
  TYPENAME Strips::iterator _next_strip;
  Colors _colors;
  ColorSheetMap _color_sheets;

  friend class MesherStrip<PrimType>;
  friend class MesherFanMaker<PrimType>;
};

#include "mesherTempl.I"

#endif
