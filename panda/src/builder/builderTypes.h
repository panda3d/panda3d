// Filename: builderTypes.h
// Created by:  drose (11Sep97)
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
#ifndef BUILDERTYPES_H
#define BUILDERTYPES_H

#include "pandabase.h"

#include "luse.h"
#include "pta_TexCoordf.h"
#include "pta_Vertexf.h"
#include "pta_Normalf.h"
#include "pta_Colorf.h"

typedef TexCoordf BuilderTC;
typedef Vertexf BuilderV;
typedef Normalf BuilderN;
typedef Colorf BuilderC;

typedef PTA_TexCoordf PTA_BuilderTC;
typedef PTA_Vertexf PTA_BuilderV;
typedef PTA_Normalf PTA_BuilderN;
typedef PTA_Colorf PTA_BuilderC;

enum BuilderAttribFlags {
  BAF_coord                  = 0x00001,
  BAF_normal                 = 0x00002,
  BAF_texcoord               = 0x00004,
  BAF_color                  = 0x00008,
  BAF_pixel_size             = 0x00010,

  BAF_overall_updated        = 0x00100,
  BAF_overall_normal         = 0x00200,
  BAF_overall_color          = 0x00400,
  BAF_overall_pixel_size     = 0x00800,

  BAF_vertex_normal          = 0x01000,
  BAF_vertex_texcoord        = 0x02000,
  BAF_vertex_color           = 0x04000,
  BAF_vertex_pixel_size      = 0x08000,

  BAF_component_normal       = 0x10000,
  BAF_component_color        = 0x20000,
  BAF_component_pixel_size   = 0x04000,
};

ostream &operator << (ostream &out, BuilderAttribFlags baf);

enum BuilderPrimType {
  BPT_poly,
  BPT_point,
  BPT_line,

  // The following types are generated internally by the builder and
  // mesher.  Normally they will not be seen by the user.
  BPT_tri,
  BPT_tristrip,
  BPT_trifan,
  BPT_quad,
  BPT_quadstrip,
  BPT_linestrip,
};

ostream &operator << (ostream &out, BuilderPrimType bpt);

#endif
