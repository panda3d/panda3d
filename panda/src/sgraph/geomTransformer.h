// Filename: geomTransformer.h
// Created by:  drose (23May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMTRANSFORMER_H
#define GEOMTRANSFORMER_H

#include <pandabase.h>

#include <geom.h>
#include <luse.h>

class GeomNode;

///////////////////////////////////////////////////////////////////
// 	 Class : GeomTransformer
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
  ~GeomTransformer();

  bool transform_vertices(Geom *geom, const LMatrix4f &mat);
  bool transform_vertices(GeomNode *node, const LMatrix4f &mat);

  bool transform_texcoords(Geom *geom, const LMatrix4f &mat);
  bool transform_texcoords(GeomNode *node, const LMatrix4f &mat);

  bool set_color(Geom *geom, const Colorf &color);
  bool set_color(GeomNode *node, const Colorf &color);

private:
  class SourceVertices {
  public:
    INLINE bool operator < (const SourceVertices &other) const;

    LMatrix4f _mat;
    PTA_Vertexf _coords;
  };
  typedef map<SourceVertices, PTA_Vertexf> Vertices;
  Vertices _vertices;

  class SourceNormals {
  public:
    INLINE bool operator < (const SourceNormals &other) const;

    LMatrix4f _mat;
    PTA_Normalf _norms;
  };
  typedef map<SourceNormals, PTA_Normalf> Normals;
  Normals _normals;

  class SourceTexCoords {
  public:
    INLINE bool operator < (const SourceTexCoords &other) const;

    LMatrix4f _mat;
    PTA_TexCoordf _texcoords;
  };
  typedef map<SourceTexCoords, PTA_TexCoordf> TexCoords;
  TexCoords _texcoords;

  typedef map<Colorf, PTA_Colorf> Colors;
  Colors _colors;
};

#include "geomTransformer.I"

#endif

