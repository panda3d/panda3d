// Filename: node.cxx
// Created by:  drose (27Oct98)
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

#include "node.h"
#include "nodeRelation.h"

#include <bamWriter.h>
#include <bamReader.h>
#include <datagramIterator.h>
#include <datagram.h>
#include <indent.h>

NodeConnection Node::_empty_connection;
TypeHandle Node::_type_handle;
Node* const Node::Null = (Node*)0L;


////////////////////////////////////////////////////////////////////
//     Function: Node::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Node::
Node() {
  MemoryUsage::update_type(this, this);
}

////////////////////////////////////////////////////////////////////
//     Function: Node::Copy Constructor
//       Access: Public
//  Description: The Node copy constructor does not copy children.
//               Use copy_subgraph() if you want a deep copy of the
//               node and all of its children.  Also, you should use
//               make_copy() if you just want a typed copy of the Node
//               (without children).
////////////////////////////////////////////////////////////////////
Node::
Node(const Node &copy) :
  TypedWritable(copy),
  BoundedObject(copy),
  ReferenceCount(copy)
{
  MemoryUsage::update_type(this, this);
}

////////////////////////////////////////////////////////////////////
//     Function: Node::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Node::
operator = (const Node &copy) {
  TypedWritable::operator = (copy);
  BoundedObject::operator = (copy);
  ReferenceCount::operator = (copy);
}

