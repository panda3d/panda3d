// Filename: builderBucket.h
// Created by:  drose (09Sep97)
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

#ifndef BUILDERBUCKET_H
#define BUILDERBUCKET_H

#include "pandabase.h"

#include "builderProperties.h"

#include "namable.h"
#include "pointerToArray.h"
#include "luse.h"
#include "pta_Vertexf.h"
#include "pta_Normalf.h"
#include "pta_Colorf.h"
#include "pta_TexCoordf.h"
#include "renderState.h"

#include "stdlib.h"

class Geom;
class PandaNode;
class GeomNode;

///////////////////////////////////////////////////////////////////
//       Class : BuilderBucket
// Description : The main grouping tool for BuilderPrims.  See the
//               comments at the beginning of builder.h.
//
//               As each primitive is added to the builder, it is
//               associated with a bucket.  The bucket stores the
//               scene-graph properties of the primitive, and is used
//               to identify primitives that have the same properties
//               and thus may be joined into a common triangle strip.
//
//               This grouping is done via the ordering operator, <,
//               which defines an arbitrary ordering for buckets and
//               identifies those buckets which are equivalent to each
//               other.  By subclassing off of BuilderBucket and
//               redefining this operator, you can change the grouping
//               behavior to suit your needs, if necessary.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderBucket : public BuilderProperties, public Namable {
public:
  BuilderBucket();
  BuilderBucket(const BuilderBucket &copy);
  BuilderBucket &operator = (const BuilderBucket &copy);
  virtual ~BuilderBucket();

  virtual BuilderBucket *make_copy() const;
  virtual GeomNode *make_geom_node();
  virtual Geom *done_geom(Geom *geom);
  void add_attrib(const RenderAttrib *attrib);

  virtual bool operator < (const BuilderBucket &other) const;

  INLINE void set_coords(const PTA_Vertexf &coords);
  INLINE PTA_Vertexf get_coords() const;

  INLINE void set_normals(const PTA_Normalf &normals);
  INLINE PTA_Normalf get_normals() const;

  INLINE void set_texcoords(const PTA_TexCoordf &texcoords);
  INLINE PTA_TexCoordf get_texcoords() const;

  INLINE void set_colors(const PTA_Colorf &colors);
  INLINE PTA_Colorf get_colors() const;

  INLINE static BuilderBucket *get_default_bucket();

  virtual void output(ostream &out) const;

  PandaNode *_node;
  CPT(RenderState) _state;
  bool _hidden;

protected:
  PTA_Vertexf _coords;
  PTA_Normalf _normals;
  PTA_TexCoordf _texcoords;
  PTA_Colorf _colors;

  static BuilderBucket *_default_bucket;

private:
  BuilderBucket(int);
};

INLINE ostream &operator << (ostream &out, const BuilderBucket &bucket) {
  bucket.output(out);
  return out;
}

#include "builderBucket.I"

#endif

