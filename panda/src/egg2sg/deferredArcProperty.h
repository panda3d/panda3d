// Filename: deferredArcProperty.h
// Created by:  drose (04Jul00)
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

#ifndef DEFERREDARCPROPERTY_H
#define DEFERREDARCPROPERTY_H

#include <pandabase.h>

#include <collideMask.h>

#include "nodeRelation.h"
class Node;

///////////////////////////////////////////////////////////////////
//       Class : DeferredArcProperty
// Description : This class keeps track of all the state we must make
//               note of during the graph traversal, but cannot apply
//               immediately.  An instance of this class may be
//               assigned to arcs as they are created, and then later,
//               after the geometry has been created, the graph will
//               be traversed again and the state will be applied.
//
//               This class is only local to this package; it is not
//               exported.
////////////////////////////////////////////////////////////////////
class DeferredArcProperty {
public:
  DeferredArcProperty();
  DeferredArcProperty(const DeferredArcProperty &copy);
  void operator = (const DeferredArcProperty &copy);

  void compose(const DeferredArcProperty &other);

  void apply_to_arc(NodeRelation *arc);
  void apply_to_node(Node *node);


public:
  enum Flags {
    F_has_from_collide_mask   = 0x0001,
    F_has_into_collide_mask   = 0x0002,
  };

  int _flags;
  CollideMask _from_collide_mask;
  CollideMask _into_collide_mask;
};

typedef map<NodeRelation *, DeferredArcProperty> DeferredArcs;


#endif
