// Filename: deferredArcTraverser.h
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

#ifndef DEFERREDARCTRAVERSER_H
#define DEFERREDARCTRAVERSER_H

#include <pandabase.h>

#include "deferredArcProperty.h"

#include <traverserVisitor.h>
#include <nullTransitionWrapper.h>

#include "pmap.h"

class NodeRelation;

///////////////////////////////////////////////////////////////////
//       Class : DeferredArcTraverser
// Description : This class is used after all of the nodes have been
//               built to go back and apply down all of the
//               DeferredArcProperties that might have been built of
//               for each arc.  It's a standard Panda
//               TraverserVisitor.
//
//               This class is only local to this package; it is not
//               exported.
////////////////////////////////////////////////////////////////////
class DeferredArcTraverser : public TraverserVisitor<NullTransitionWrapper, DeferredArcProperty> {
public:
  DeferredArcTraverser(const DeferredArcs &deferred_arcs);

  bool forward_arc(NodeRelation *arc, NullTransitionWrapper &,
                   NullTransitionWrapper &, NullTransitionWrapper &,
                   DeferredArcProperty &level_state);

  const DeferredArcs &_deferred_arcs;
};


#endif
