/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cardMaker.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef CARDMAKER_H
#define CARDMAKER_H

#include "pandabase.h"

#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "namable.h"
#include "texture.h"

/**
 * This class generates 2-d "cards", that is, rectangular polygons,
 * particularly useful for showing textures etc.  in the 2-d scene graph.
 */
class EXPCL_PANDA_GRUTIL CardMaker : public Namable {
PUBLISHED:
  INLINE explicit CardMaker(const std::string &name);
  INLINE ~CardMaker();

  void reset();
  void set_uv_range(const LTexCoord &ll, const LTexCoord &ur);
  void set_uv_range(const LTexCoord &ll, const LTexCoord &lr, const LTexCoord &ur, const LTexCoord &ul);
  void set_uv_range(const LTexCoord3 &ll, const LTexCoord3 &lr, const LTexCoord3 &ur, const LTexCoord3 &ul);
  void set_uv_range(const LVector4 &x, const LVector4 &y, const LVector4 &z);
  void set_uv_range_cube(int face);
  void set_uv_range(const Texture *tex);

  INLINE void set_has_uvs(bool flag);
  INLINE void set_has_3d_uvs(bool flag);

  INLINE void set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_frame(const LVecBase4 &frame);
  INLINE void set_frame(const LVertex &ll, const LVertex &lr, const LVertex &ur, const LVertex &ul);
  INLINE void set_frame_fullscreen_quad();

  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_color(const LColor &color);

  INLINE void set_has_normals(bool flag);

  INLINE void set_source_geometry(PandaNode *node, const LVecBase4 &frame);
  INLINE void clear_source_geometry();

  PT(PandaNode) generate();

private:
  PT(PandaNode) rescale_source_geometry();

  bool _has_uvs, _has_3d_uvs;
  LVertex _ul_tex, _ll_tex, _lr_tex, _ur_tex;
  LTexCoord3 _ul_pos, _ll_pos, _lr_pos, _ur_pos;

  bool _has_color;
  LColor _color;

  bool _has_normals;

  PT(PandaNode) _source_geometry;
  LVecBase4 _source_frame;
};

#include "cardMaker.I"

#endif
