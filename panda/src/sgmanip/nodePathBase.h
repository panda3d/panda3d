// Filename: nodePathBase.h
// Created by:  drose (06Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef NODEPATHBASE_H
#define NODEPATHBASE_H

#include <pandabase.h>

#include <node.h>
#include <pt_Node.h>
#include <arcChain.h>
#include <renderRelation.h>

////////////////////////////////////////////////////////////////////
//       Class : NodePathBase
// Description : This is the base class for NodePath so we can defined
//               NodePathCollections as collections of something.  All
//               of the interface for NodePath is defined at that
//               level; this just defines the things that are stored
//               here.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePathBase : public ArcChain {
public:
  INLINE NodePathBase(TypeHandle graph_type = RenderRelation::get_class_type());
  INLINE NodePathBase(const ArcChain &chain, TypeHandle graph_type);
  INLINE NodePathBase(const NodePathBase &copy);
  INLINE void operator = (const NodePathBase &copy);

protected:
  // Most of the interesting part of NodePathBase is inherited from
  // ArcChain.  This gives us a sharable linked list of arcs.

  // The _top_node pointer is only used when the NodePath is a
  // singleton.  If there is at least one arc, this is ignored.
  PT_Node _top_node;

  TypeHandle _graph_type;
  static int _max_search_depth;
};

#include "nodePathBase.I"

#endif
