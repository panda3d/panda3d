// Filename: nodeAttributeWrapper.h
// Created by:  drose (20Mar00)
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

#ifndef NODEATTRIBUTEWRAPPER_H
#define NODEATTRIBUTEWRAPPER_H

#include <pandabase.h>

#include "nodeAttribute.h"

#include <updateSeq.h>
#include <pointerTo.h>

class NodeTransitionWrapper;

////////////////////////////////////////////////////////////////////
//       Class : NodeAttributeWrapper
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeAttributeWrapper {
public:
  typedef NodeTransitionWrapper TransitionWrapper;
  typedef NodeAttributeWrapper AttributeWrapper;

  INLINE_GRAPH NodeAttributeWrapper(TypeHandle handle);
  INLINE_GRAPH NodeAttributeWrapper(const NodeAttributeWrapper &copy);
  INLINE_GRAPH void operator = (const NodeAttributeWrapper &copy);
  static NodeAttributeWrapper init_from(const NodeTransitionWrapper &trans);

  INLINE_GRAPH TypeHandle get_handle() const;
  INLINE_GRAPH NodeAttribute *get_attrib() const;
  INLINE_GRAPH void set_attrib(NodeAttribute *attrib);

  INLINE_GRAPH bool is_initial() const;
  INLINE_GRAPH int compare_to(const NodeAttributeWrapper &other) const;

  INLINE_GRAPH void make_initial();
  void apply_in_place(const NodeTransitionWrapper &trans);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  TypeHandle _handle;
  PT(NodeAttribute) _attrib;
};

EXPCL_PANDA INLINE_GRAPH ostream &operator << (ostream &out, const NodeAttributeWrapper &naw);

#include "nodeAttributeWrapper.T"

#ifndef DONT_INLINE_GRAPH
#include "nodeAttributeWrapper.I"
#endif

#endif
