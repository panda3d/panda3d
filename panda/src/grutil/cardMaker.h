// Filename: cardMaker.h
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CARDMAKER_H
#define CARDMAKER_H

#include "pandabase.h"

#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "namable.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : CardMaker
// Description : This class generates 2-d "cards", that is,
//               rectangular polygons, particularly useful for showing
//               textures etc. in the 2-d scene graph.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL CardMaker : public Namable {
PUBLISHED:
  INLINE CardMaker(const string &name);
  INLINE ~CardMaker();

  void reset();
  void set_uv_range(const TexCoordf &ll, const TexCoordf &ur);
  void set_uv_range(const TexCoordf &ll, const TexCoordf &lr, const TexCoordf &ur, const TexCoordf &ul);
  void set_uv_range(const TexCoord3f &ll, const TexCoord3f &lr, const TexCoord3f &ur, const TexCoord3f &ul);
  void set_uv_range(const LVector4f &x, const LVector4f &y, const LVector4f &z);
  void set_uv_range_cube(int face);
  void set_uv_range(const Texture *tex);

  INLINE void set_has_uvs(bool flag);
  INLINE void set_has_3d_uvs(bool flag);

  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE void set_frame(const Vertexf &ll, const Vertexf &lr, const Vertexf &ur, const Vertexf &ul);
  INLINE void set_frame_fullscreen_quad();

  INLINE void set_color(float r, float g, float b, float a);
  INLINE void set_color(const Colorf &color);

  INLINE void set_has_normals(bool flag);

  INLINE void set_source_geometry(PandaNode *node, const LVecBase4f &frame);
  INLINE void clear_source_geometry();

  PT(PandaNode) generate();

private:
  PT(PandaNode) rescale_source_geometry();

  bool _has_uvs, _has_3d_uvs;
  Vertexf    _ul_tex, _ll_tex, _lr_tex, _ur_tex;
  TexCoord3f _ul_pos, _ll_pos, _lr_pos, _ur_pos;

  bool _has_color;
  Colorf _color;

  bool _has_normals;

  PT(PandaNode) _source_geometry;
  LVecBase4f _source_frame;
};

#include "cardMaker.I"

#endif

