// Filename: computeNode.cxx
// Created by:  rdb (19Jun14)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "computeNode.h"
#include "cullTraverser.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "geomDrawCallbackData.h"
#include "omniBoundingVolume.h"
#include "config_pgraph.h"

TypeHandle ComputeNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Constructor
//       Access: Published
//  Description: Creates a ComputeNode with the given name.  Use
//               add_dispatch and  also assign a shader using a
//               ShaderAttrib.
////////////////////////////////////////////////////////////////////
ComputeNode::
ComputeNode(const string &name) :
  PandaNode(name),
  _dispatcher(new ComputeNode::Dispatcher)
{
  set_internal_bounds(new OmniBoundingVolume);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
ComputeNode::
ComputeNode(const ComputeNode &copy) :
  PandaNode(copy),
  _dispatcher(new ComputeNode::Dispatcher(*copy._dispatcher))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *ComputeNode::
make_copy() const {
  return new ComputeNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool ComputeNode::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::is_renderable
//       Access: Public, Virtual
//  Description: Returns true if there is some value to visiting this
//               particular node during the cull traversal for any
//               camera, false otherwise.  This will be used to
//               optimize the result of get_net_draw_show_mask(), so
//               that any subtrees that contain only nodes for which
//               is_renderable() is false need not be visited.
////////////////////////////////////////////////////////////////////
bool ComputeNode::
is_renderable() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::add_for_draw
//       Access: Public, Virtual
//  Description: Adds the node's contents to the CullResult we are
//               building up during the cull traversal, so that it
//               will be drawn at render time.  For most nodes other
//               than GeomNodes, this is a do-nothing operation.
////////////////////////////////////////////////////////////////////
void ComputeNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Found " << *this << " in state " << *data._state
      << " draw_mask = " << data._draw_mask << "\n";
  }

  // OK, render this node.  Rendering this node means creating a
  // CullableObject for the Dispatcher.  We don't need to pass
  // any Geoms, however.
  CullableObject *object =
    new CullableObject(NULL, data._state,
                       data.get_internal_transform(trav));
  object->set_draw_callback(_dispatcher);
  trav->get_cull_handler()->record_object(object, trav);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::output
//       Access: Public, Virtual
//  Description: Writes a brief description of the node to the
//               indicated output stream.  This is invoked by the <<
//               operator.  It may be overridden in derived classes to
//               include some information relevant to the class.
////////////////////////////////////////////////////////////////////
void ComputeNode::
output(ostream &out) const {
  PandaNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ComputeNode::Dispatcher::
Dispatcher() {
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ComputeNode::Dispatcher::
Dispatcher(const Dispatcher &copy) :
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::do_callback
//       Access: Public, Virtual
//  Description: Asks the GSG to dispatch the compute shader.
////////////////////////////////////////////////////////////////////
void ComputeNode::Dispatcher::
do_callback(CallbackData *cbdata) {
  GeomDrawCallbackData *data = (GeomDrawCallbackData *)cbdata;
  GraphicsStateGuardianBase *gsg = data->get_gsg();

  CDReader cdata(_cycler);

  Dispatches::const_iterator it;
  for (it = cdata->_dispatches.begin(); it != cdata->_dispatches.end(); ++it) {
    gsg->dispatch_compute(it->get_x(), it->get_y(), it->get_z());
  }

  // No need to upcall; we don't have any geometry, after all.
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ComputeNode.
////////////////////////////////////////////////////////////////////
void ComputeNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ComputeNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _dispatcher->_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ComputeNode is encountered
//               in the Bam file.  It should create the ComputeNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ComputeNode::
make_from_bam(const FactoryParams &params) {
  ComputeNode *node = new ComputeNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ComputeNode.
////////////////////////////////////////////////////////////////////
void ComputeNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _dispatcher->_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *ComputeNode::Dispatcher::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ComputeNode::Dispatcher::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint16(_dispatches.size());

  Dispatches::const_iterator it;
  for (it = _dispatches.begin(); it != _dispatches.end(); ++it) {
    generic_write_datagram(dg, *it);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ComputeNode::Dispatcher::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ComputeNode.
////////////////////////////////////////////////////////////////////
void ComputeNode::Dispatcher::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  int num_dispatches = scan.get_uint16();
  _dispatches.resize(num_dispatches);

  for (int i = 0; i < num_dispatches; ++i) {
    generic_read_datagram(_dispatches[i], scan);
  }
}
