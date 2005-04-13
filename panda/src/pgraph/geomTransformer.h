// Filename: geomTransformer.h
// Created by:  drose (14Mar02)
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

#ifndef GEOMTRANSFORMER_H
#define GEOMTRANSFORMER_H

#include "pandabase.h"

#include "geom.h"
#include "luse.h"
#include "qpgeomVertexData.h"

class GeomNode;
class RenderState;
class InternalName;

///////////////////////////////////////////////////////////////////
//       Class : GeomTransformer
// Description : An object specifically designed to transform the
//               vertices of a Geom without disturbing indexing or
//               affecting any other Geoms that may share the same
//               vertex arrays, and without needlessly wasting memory
//               when different Geoms sharing the same vertex arrays
//               are transformed by the same amount.
//
//               If you create a single GeomTransformer and use it to
//               transform a number of different Geoms by various
//               transformations, then those Geoms which happen to
//               share the same arrays and are transformed by the same
//               amounts will still share the same arrays as each
//               other (but different from the original arrays).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomTransformer {
public:
  GeomTransformer();
  GeomTransformer(const GeomTransformer &copy);
  ~GeomTransformer();

  INLINE int get_max_collect_vertices() const;
  INLINE void set_max_collect_vertices(int max_collect_vertices);

  bool transform_vertices(Geom *geom, const LMatrix4f &mat);
  bool transform_vertices(GeomNode *node, const LMatrix4f &mat);

  bool transform_texcoords(Geom *geom, const InternalName *from_name,
                           const InternalName *to_name,
                           const LMatrix4f &mat);
  bool transform_texcoords(GeomNode *node, const InternalName *from_name,
                           const InternalName *to_name,
                           const LMatrix4f &mat);

  bool set_color(Geom *geom, const Colorf &color);
  bool set_color(GeomNode *node, const Colorf &color);

  bool transform_colors(Geom *geom, const LVecBase4f &scale);
  bool transform_colors(GeomNode *node, const LVecBase4f &scale);

  bool apply_state(GeomNode *node, const RenderState *state);

  bool collect_vertex_data(Geom *geom, bool keep_names);
  bool collect_vertex_data(GeomNode *node, bool keep_names);

private:
  int _max_collect_vertices;

  class qpSourceVertices {
  public:
    INLINE bool operator < (const qpSourceVertices &other) const;

    LMatrix4f _mat;
    CPT(qpGeomVertexData) _vertex_data;
  };
  typedef pmap<qpSourceVertices, PT(qpGeomVertexData) > NewVertices;
  NewVertices _qpvertices;

  class qpSourceTexCoords {
  public:
    INLINE bool operator < (const qpSourceTexCoords &other) const;

    LMatrix4f _mat;
    CPT(InternalName) _from;
    CPT(InternalName) _to;
    CPT(qpGeomVertexData) _vertex_data;
  };
  typedef pmap<qpSourceTexCoords, PT(qpGeomVertexData) > NewTexCoords;
  NewTexCoords _qptexcoords;

  class qpSourceColors {
  public:
    INLINE bool operator < (const qpSourceColors &other) const;

    LVecBase4f _color;
    CPT(qpGeomVertexData) _vertex_data;
  };
  typedef pmap<qpSourceColors, CPT(qpGeomVertexData) > NewColors;

  // We have two concepts of colors: the "fixed" colors, which are
  // slapped in as a complete replacement of the original colors
  // (e.g. via a ColorAttrib), and the "transformed" colors, which are
  // modified from the original colors (e.g. via a ColorScaleAttrib).
  NewColors _qpfcolors, _qptcolors;


  class SourceVertices {
  public:
    INLINE bool operator < (const SourceVertices &other) const;

    LMatrix4f _mat;
    PTA_Vertexf _coords;
  };
  typedef pmap<SourceVertices, PTA_Vertexf> Vertices;
  Vertices _vertices;

  class SourceNormals {
  public:
    INLINE bool operator < (const SourceNormals &other) const;

    LMatrix4f _mat;
    PTA_Normalf _norms;
  };
  typedef pmap<SourceNormals, PTA_Normalf> Normals;
  Normals _normals;

  class SourceTexCoords {
  public:
    INLINE bool operator < (const SourceTexCoords &other) const;

    LMatrix4f _mat;
    PTA_TexCoordf _texcoords;
  };
  typedef pmap<SourceTexCoords, PTA_TexCoordf> TexCoords;
  TexCoords _texcoords;

  // We have two concepts of colors: the "fixed" colors, which are
  // slapped in as a complete replacement of the original colors
  // (e.g. via a ColorAttrib), and the "transformed" colors, which are
  // modified from the original colors (e.g. via a ColorScaleAttrib).
  typedef pmap<Colorf, PTA_Colorf> FColors;
  FColors _fcolors;

  class SourceColors {
  public:
    INLINE bool operator < (const SourceColors &other) const;

    LVecBase4f _scale;
    PTA_Colorf _colors;
  };
  typedef pmap<SourceColors, PTA_Colorf> TColors;
  TColors _tcolors;

  class AlreadyCollectedData {
  public:
    CPT(qpGeomVertexData) _data;
    int _offset;
  };
  typedef pmap< CPT(qpGeomVertexData), AlreadyCollectedData> AlreadyCollected;
  AlreadyCollected _already_collected;

  class NewCollectedKey {
  public:
    INLINE bool operator < (const NewCollectedKey &other) const;

    string _name;
    CPT(qpGeomVertexFormat) _format;
    qpGeomUsageHint::UsageHint _usage_hint;
  };
  typedef pmap< NewCollectedKey, PT(qpGeomVertexData) > NewCollectedData;
  NewCollectedData _new_collected_data;
};

#include "geomTransformer.I"

#endif

