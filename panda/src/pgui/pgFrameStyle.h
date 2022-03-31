/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgFrameStyle.h
 * @author drose
 * @date 2001-07-03
 */

#ifndef PGFRAMESTYLE_H
#define PGFRAMESTYLE_H

#include "pandabase.h"

#include "luse.h"
#include "texture.h"
#include "pointerTo.h"

class PandaNode;
class NodePath;

/**
 *
 */
class EXPCL_PANDA_PGUI PGFrameStyle {
PUBLISHED:
  INLINE PGFrameStyle();
  INLINE PGFrameStyle(const PGFrameStyle &copy);
  INLINE void operator = (const PGFrameStyle &copy);

  INLINE ~PGFrameStyle();

  enum Type {
    T_none,
    T_flat,
    T_bevel_out,
    T_bevel_in,
    T_groove,
    T_ridge,
    T_texture_border
  };

  INLINE void set_type(Type type);
  INLINE Type get_type() const;

  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a);
  INLINE void set_color(const LColor &color);
  INLINE LColor get_color() const;

  INLINE void set_texture(Texture *texture);
  INLINE bool has_texture() const;
  INLINE Texture *get_texture() const;
  INLINE void clear_texture();

  INLINE void set_width(PN_stdfloat x, PN_stdfloat y);
  INLINE void set_width(const LVecBase2 &width);
  INLINE const LVecBase2 &get_width() const;

  INLINE void set_uv_width(PN_stdfloat u, PN_stdfloat v);
  INLINE void set_uv_width(const LVecBase2 &uv_width);
  INLINE const LVecBase2 &get_uv_width() const;

  INLINE void set_visible_scale(PN_stdfloat x, PN_stdfloat y);
  INLINE void set_visible_scale(const LVecBase2 &visible_scale);
  INLINE const LVecBase2 &get_visible_scale() const;

  LVecBase4 get_internal_frame(const LVecBase4 &frame) const;

  void output(std::ostream &out) const;

public:
  bool xform(const LMatrix4 &mat);
  NodePath generate_into(const NodePath &parent, const LVecBase4 &frame,
                         int sort = 0);

private:
  PT(PandaNode) generate_flat_geom(const LVecBase4 &frame);
  PT(PandaNode) generate_bevel_geom(const LVecBase4 &frame, bool in);
  PT(PandaNode) generate_groove_geom(const LVecBase4 &frame, bool in);
  PT(PandaNode) generate_texture_border_geom(const LVecBase4 &frame);

private:
  Type _type;
  UnalignedLVecBase4 _color;
  PT(Texture) _texture;
  LVecBase2 _width;
  LVecBase2 _uv_width;
  LVecBase2 _visible_scale;
};

INLINE std::ostream &operator << (std::ostream &out, const PGFrameStyle &pfs);
std::ostream &operator << (std::ostream &out, PGFrameStyle::Type type);

#include "pgFrameStyle.I"

#endif
