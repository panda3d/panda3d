// Filename: node.cxx
// Created by:  drose (27Oct98)
// 

#include "node.h"
#include "nodeRelation.h"
#include <bamWriter.h>
#include <bamReader.h>
#include <datagramIterator.h>
#include <datagram.h>

TypeHandle Node::_type_handle;
Node* const Node::Null = (Node*)0L;

////////////////////////////////////////////////////////////////////
//     Function: Node::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Node::
Node() {
}

////////////////////////////////////////////////////////////////////
//     Function: Node::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Node::
Node(const Node &copy) :
  TypedWriteable(copy),
  BoundedObject(copy),
  ReferenceCount(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Node::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void Node::
operator = (const Node &copy) {
  TypedWriteable::operator = (copy);
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
  UpRelations::iterator uri;
  for (uri = _parents.begin(); uri != _parents.end(); ++uri) {
    nassertv((*uri).second.empty());
  }

  DownRelations::iterator dri;
  for (dri = _children.begin(); dri != _children.end(); ++dri) {
    DownRelationPointers &drp = (*dri).second;

    DownRelationPointers::iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      PT(NodeRelation) relation = (*drpi);

      // This deletes the arc and anything below it through the magic
      // of reference-counting.
      relation->detach_below();
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
  UpRelations::const_iterator uri;
  uri = _parents.find(type);
  if (uri == _parents.end()) {
    return 0;
  }
  return (*uri).second.size();
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
  UpRelations::const_iterator uri;
  uri = _parents.find(type);
  nassertr(uri != _parents.end(), (NodeRelation *)NULL);
  nassertr(index >= 0 && index < (int)(*uri).second.size(),
	   (NodeRelation *)NULL);
  return (*uri).second[index];
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
  DownRelations::const_iterator dri;
  dri = _children.find(type);
  if (dri == _children.end()) {
    return 0;
  }
  return (*dri).second.size();
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
  DownRelations::const_iterator dri;
  dri = _children.find(type);
  nassertr(dri != _children.end(), (NodeRelation *)NULL);
  nassertr(index >= 0 && index < (int)(*dri).second.size(), 
	   (NodeRelation *)NULL);
  return (*dri).second[index];
}

////////////////////////////////////////////////////////////////////
//     Function: Node::sub_render
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool Node::
sub_render(const AllAttributesWrapper &, AllTransitionsWrapper &, 
	   GraphicsStateGuardianBase *) {
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
//     Function: Node::propagate_stale_bound
//       Access: Protected, Virtual
//  Description: Called by BoundedObject::mark_bound_stale(), this
//               should make sure that all bounding volumes that
//               depend on this one are marked stale also.
////////////////////////////////////////////////////////////////////
void Node::
propagate_stale_bound() {
  // Mark all of our parent arcs, in all graphs, stale as well.
  UpRelations::const_iterator uri;
  for (uri = _parents.begin(); uri != _parents.end(); ++uri) {
    const UpRelationPointers &urp = (*uri).second;
    
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
//  Description: This is called by r_deep_copy(); the copy has already
//               been made of this particular node (and this is the
//               copy); this function's job is to copy all of the
//               children from the original.
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
  DownRelations::const_iterator dri;
  dri = from->_children.find(graph_type);
  if (dri != from->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    
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
}

////////////////////////////////////////////////////////////////////
//     Function: Node::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void Node::
write_datagram(BamWriter *manager, Datagram &me)
{

  //A node should not write out it's UpRelations,
  //the rationale for this restriction is that if a
  //a node is choosen in the middle of the graph, then
  //most likely we only want to write it and it's children.
  //if we did write out all of the UpRelations then, writing
  //any node in a graph would cause the entire graph to be written
  //out

  me.add_uint16(_children.size());

  DownRelations::iterator dri;
  for (dri = _children.begin(); dri != _children.end(); ++dri) 
  {
    DownRelationPointers &drp = (*dri).second;
    me.add_uint16(drp.size());

    DownRelationPointers::iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) 
    {
      PT(NodeRelation) relation = (*drpi);

      //Ask manager to write out the pointer for me
      manager->write_pointer(me, relation);
    }
  }

}


////////////////////////////////////////////////////////////////////
//     Function: Node::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointers to TypedWriteable
//               objects that correspond to all the requests for 
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int Node::
complete_pointers(vector_typedWriteable &, BamReader*)
{
  //Dummy function that BamReader expects if there are
  //any read_pointer requests made by an object. But Node is
  //a special case that only calls read_pointer to force its
  //NodeRelations to be read.  Those NodeRelations will add themselves
  //into the Node, so don't attempt to do that as well here.
  return _num_pointers;
}

////////////////////////////////////////////////////////////////////
//     Function: Node::make_Node
//       Access: Protected
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
TypedWriteable* Node::
make_Node(const FactoryParams &params)
{
  Node *me = new Node;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

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
fillin(DatagramIterator& scan, BamReader* manager)
{
  PN_uint16 numRelations = scan.get_uint16();
  PN_uint16 numRelationPointers;
  _num_pointers = 0;
  

  while(numRelations > 0)
  {
    numRelationPointers = scan.get_uint16();
    while(numRelationPointers > 0)
    {
      manager->read_pointer(scan, this);
      numRelationPointers--;
      _num_pointers++;
    }
    numRelations--;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Node::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
void Node::
register_with_read_factory(void)
{
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

  UpRelations::const_iterator uri = child->_parents.find(graph_type);

  if (uri != child->_parents.end()) {
    const UpRelationPointers &urp = (*uri).second;
    UpRelationPointers::const_iterator urpi;
    for (urpi = urp.begin(); urpi != urp.end(); ++urpi) {
      if ((*urpi)->get_parent() == parent && 
	  (*urpi)->get_child() == child) {
	return (*urpi);
      }
    }
  }

  // There is no matching arc.
  return (NodeRelation *)NULL;
}
