// Filename: deferredArcTraverser.h
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DEFERREDARCTRAVERSER_H
#define DEFERREDARCTRAVERSER_H

#include <pandabase.h>

#include "deferredArcProperty.h"

#include <traverserVisitor.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>

#include <map>

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
                   NullAttributeWrapper &, NullAttributeWrapper &,
                   DeferredArcProperty &level_state);

  const DeferredArcs &_deferred_arcs;
};


#endif