////////////////////////////////////////////////////////////////////
//     Function: Node::Destructor
//       Access: Public, Virtual
//  Description: When a Node destructs, all of its arcs must be
//               deleted as well.
////////////////////////////////////////////////////////////////////
Node::
~Node() {
  // We'd better not have any arcs pointing into this node, since
  // we're destructing it now.  If we do, the destructor was called in
  // error.
  int i;
  for (i = 0; i < max_node_graphs; i++) {
    UpRelationPointers &urp = _connections[i].get_up();
    nassertv(urp.empty());
  }

  // Now disconnect all the child arcs.
  for (i = 0; i < max_node_graphs; i++) {
    DownRelationPointers &drp = _connections[i].get_down();

    DownRelationPointers::iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      nassertv(arc != (NodeRelation *)NULL);

      // This deletes the arc and anything below it through the magic
      // of reference-counting.
      arc->detach_below();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
Node *Node::
make_copy() const {
  return new Node(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Node::copy_subgraph
//       Access: Public
//  Description: Allocates and returns a complete copy of this node
//               and the entire scene graph rooted at this node.  Some
//               data may still be shared from the original
//               (e.g. vertex index tables), but nothing that will
//               impede normal use of the node.
////////////////////////////////////////////////////////////////////
Node *Node::
copy_subgraph(TypeHandle graph_type) const {
  InstanceMap inst_map;
  return r_copy_subgraph(graph_type, inst_map);
}

////////////////////////////////////////////////////////////////////
//     Function: Node::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool Node::
safe_to_flatten() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool Node::
safe_to_transform() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine
//               this particular kind of Node with other kinds of
//               Nodes, adding children or whatever.  For instance, an
//               LODNode should not be combined with any other node,
//               because its set of children is meaningful.
////////////////////////////////////////////////////////////////////
bool Node::
safe_to_combine() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
////////////////////////////////////////////////////////////////////
void Node::
xform(const LMatrix4f &) {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::combine_with
//       Access: Public, Virtual
//  Description: Collapses this node with the other node, if possible,
//               and returns a pointer to the combined node, or NULL
//               if the two nodes cannot safely be combined.
//
//               The return value may be this, other, or a new node
//               altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two nodes and what the
//               collapsed node should look like.
////////////////////////////////////////////////////////////////////
Node *Node::
combine_with(Node *other) {
  // An unadorned Node always combines with any other Nodes by
  // yielding completely.  However, if we are actually some fancy Node
  // type that derives from Node but didn't redefine this function, we
  // should refuse to combine.
  if (is_exact_type(get_class_type())) {
    // No, we're an ordinary Node.
    return other;

  } else if (other->is_exact_type(get_class_type())) {
    // We're not an ordinary Node, but the other one is.
    return this;
  }

  // We're something other than an ordinary Node.  Don't combine.
  return (Node *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::transform_changed
//       Access: Public, Virtual
//  Description: Called whenever the transform matrix on one of the
//               arcs directly above this node has changed.  This is
//               simply a hook so the node can do something
//               appropriate.  It does not get called when arcs far
//               above the node change.
////////////////////////////////////////////////////////////////////
void Node::
transform_changed(NodeRelation *) {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::get_num_parents
//       Access: Public
//  Description: Returns the number of parent arcs of the indicated
//               type (e.g. RenderRelation::get_class_type()) the node
//               has.
////////////////////////////////////////////////////////////////////
int Node::
get_num_parents(TypeHandle type) const {
  const UpRelationPointers &urp = find_connection(type).get_up();
  return urp.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Node::get_parent
//       Access: Public
//  Description: Returns the nth parent arc of the indicated type the
//               node has.  The index must be in the range 0 <= index
//               < get_num_parents(type).
////////////////////////////////////////////////////////////////////
NodeRelation *Node::
get_parent(TypeHandle type, int index) const {
  const UpRelationPointers &urp = find_connection(type).get_up();
  nassertr(index >= 0 && index < (int)urp.size(),
           (NodeRelation *)NULL);
  return urp[index];
}

////////////////////////////////////////////////////////////////////
//     Function: Node::get_num_children
//       Access: Public
//  Description: Returns the number of child arcs of the indicated
//               type (e.g. RenderRelation::get_class_type()) the node
//               has.
////////////////////////////////////////////////////////////////////
int Node::
get_num_children(TypeHandle type) const {
  const DownRelationPointers &drp = find_connection(type).get_down();
  return drp.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Node::get_child
//       Access: Public
//  Description: Returns the nth child arc of the indicated type the
//               node has.  The index must be in the range 0 <= index
//               < get_num_children(type).
////////////////////////////////////////////////////////////////////
NodeRelation *Node::
get_child(TypeHandle type, int index) const {
  const DownRelationPointers &drp = find_connection(type).get_down();
  nassertr(index >= 0 && index < (int)drp.size(),
           (NodeRelation *)NULL);
  return drp[index];
}

////////////////////////////////////////////////////////////////////
//     Function: Node::app_traverse
//       Access: Public, Virtual
//  Description: This hook function is called on each node visited
//               during the App traversal.  The ArcChain passed in
//               represents the complete chain from the root of the
//               graph to this node, if it is known.
////////////////////////////////////////////////////////////////////
void Node::
app_traverse(const ArcChain &) {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::draw_traverse
//       Access: Public, Virtual
//  Description: This hook function is called on each node visited
//               during the Draw traversal.  The ArcChain passed in
//               represents the complete chain from the root of the
//               graph to this node, if it is known.
////////////////////////////////////////////////////////////////////
void Node::
draw_traverse(const ArcChain &) {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::dgraph_traverse
//       Access: Public, Virtual
//  Description: This hook function is called on each node visited
//               during the data graph traversal.  The ArcChain passed
//               in represents the complete chain from the root of the
//               graph to this node, if it is known.
////////////////////////////////////////////////////////////////////
void Node::
dgraph_traverse(const ArcChain &) {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::sub_render
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool Node::
sub_render(const AllAttributesWrapper &, AllTransitionsWrapper &,
           RenderTraverser *) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool Node::
has_sub_render() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::output
//       Access: Public, Virtual
//  Description: Writes a brief description of the node to the
//               indicated output stream.  This is invoked by the <<
//               operator.  It may be overridden in derived classes to
//               include some information relevant to the class.
////////////////////////////////////////////////////////////////////
void Node::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: Node::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Node::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: Node::p_find_connection
//       Access: Private
//  Description: The non-inline implementation of find_connection().
////////////////////////////////////////////////////////////////////
const NodeConnection &Node::
p_find_connection(TypeHandle graph_type) const {
  // _connections[0] is already tested by the inline implementation.
  for (int i = 1; i < max_node_graphs; i++) {
    if (_connections[i].get_graph_type() == graph_type) {
      return _connections[i];
    }
  }
  return _empty_connection;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::p_update_connection
//       Access: Private
//  Description: The non-inline implementation of update_connection().
////////////////////////////////////////////////////////////////////
NodeConnection *Node::
p_update_connection(TypeHandle graph_type) {
  // _connections[0] is already tested by the inline implementation.
  int i;
  for (i = 1; i < max_node_graphs; i++) {
    if (_connections[i].get_graph_type() == graph_type) {
      return &_connections[i];
    }
  }

  // No such connection; can we create a new one?
  for (i = 0; i < max_node_graphs; i++) {
    if (_connections[i].is_empty()) {
      _connections[i].set_graph_type(graph_type);
      return &_connections[i];
    }
  }

  // Too bad.
  return (NodeConnection *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void Node::
propagate_stale_bound() {
  // Mark all of our parent arcs, in all graphs, stale as well.
  for (int i = 0; i < max_node_graphs; i++) {
    const UpRelationPointers &urp = _connections[i].get_up();
    UpRelationPointers::const_iterator urpi;
    for (urpi = urp.begin(); urpi != urp.end(); ++urpi) {
      (*urpi)->mark_bound_stale();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::r_copy_subgraph
//       Access: Protected, Virtual
//  Description: This is the recursive implementation of copy_subgraph().
//               It returns a copy of the entire subgraph rooted at
//               this node.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of Node that is
//               not also a member of PANDA.DLL *cannot* access this
//               map.
////////////////////////////////////////////////////////////////////
Node *Node::
r_copy_subgraph(TypeHandle graph_type, Node::InstanceMap &inst_map) const {
  Node *copy = make_copy();
  nassertr(copy != (Node *)NULL, NULL);
  if (copy->get_type() != get_type()) {
    graph_cat.warning()
      << "Don't know how to copy nodes of type " << get_type() << "\n";
  }

  copy->r_copy_children(this, graph_type, inst_map);
  return copy;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::r_copy_children
//       Access: Protected, Virtual
//  Description: This is called by r_copy_subgraph(); the copy has
//               already been made of this particular node (and this
//               is the copy); this function's job is to copy all of
//               the children from the original.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of Node that is
//               not also a member of PANDA.DLL *cannot* access this
//               map, and probably should not even override this
//               function.
////////////////////////////////////////////////////////////////////
void Node::
r_copy_children(const Node *from, TypeHandle graph_type,
                Node::InstanceMap &inst_map) {
  const DownRelationPointers &drp =
    from->find_connection(graph_type).get_down();
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    NodeRelation *source_arc = (*drpi);
    Node *source_child = source_arc->get_child();
    nassertv(source_child != (Node *)NULL);

    Node *dest_child;

    // Check to see if we have already copied this child.  If we
    // have, use the copy.  In this way, a subgraph that contains
    // instances will be correctly duplicated into another subgraph
    // that also contains its own instances.
    InstanceMap::const_iterator ci;
    ci = inst_map.find(source_child);
    if (ci != inst_map.end()) {
      dest_child = (*ci).second;
    } else {
      dest_child = source_child->r_copy_subgraph(graph_type, inst_map);
      inst_map[source_child] = dest_child;
    }

    NodeRelation *dest_arc =
      NodeRelation::create_typed_arc(graph_type, this, dest_child);
    nassertv(dest_arc != (NodeRelation *)NULL);
    nassertv(dest_arc->is_exact_type(graph_type));

    dest_arc->copy_transitions_from(source_arc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Node::
write_datagram(BamWriter *manager, Datagram &me) {
  // A node should not write out its UpRelations.  This is because if
  // a node is written out from the middle of the graph, then most
  // likely we only want to write it and its children.  If we did
  // write out all of the UpRelations then writing any node in a graph
  // would cause the entire graph to be written out.

  // First, count up the number of NodeConnections that are nonempty.
  int num_connections = 0;
  int i;
  for (i = 0; i < max_node_graphs; i++) {
    if (!_connections[i].is_empty()) {
      num_connections++;
    }
  }

  me.add_uint16(num_connections);

  // Now write them out.
  for (i = 0; i < max_node_graphs; i++) {
    if (!_connections[i].is_empty()) {
      manager->write_handle(me, _connections[i].get_graph_type());

      const DownRelationPointers &drp = _connections[i].get_down();
      me.add_uint16(drp.size());
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
        NodeRelation *relation = (*drpi);
        manager->write_pointer(me, relation);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Node::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointers to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int Node::
complete_pointers(vector_typedWritable &p_list, BamReader *manager) {
  if (manager->get_file_minor_ver() < 3) {
    // In bam versions before 3.3, this function does nothing (since
    // the arcs are completely responsible for adding themselves to
    // our list); we only need to return the number of pointers we
    // expected to receive.

    // As of 5/7/01, we no longer store this number, as a memory
    // optimization.  This means bams before 3.3 cannot be reliably
    // loaded.

    graph_cat.warning()
      << "Unable to reliably load bam version "
      << manager->get_file_major_ver() << "."
      << manager->get_file_minor_ver() << "\n";

    return 0;
  }

  // Beginning at bam version 3.3, we are responsible for adding our
  // child arcs directly.

  int count = 0;

  for (int i = 0; i < max_node_graphs; i++) {
    if (!_connections[i].is_empty()) {
      DownRelationPointers &drp = _connections[i].get_down();

      DownRelationPointers::iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
        (*drpi) = DCAST(NodeRelation, p_list[count]);
        count++;
      }
    }
  }

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::make_Node
//       Access: Protected
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
TypedWritable* Node::
make_Node(const FactoryParams &params) {
  Node *me = new Node;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void Node::
fillin(DatagramIterator &scan, BamReader *manager) {
  if (manager->get_file_minor_ver() < 3) {
    // In bam versions before 3.3, we only need to count up the total
    // number of arcs across all types, because we don't add the arc
    // pointers explicitly here.
    int num_types = scan.get_uint16();

    while (num_types > 0) {
      int num_arcs = scan.get_uint16();
      while (num_arcs > 0) {
        manager->read_pointer(scan, this);
        num_arcs--;
      }
      num_types--;
    }

  } else {
    // Beginning at bam version 3.3, we do all the reading of our
    // children arcs of all types, which means we need to record the
    // numbers of each type of children.  We do this by recording a
    // series of NULL pointers in our NodeConnections.

    int num_connections = scan.get_uint16();
    if (num_connections > max_node_graphs) {
      // Oops, too many graph types in this bam file.  We'll have to
      // discard some.
      graph_cat.error()
        << "Bam file specifies " << num_connections << " graph types for "
        << *this << "; this version of Panda can only support "
        << max_node_graphs << " simultaneous graph types.\n";
    }
    for (int i = 0; i < num_connections; i++) {
      TypeHandle type = manager->read_handle(scan);

      if (i < max_node_graphs) {
        _connections[i].set_graph_type(type);

        DownRelationPointers &drp = _connections[i].get_down();
        int num_arcs = scan.get_uint16();
        while (num_arcs > 0) {
          manager->read_pointer(scan, this);
          drp.push_back((NodeRelation *)NULL);
          num_arcs--;
        }
      } else {
        // Read and discard.
        int num_arcs = scan.get_uint16();
        while (num_arcs > 0) {
          manager->skip_pointer(scan);
          num_arcs--;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
void Node::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_Node);
}

////////////////////////////////////////////////////////////////////
//     Function: find_arc
//  Description: Searches for the arc of the indicated type that
//               connects the two indicated nodes.  Returns the arc if
//               it exists, or NULL if does not.
////////////////////////////////////////////////////////////////////
NodeRelation *
find_arc(Node *parent, Node *child, TypeHandle graph_type) {
  // We must now walk the RelationPointers list, looking for the
  // matching parent-child arc.  We'll start in the child looking for
  // the parent arc, on the assumption there are likely to be fewer
  // parents than children.

  const UpRelationPointers &urp = child->find_connection(graph_type).get_up();
  UpRelationPointers::const_iterator urpi;
  for (urpi = urp.begin(); urpi != urp.end(); ++urpi) {
    NodeRelation *arc = (*urpi);
    if (arc->get_parent() == parent && arc->get_child() == child) {
      return arc;
    }
  }

  // There is no matching arc.
  return (NodeRelation *)NULL;
}
