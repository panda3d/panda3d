// Filename: deferredNodeProperty.h
// Created by:  drose (20Mar02)
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

#ifndef DEFERREDNODEPROPERTY_H
#define DEFERREDNODEPROPERTY_H

#include "pandabase.h"

#include "collideMask.h"
#include "pmap.h"

class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : DeferredNodeProperty
// Description : This class keeps track of all the state we must make
//               note of during the graph traversal, but cannot apply
//               immediately.  An instance of this class may be
//               assigned to nodes as they are created, and then later,
//               after the geometry has been created, the graph will
//               be traversed again and the state will be applied.
//
//               This class is only local to this package; it is not
//               exported.
////////////////////////////////////////////////////////////////////
class DeferredNodeProperty {
public:
  DeferredNodeProperty();
  DeferredNodeProperty(const DeferredNodeProperty &copy);
  void operator = (const DeferredNodeProperty &copy);

  void compose(const DeferredNodeProperty &other);

  void apply_to_node(PandaNode *node);


public:
  enum Flags {
    F_has_from_collide_mask   = 0x0001,
    F_has_into_collide_mask   = 0x0002,
  };

  int _flags;
  CollideMask _from_collide_mask;
  CollideMask _into_collide_mask;
};

typedef pmap<PandaNode *, DeferredNodeProperty> DeferredNodes;


#endif
