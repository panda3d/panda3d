// Filename: pgFrameStyle.h
// Created by:  drose (03Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PGFRAMESTYLE_H
#define PGFRAMESTYLE_H

#include "pandabase.h"

#include "luse.h"
#include "pt_Node.h"

class NodeRelation;
class Node;
class PandaNode;
class qpNodePath;

////////////////////////////////////////////////////////////////////
//       Class : PGFrameStyle
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGFrameStyle {
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
    T_ridge
  };

  INLINE void set_type(Type type);
  INLINE Type get_type() const;

  INLINE void set_color(float r, float g, float b, float a);
  INLINE void set_color(const Colorf &color);
  INLINE const Colorf &get_color() const;

  INLINE void set_width(float x, float y);
  INLINE void set_width(const LVecBase2f &width);
  INLINE const LVecBase2f &get_width() const;

  void output(ostream &out) const;

public:
  bool xform(const LMatrix4f &mat);
  NodeRelation *generate_into(Node *node, const LVecBase4f &frame);
  qpNodePath generate_into(const qpNodePath &parent, const LVecBase4f &frame);

private:
  PT_Node generate_flat_geom(const LVecBase4f &frame);
  PT_Node generate_bevel_geom(const LVecBase4f &frame, bool in);
  PT_Node generate_groove_geom(const LVecBase4f &frame, bool in);
  PT(PandaNode) qpgenerate_flat_geom(const LVecBase4f &frame);
  PT(PandaNode) qpgenerate_bevel_geom(const LVecBase4f &frame, bool in);
  PT(PandaNode) qpgenerate_groove_geom(const LVecBase4f &frame, bool in);

private:
  Type _type;
  Colorf _color;
  LVecBase2f _width;
};

INLINE ostream &operator << (ostream &out, const PGFrameStyle &pfs);
ostream &operator << (ostream &out, PGFrameStyle::Type type);

#include "pgFrameStyle.I"

#endif
