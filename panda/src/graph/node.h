// Filename: node.h
// Created by:  drose (26Oct98)
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

#ifndef NODE_H
#define NODE_H

#include <pandabase.h>

#include "nodeRelation.h"
#include "boundedObject.h"
#include "nodeConnection.h"

#include <typedWritable.h>
#include <referenceCount.h>
#include <luse.h>

class RenderTraverser;
class AllTransitionsWrapper;
class BamWriter;
class BamReader;
class Datagram;
class DatagramIterator;
class ArcChain;

// This is the maximum number of graph types a node may simultaneously
// exist in.
static const int max_node_graphs = 2;

////////////////////////////////////////////////////////////////////
//       Class : Node
// Description : The base class for all scene graph nodes.  A Node may
//               be joined to any number of other nodes, as a parent
//               or as a child, with any kind of relation arcs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Node : public TypedWritable, public BoundedObject,
             virtual public ReferenceCount {
  // We can't simply inherit from TypedWritableReferenceCount, because we need
  // to inherit virtually from ReferenceCount.
public:
  static Node* const Null;

PUBLISHED:
  Node();
  Node(const Node &copy);

public:
  void operator = (const Node &copy);
  virtual ~Node();

  virtual Node *make_copy() const;
  Node *copy_subgraph(TypeHandle graph_type) const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_combine() const;
  virtual void xform(const LMatrix4f &mat);
  virtual Node *combine_with(Node *other); 

  virtual void transform_changed(NodeRelation *arc);

PUBLISHED:
  int get_num_parents(TypeHandle type) const;
  NodeRelation *get_parent(TypeHandle type, int index) const;
  int get_num_children(TypeHandle type) const;
  NodeRelation *get_child(TypeHandle type, int index) const;

public:
  // These functions will be called when the node is visited during
  // the indicated traversal.
  virtual void app_traverse(const ArcChain &chain);
  virtual void draw_traverse(const ArcChain &chain);
  virtual void dgraph_traverse(const ArcChain &chain);

  // This function is similar to another function in NodeTransition.
  // It may or may not intercept the render traversal.
  virtual bool sub_render(const AllTransitionsWrapper &input_trans,
                          AllTransitionsWrapper &modify_trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  // These members define the actual connection of arcs to the nodes.
  // Generally, you should not monkey with them, unless you know what
  // you're doing; the get_parent()/get_child() interface above is
  // simpler to use.

  INLINE_GRAPH const NodeConnection &find_connection(TypeHandle graph_type) const;
  INLINE_GRAPH NodeConnection *update_connection(TypeHandle graph_type);

  NodeConnection _connections[max_node_graphs];

private:
  const NodeConnection &p_find_connection(TypeHandle graph_type) const;
  NodeConnection *p_update_connection(TypeHandle graph_type);
  static NodeConnection _empty_connection;

protected:
  virtual void propagate_stale_bound();

  typedef pmap<Node *, Node *> InstanceMap;
  virtual Node *r_copy_subgraph(TypeHandle graph_type,
                                InstanceMap &inst_map) const;
  virtual void r_copy_children(const Node *from, TypeHandle graph_type,
                               InstanceMap &inst_map);

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(vector_typedWritable &plist,
                                BamReader *manager);

  static TypedWritable *make_Node(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    BoundedObject::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "Node",
                  TypedWritable::get_class_type(),
                  BoundedObject::get_class_type(),
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA INLINE_GRAPH ostream & operator << (ostream &out, const Node &node);

EXPCL_PANDA NodeRelation *
find_arc(Node *parent, Node *child, TypeHandle graph_type);

EXPCL_PANDA INLINE_GRAPH bool
remove_child(Node *parent, Node *child, TypeHandle graph_type);

#ifndef DONT_INLINE_GRAPH
#include "node.I"
#endif

#endif

