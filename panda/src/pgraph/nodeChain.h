// Filename: nodeChain.h
// Created by:  drose (25Feb02)
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

#ifndef NODECHAIN_H
#define NODECHAIN_H

#include "pandabase.h"

#include "pandaNode.h"
#include "renderState.h"
#include "transformState.h"
#include "nodeChainComponent.h"

#include "pointerTo.h"
#include "referenceCount.h"
#include "notify.h"
#include "typedObject.h"

////////////////////////////////////////////////////////////////////
//       Class : NodeChain
// Description : A NodeChain is the fundamental system for
//               disambiguating instances and manipulating the scene
//               graph.  NodeChain is the base class of NodePath,
//               which adds a bit more high-level functionality, but
//               the fundamental scene graph manipulations are defined
//               at the NodeChain level.
//
//               A NodeChain is a list of connected nodes from the
//               root of the graph to any sub-node.  Each NodeChain
//               therefore unqiuely describes one instance of a node.
//
//               NodeChains themselves are lightweight objects that
//               may easily be copied and passed by value.  Their data
//               is stored as a series of NodeChainComponents that are
//               stored on the nodes.  Holding a NodeChain will keep a
//               reference count to all the nodes in the chain.
//               However, if any node in the chain is removed or
//               reparented (perhaps through a different NodeChain),
//               the NodeChain will automatically be updated to
//               reflect the changes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeChain {
PUBLISHED:
  // This enumeration is returned by get_error_type() for an empty
  // NodeChain to report the reason it's empty.
  enum ErrorType {
    ET_ok = 0,     // i.e. not empty, or never assigned to anything.
    ET_not_found,  // returned from a failed find() or similar function.
    ET_removed,    // remove_node() was previously called on this NodeChain.
    ET_fail,       // general failure return from some function.
  };

  INLINE NodeChain();
  INLINE NodeChain(PandaNode *top_node);
  INLINE NodeChain(const NodeChain &copy);
  INLINE void operator = (const NodeChain &copy);

  INLINE static NodeChain not_found();
  INLINE static NodeChain removed();
  INLINE static NodeChain fail();

  // Methods to query a NodeChain's contents.
  INLINE bool is_empty() const;
  INLINE bool is_singleton() const;
  int get_num_nodes() const;
  PandaNode *get_node(int index) const;

  INLINE ErrorType get_error_type() const;

  PandaNode *get_top_node() const;
  INLINE PandaNode *node() const;

  // Methods that return collections of NodePaths derived from or
  // related to this one.

  /*
  NodeChainCollection get_siblings() const;
  NodeChainCollection get_children() const;
  INLINE int get_num_children() const;
  INLINE NodeChain get_child(int n) const;
  */

  INLINE bool has_parent() const;
  INLINE NodeChain get_parent() const;

  /*
  INLINE NodeChain find_path_down_to(Node *dnode) const;
  INLINE NodeChain find(const string &chain) const;

  NodeChainCollection
  find_all_paths_down_to(Node *dnode) const;

  NodeChainCollection
  find_all_matches(const string &chain) const;
  */

  // Methods that actually move nodes around in the scene graph.  The
  // optional "sort" parameter can be used to force a particular
  // ordering between sibling nodes, useful when dealing with LOD's
  // and similar switch nodes.  If the sort value is the same, nodes
  // will be arranged in the order they were added.
  void reparent_to(const NodeChain &other, int sort = 0);
  void wrt_reparent_to(const NodeChain &other, int sort = 0);
  NodeChain instance_to(const NodeChain &other, int sort = 0) const;
  NodeChain copy_to(const NodeChain &other, int sort = 0) const;
  NodeChain attach_new_node(PandaNode *node, int sort = 0) const;
  INLINE NodeChain attach_new_node(const string &name, int sort = 0) const;
  void remove_node();


  // Relative transform and state changes between nodes.
  CPT(RenderState) get_rel_state(const NodeChain &other) const;
  INLINE CPT(RenderState) get_net_state() const;

  CPT(TransformState) get_rel_transform(const NodeChain &other) const;
  INLINE CPT(TransformState) get_net_transform() const;

  INLINE const LMatrix4f &get_mat(const NodeChain &other) const;

  bool verify_complete() const;

public:
  INLINE bool operator == (const NodeChain &other) const;
  INLINE bool operator != (const NodeChain &other) const;
  INLINE bool operator < (const NodeChain &other) const;
  INLINE int compare_to(const NodeChain &other) const;

  INLINE void output(ostream &out) const;

private:
  void uncollapse_head() const;
  static void find_common_ancestor(const NodeChain &a, const NodeChain &b,
                                   int &a_count, int &b_count);

  CPT(RenderState) r_get_net_state(NodeChainComponent *comp) const;
  CPT(RenderState) r_get_partial_state(NodeChainComponent *comp, int n) const;
  CPT(TransformState) r_get_net_transform(NodeChainComponent *comp) const;
  CPT(TransformState) r_get_partial_transform(NodeChainComponent *comp, int n) const;
  void r_output(ostream &out, NodeChainComponent *comp) const;
  static int r_compare_to(const NodeChainComponent *a, const NodeChainComponent *v);

  PT(NodeChainComponent) _head;
  ErrorType _error_type;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "NodeChain");
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const NodeChain &node_chain);

#include "nodeChain.I"

#endif
