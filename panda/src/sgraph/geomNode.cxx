// Filename: geomNode.cxx
// Created by:  mike (09Jan97)
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
#include "geomNode.h"
#include "geomTransformer.h"

#include <geom.h>
#include <allTransitionsWrapper.h>
#include <indent.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle GeomNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const string &name) : NamedNode(name), _num_geoms(0) {
  _prepared_gsg = (GraphicsStateGuardianBase *)NULL;
  _prepared_context = (GeomNodeContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const GeomNode &copy) :
  NamedNode(copy),
  _geoms(copy._geoms),
  _num_geoms(0)
{
  _prepared_gsg = (GraphicsStateGuardianBase *)NULL;
  _prepared_context = (GeomNodeContext *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GeomNode::
operator = (const GeomNode &copy) {
  unprepare();
  NamedNode::operator = (copy);
  _geoms = copy._geoms;
  _num_geoms = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
~GeomNode() {
  unprepare();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *GeomNode::
make_copy() const {
  return new GeomNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
//
//               For a GeomNode, this does the right thing, but it is
//               better to use a GeomTransformer instead, since it
//               will share the new arrays properly between different
//               GeomNodes.
////////////////////////////////////////////////////////////////////
void GeomNode::
xform(const LMatrix4f &mat) {
  GeomTransformer transformer;
  transformer.transform_vertices(this, mat);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GeomNode::
write(ostream &out, int indent_level) const {
  Geoms::const_iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      indent(out, indent_level) << *DCAST(Geom, drawable) << "\n";
    } else {
      indent(out, indent_level) << drawable->get_type() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_verbose
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void GeomNode::
write_verbose(ostream &out, int indent_level) const {
  Geoms::const_iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      DCAST(Geom, drawable)->write_verbose(out, indent_level);
    } else {
      out << drawable->get_type() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::draw
//       Access: Public
//  Description: Draws all the geometry belonging to the geom node,
//               in the gsg's current state.
////////////////////////////////////////////////////////////////////
void GeomNode::
draw(GraphicsStateGuardianBase *gsg) {
  if (_prepared_gsg == gsg) {
    gsg->draw_geom_node(this, _prepared_context);
  } else {
    Geoms::const_iterator gi;
    for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
      (*gi)->draw(gsg);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::prepare
//       Access: Public
//  Description: Creates a context for the GeomNode on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) GeomNodeContext.
//
//               If the given GeomNodeContext pointer is non-NULL, it will
//               be passed to the GSG, which may or may not choose to
//               extend the existing GeomNodeContext, or create a totally
//               new one.
////////////////////////////////////////////////////////////////////
GeomNodeContext *GeomNode::
prepare(GraphicsStateGuardianBase *gsg) {
  if (gsg != _prepared_gsg) {
    GeomNodeContext *gc = gsg->prepare_geom_node(this);
    if (gc != (GeomNodeContext *)NULL) {
      unprepare();
      _prepared_context = gc;
      _prepared_gsg = gsg;
    }
    return gc;
  }

  return _prepared_context;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::unprepare
//       Access: Public
//  Description: Frees the context allocated on all GSG's for which
//               the geom has been declared.
////////////////////////////////////////////////////////////////////
void GeomNode::
unprepare() {
  if (_prepared_gsg != (GraphicsStateGuardianBase *)NULL) {
    _prepared_gsg->release_geom_node(_prepared_context);
    _prepared_gsg = (GraphicsStateGuardianBase *)NULL;
    _prepared_context = (GeomNodeContext *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::unprepare
//       Access: Public
//  Description: Frees the geom context only on the indicated GSG,
//               if it exists there.
////////////////////////////////////////////////////////////////////
void GeomNode::
unprepare(GraphicsStateGuardianBase *gsg) {
  if (_prepared_gsg == gsg) {
    _prepared_gsg->release_geom_node(_prepared_context);
    _prepared_gsg = (GraphicsStateGuardianBase *)NULL;
    _prepared_context = (GeomNodeContext *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::clear_gsg
//       Access: Public
//  Description: Removes the indicated GSG from the Geom's known
//               GSG's, without actually releasing the geom on that
//               GSG.  This is intended to be called only from
//               GSG::release_geom_node(); it should never be called by
//               user code.
////////////////////////////////////////////////////////////////////
void GeomNode::
clear_gsg(GraphicsStateGuardianBase *gsg) {
  if (_prepared_gsg == gsg) {
    _prepared_gsg = (GraphicsStateGuardianBase *)NULL;
    _prepared_context = (GeomNodeContext *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_num_geoms
//       Access: Public
//  Description: Returns the number of geoms stored in the node.
////////////////////////////////////////////////////////////////////
int GeomNode::
get_num_geoms() const {
  return _geoms.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_geom
//       Access: Public
//  Description: Returns a pointer to the nth Geom (actually,
//               Drawable) stored in the node.
////////////////////////////////////////////////////////////////////
dDrawable *GeomNode::
get_geom(int n) const {
  nassertr(n >= 0 && n < get_num_geoms(), (dDrawable *)NULL);
  return _geoms[n];
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::remove_geom
//       Access: Public
//  Description: Removes the nth Geom from the node.  All subsequent
//               index numbers are shifted down by one.
////////////////////////////////////////////////////////////////////
void GeomNode::
remove_geom(int n) {
  nassertv(n >= 0 && n < get_num_geoms());
  if (_geoms.get_ref_count() == 1) {
    // The normal case; we own the only pointer to the array.
    _geoms.erase(_geoms.begin() + n);

  } else {
    // Copy-on-write.
    int num_geoms = _geoms.size();
    Geoms new_geoms;
    new_geoms.reserve(num_geoms - 1);
    int i;
    for (i = 0; i < n; i++) {
      new_geoms.push_back(_geoms[i]);
    }
    for (i = n + 1; i < num_geoms; i++) {
      new_geoms.push_back(_geoms[i]);
    }
    nassertv(new_geoms.size() == _geoms.size());
    _geoms = new_geoms;
    nassertv(_geoms.get_ref_count() == 1);
  }

  unprepare();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::clear
//       Access: Public
//  Description: Removes all the Geoms from the node.
////////////////////////////////////////////////////////////////////
void GeomNode::
clear() {
  _geoms.clear();
  unprepare();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geom
//       Access: Public
//  Description: Adds a new Geom to the end of the list.  Returns the
//               new Geom's index number within the node.
////////////////////////////////////////////////////////////////////
int GeomNode::
add_geom(dDrawable *geom) {
  if (_geoms.get_ref_count() == 1) {
    // The normal case; we own the only pointer to the array.
    _geoms.push_back(geom);
  } else {
    // Copy on write.
    Geoms new_geoms(0);
    if (_geoms.size() != 0) {
      new_geoms.v() = _geoms.v();
    }
    _geoms = new_geoms;
    _geoms.push_back(geom);
  }

  unprepare();
  mark_bound_stale();
  return _geoms.size() - 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geoms_from
//       Access: Public
//  Description: Adds all the Geoms in the indicated GeomNode to the
//               end of the list.
////////////////////////////////////////////////////////////////////
void GeomNode::
add_geoms_from(const GeomNode *other) {
  const PT(dDrawable) *geoms_begin = &other->_geoms[0];
  const PT(dDrawable) *geoms_end = geoms_begin + other->_geoms.size();

  if (_geoms.get_ref_count() == 1) {
    // The normal case; we own the only pointer to the array.
    _geoms.v().insert(_geoms.end(), geoms_begin, geoms_end);

  } else {
    // Copy on write.
    Geoms new_geoms(0);
    if (_geoms.size() != 0) {
      new_geoms.v() = _geoms.v();
    }
    _geoms = new_geoms;
    _geoms.v().insert(_geoms.end(), geoms_begin, geoms_end);
  }

  unprepare();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this node.
////////////////////////////////////////////////////////////////////
void GeomNode::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundedObject::recompute_bound();
  assert(_bound != (BoundingVolume *)NULL);

  // Now actually compute the bounding volume by putting it around all
  // of our drawable's bounding volumes.
  pvector<const BoundingVolume *> child_volumes;

  Geoms::const_iterator gi;
  for (gi = _geoms.begin(); gi != _geoms.end(); ++gi) {
    child_volumes.push_back(&(*gi)->get_bound());
  }

  const BoundingVolume **child_begin = &child_volumes[0];
  const BoundingVolume **child_end = child_begin + child_volumes.size();
  _bound->around(child_begin, child_end);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void GeomNode::
write_datagram(BamWriter *manager, Datagram &me)
{
  NamedNode::write_datagram(manager, me);
  //Write out all of the Geom objects that this node
  //stores
  me.add_uint16(_geoms.size());
  for(int i = 0; i < (int)_geoms.size(); i++)
  {
    manager->write_pointer(me, _geoms[i]);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int GeomNode::
complete_pointers(vector_typedWritable &p_list, BamReader* manager)
{
  int start = NamedNode::complete_pointers(p_list, manager);
  for(int i = start; i < _num_geoms+start; i++)
  {
    PT(dDrawable) temp = DCAST(dDrawable, p_list[i]);
    _geoms.push_back(temp);
  }
  return start+_num_geoms;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_GeomNode
//       Access: Protected
//  Description: Factory method to generate a GeomNode object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomNode::
make_GeomNode(const FactoryParams &params)
{
  GeomNode *me = new GeomNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void GeomNode::
fillin(DatagramIterator& scan, BamReader* manager)
{
  NamedNode::fillin(scan, manager);
  _num_geoms = scan.get_uint16();
  for(int i = 0; i < _num_geoms; i++)
  {
    manager->read_pointer(scan, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomNode object
////////////////////////////////////////////////////////////////////
void GeomNode::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomNode);
}







