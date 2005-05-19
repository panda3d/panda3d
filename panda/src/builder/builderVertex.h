// Filename: builderVertex.h
// Created by:  drose (18Sep97)
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

#ifndef BUILDERVERTEX_H
#define BUILDERVERTEX_H

////////////////////////////////////////////////////////////////////
//
// BuilderVertex, BuilderVertexI
//
// The basic class for passing vertex values to the builder.  See the
// comments at the beginning of builder.h and builderPrim.h.
//
// Like BuilderPrim, the BuilderVertex and BuilderVertexI classes are
// actually two different instantiations of the same template class,
// BuilderVertexTempl.  The difference is in the types of the
// attribute values for the four kinds of attributes: vertices
// (coords), normals, texture coordinates, and colors.  BuilderVertex
// specifies Coordf, Normalf, TexCoordf, and Colorf for each of these
// (actually, it's BuilderV, BuilderN, BuilderTC, and BuilderC, which
// are simply wrappers around the above types), while BuilderVertexI
// specifies ushort for all of them.
//
// It is this templating that drives the whole indexed/nonindexed
// support in this package and in the mesher.  The two kinds of
// BuilderVertex are designed to present largely the same interface,
// regardless of whether its component values are actual vector
// values, or simply index numbers.  Lots of things, therefore, can
// template on the BuilderPrim type (which in turn termplates on the
// BuilderVertex type) and thus easily support both indexed and
// nonindexed geometry.
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "builderAttrib.h"
#include "builderVertexTempl.h"
#include "builderBucket.h"

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define BUILDERVERTEXTEMPL_BUILDERV BuilderVertexTempl<BuilderV, BuilderN, BuilderTC, BuilderC>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, BUILDERVERTEXTEMPL_BUILDERV);
#define BUILDERVERTEXTEMPL_USHORT BuilderVertexTempl<ushort, ushort, ushort, ushort>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDAEGG, EXPTP_PANDAEGG, BUILDERVERTEXTEMPL_USHORT);

/////////////////////////////////////////////////////////////////////
//       Class : BuilderVertex
// Description : The basic class for passing nonindexed vertices to
//               the builder.  See the comments at the the head of
//               this file, and in builder.h.
//
//               Look in builderVertexTempl.h and builderAttribTempl.h
//               for most of the interface to BuilderVertex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderVertex
  : public BuilderVertexTempl<BuilderV, BuilderN, BuilderTC, BuilderC> {
public:
  typedef BuilderAttrib Attrib;

  BuilderVertex() {}
  BuilderVertex(const BuilderV &c) :
    BuilderVertexTempl<BuilderV, BuilderN, BuilderTC, BuilderC>(c) {}

  INLINE void set_coord_value(const BuilderV *array, ushort index);
  INLINE void set_normal_value(const BuilderN *array, ushort index);
  INLINE void set_texcoord_value(const InternalName *name, const BuilderTC *array, ushort index);
  INLINE void set_color_value(const BuilderC *array, ushort index);

  INLINE BuilderV get_coord_value(const BuilderBucket &bucket) const;
  INLINE BuilderN get_normal_value(const BuilderBucket &bucket) const;
  INLINE BuilderTC get_texcoord_value(const InternalName *name, const BuilderBucket &bucket) const;
  INLINE BuilderC get_color_value(const BuilderBucket &bucket) const;

};


/////////////////////////////////////////////////////////////////////
//       Class : BuilderVertexI
// Description : The basic class for passing indexed vertices to the
//               builder.  See the comments at the the head of this
//               file, and in builder.h.
//
//               Look in builderVertexTempl.h and builderAttribTempl.h
//               for most of the interface to BuilderVertex.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG BuilderVertexI
  : public BuilderVertexTempl<ushort, ushort, ushort, ushort> {
public:
  typedef BuilderAttribI Attrib;

  BuilderVertexI() {}
  BuilderVertexI(ushort c) :
    BuilderVertexTempl<ushort, ushort, ushort, ushort>(c) {}

  INLINE void set_coord_value(const BuilderV *array, ushort index);
  INLINE void set_normal_value(const BuilderN *array, ushort index);
  INLINE void set_texcoord_value(const InternalName *name, const BuilderTC *array, ushort index);
  INLINE void set_color_value(const BuilderC *array, ushort index);

  INLINE BuilderV get_coord_value(const BuilderBucket &bucket) const;
  INLINE BuilderN get_normal_value(const BuilderBucket &bucket) const;
  INLINE BuilderTC get_texcoord_value(const InternalName *name, const BuilderBucket &bucket) const;
  INLINE BuilderC get_color_value(const BuilderBucket &bucket) const;
};

#include "builderVertex.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif


