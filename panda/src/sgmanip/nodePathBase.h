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

  // We also add an explicit pointer to the top node in the chain,
  // mainly to allow us to define a NodePath containing a single node,
  // even if the chain of arcs is empty.

  // If the chain is nonempty, this might still be useful (int that it
  // keeps a reference count to the top node), but this is problematic
  // since it will not automatically update if our parent is changed
  // without our knowledge.
  PT_Node _top_node;

  TypeHandle _graph_type;
  static int _max_search_depth;
};

#include "nodePathBase.I"

#endif
