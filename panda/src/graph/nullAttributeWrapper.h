// Filename: nullAttributeWrapper.h
// Created by:  drose (22Mar00)
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

#ifndef NULLATTRIBUTEWRAPPER_H
#define NULLATTRIBUTEWRAPPER_H

#include <pandabase.h>

#include "nodeAttribute.h"

#include <updateSeq.h>
#include <pointerTo.h>

class NullTransitionWrapper;

////////////////////////////////////////////////////////////////////
//       Class : NullAttributeWrapper
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NullAttributeWrapper {
public:
  typedef NullTransitionWrapper TransitionWrapper;
  typedef NullAttributeWrapper AttributeWrapper;

  INLINE_GRAPH NullAttributeWrapper();
  INLINE_GRAPH NullAttributeWrapper(const NullAttributeWrapper &copy);
  INLINE_GRAPH void operator = (const NullAttributeWrapper &copy);
  INLINE_GRAPH static NullAttributeWrapper
  init_from(const NullTransitionWrapper &trans);

  INLINE_GRAPH bool is_initial() const;
  INLINE_GRAPH int compare_to(const NullAttributeWrapper &other) const;

  INLINE_GRAPH void make_initial();
  INLINE_GRAPH void apply_in_place(const NullTransitionWrapper &trans);

  INLINE_GRAPH void output(ostream &out) const;
  INLINE_GRAPH void write(ostream &out, int indent_level = 0) const;
};

INLINE_GRAPH ostream &operator << (ostream &out, const NullAttributeWrapper &naw);

#ifndef DONT_INLINE_GRAPH
#include "nullAttributeWrapper.I"
#endif

#endif
