// Filename: collisionNode.cxx
// Created by:  drose (24Apr00)
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


#include "collisionNode.h"
#include "config_collide.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle CollisionNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionNode::
CollisionNode(const string &name) :
  NamedNode(name),
  _from_collide_mask(CollideMask::all_on()),
  _into_collide_mask(CollideMask::all_on()),
  _collide_geom(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionNode::
CollisionNode(const CollisionNode &copy) :
  NamedNode(copy),
  _from_collide_mask(copy._from_collide_mask),
  _into_collide_mask(copy._into_collide_mask),
  _collide_geom(copy._collide_geom),
  _solids(copy._solids)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionNode::
operator = (const CollisionNode &copy) {
  NamedNode::operator = (copy);
  _from_collide_mask = copy._from_collide_mask;
  _into_collide_mask = copy._into_collide_mask;
  _collide_geom = copy._collide_geom;
  _solids = copy._solids;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionNode::
~CollisionNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *CollisionNode::
make_copy() const {
  return new CollisionNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
////////////////////////////////////////////////////////////////////
void CollisionNode::
xform(const LMatrix4f &mat) {
  nassertv(!mat.is_nan());

  if (mat.almost_equal(LMatrix4f::ident_mat())) {
    return;
  }

  Solids::iterator si;
  for (si = _solids.begin(); si != _solids.end(); ++si) {
    CollisionSolid *solid = (*si);

    // We may have to copy each of our solids as we transform them if
    // someone else is sharing their pointers.
    if (solid->get_ref_count() > 1) {
      solid = solid->make_copy();
      (*si) = solid;
    }

    solid->xform(mat);
  }
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::combine_with
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
Node *CollisionNode::
combine_with(Node *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two CollisionNodes can combine, but only if they have the same
    // name.
    CollisionNode *cother = DCAST(CollisionNode, other);
    if (get_name() == cother->get_name()) {
      const PT(CollisionSolid) *solids_begin = &cother->_solids[0];
      const PT(CollisionSolid) *solids_end = solids_begin + cother->_solids.size();
      _solids.insert(_solids.end(), solids_begin, solids_end);
      mark_bound_stale();
      return this;
    }

    // Two CollisionNodes with different names can't combine.
    return (Node *)NULL;
  }

  return NamedNode::combine_with(other);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::preserve_name
//       Access: Public, Virtual
//  Description: Returns true if the node's name has extrinsic meaning
//               and must be preserved across a flatten operation,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionNode::
preserve_name() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::draw_traverse
//       Access: Public, Virtual
//  Description: This is called by the Draw traversal by virtue of the
//               node's being present in the scene graph.  Its job is
//               to make sure the visualization of the collideable
//               geometry is up-to-date.
////////////////////////////////////////////////////////////////////
void CollisionNode::
draw_traverse() {
  Solids::iterator si;
  for (si = _solids.begin(); si != _solids.end(); ++si) {
    (*si)->update_viz(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::output
//       Access: Public, Virtual
//  Description: Writes a brief description of the node to the
//               indicated output stream.  This is invoked by the <<
//               operator.  It may be overridden in derived classes to
//               include some information relevant to the class.
////////////////////////////////////////////////////////////////////
void CollisionNode::
output(ostream &out) const {
  NamedNode::output(out);
  out << " (" << _solids.size() << " solids)";
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this node.
////////////////////////////////////////////////////////////////////
void CollisionNode::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundedObject::recompute_bound();
  nassertv(_bound != (BoundingVolume *)NULL);

  // Now actually compute the bounding volume by putting it around all
  // of our solids' bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  Solids::const_iterator gi;
  for (gi = _solids.begin(); gi != _solids.end(); ++gi) {
    child_volumes.push_back(&(*gi)->get_bound());
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();
  bool success =
    _bound->around(child_begin, child_end);

#ifdef NOTIFY_DEBUG
  if (!success) {
    collide_cat.error()
      << "Unable to generate bounding volume for " << *this << ":\n"
      << "Cannot put " << _bound->get_type() << " around:\n";
    for (int i = 0; i < (int)child_volumes.size(); i++) {
      collide_cat.error(false)
        << "  " << *child_volumes[i] << "\n";
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionNode::
write_datagram(BamWriter *manager, Datagram &me)
{
  int i;
  NamedNode::write_datagram(manager, me);
  me.add_uint16(_solids.size());

  for(i = 0; i < (int)_solids.size(); i++)
  {
    manager->write_pointer(me, _solids[i]);
  }

  me.add_uint32(_from_collide_mask.get_word());
  me.add_uint32(_into_collide_mask.get_word());
  me.add_bool(_collide_geom);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionNode::
fillin(DatagramIterator& scan, BamReader* manager)
{
  int i;
  NamedNode::fillin(scan, manager);
  int num_solids = scan.get_uint16();
  _solids.clear();
  _solids.reserve(num_solids);

  for(i = 0; i < num_solids; i++) {
    manager->read_pointer(scan, this);
    _solids.push_back((CollisionSolid *)NULL);
  }

  _from_collide_mask.set_word(scan.get_uint32());
  _into_collide_mask.set_word(scan.get_uint32());
  _collide_geom = scan.get_bool();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int CollisionNode::
complete_pointers(vector_typedWritable &p_list, BamReader* manager) {
  int num_solids = _solids.size();
  int start = NamedNode::complete_pointers(p_list, manager);

  for (int i = 0; i < num_solids; i++) {
    _solids[i] = DCAST(CollisionSolid, p_list[start + i]);
  }

  return start + num_solids;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::make_CollisionNode
//       Access: Protected
//  Description: Factory method to generate a CollisionNode object
////////////////////////////////////////////////////////////////////
TypedWritable* CollisionNode::
make_CollisionNode(const FactoryParams &params)
{
  CollisionNode *me = new CollisionNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionNode object
////////////////////////////////////////////////////////////////////
void CollisionNode::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionNode);
}




