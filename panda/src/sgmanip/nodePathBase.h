// Filename: nodePathBase.h
// Created by:  drose (06Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODEPATHBASE_H
#define NODEPATHBASE_H

#include <pandabase.h>

#include <node.h>
#include <pt_Node.h>
#include <nodeRelation.h>
#include <pointerTo.h>
#include <referenceCount.h>
#include <renderRelation.h>

#include <vector>

////////////////////////////////////////////////////////////////////
//       Class : NodePathBase
// Description : This is the base class for NodePath so we can defined
//               NodePathCollections as collections of something.  All
//               of the interface for NodePath is defined at that
//               level; this just defines the things that are stored
//               here.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePathBase {
public:
  INLINE NodePathBase(TypeHandle graph_type = RenderRelation::get_class_type());
  INLINE NodePathBase(const NodePathBase &copy);
  INLINE void operator = (const NodePathBase &copy);

protected:
  // We maintain our own linked list structure here instead of using
  // some STL structure, so we can efficiently copy-construct these
  // things by sharing the initial part of the list.

  // We have a singly-linked list whose head is the bottom arc of the
  // path, and whose tail is the top arc of the path.  Thus, we can
  // copy the entire path by simply copying the head pointer, and we
  // can then append to or shorten our own path without affecting the
  // paths we're sharing ArcComponents with.  Very LISPy.
  class ArcComponent : public ReferenceCount {
  public:
    INLINE ArcComponent(NodeRelation *arc, ArcComponent *next);
    PT(NodeRelation) _arc;
    PT(ArcComponent) _next;
  };

  PT(ArcComponent) _head;

  // This is only used when the NodePath is a singleton.  If there is
  // at least one arc, this is ignored.
  PT_Node _top_node;

  TypeHandle _graph_type;
  static int _max_search_depth;
};

#include "nodePathBase.I"

#endif
