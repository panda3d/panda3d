/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file callbackNode.cxx
 * @author drose
 * @date 2009-03-13
 */

#include "pandabase.h"
#include "callbackNode.h"
#include "cullTraverser.h"
#include "nodeCullCallbackData.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "omniBoundingVolume.h"
#include "config_pgraph.h"

TypeHandle CallbackNode::_type_handle;

/**
 *
 */
CallbackNode::
CallbackNode(const std::string &name) :
  PandaNode(name)
{
  PandaNode::set_cull_callback();

  // Set up a default, infinite bounding volume, unless the user tells us
  // otherwise.  Not sure if this is a great idea, because it means a naive
  // user will never set the bounding volume and always trigger the callback--
  // but that's not altogether a bad default behavior.
  set_internal_bounds(new OmniBoundingVolume);
}

/**
 *
 */
CallbackNode::
CallbackNode(const CallbackNode &copy) :
  PandaNode(copy),
  _cycler(copy._cycler)
{
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *CallbackNode::
make_copy() const {
  return new CallbackNode(*this);
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool CallbackNode::
safe_to_combine() const {
  return false;
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool CallbackNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  CallbackObject *cbobj = get_cull_callback();
  if (cbobj != nullptr) {
    NodeCullCallbackData cbdata(trav, data);
    cbobj->do_callback(&cbdata);

    // No further cull: the callback takes care of all of it.
    return false;
  }

  // Recurse onto the node's children.
  return true;
}

/**
 * Returns true if there is some value to visiting this particular node during
 * the cull traversal for any camera, false otherwise.  This will be used to
 * optimize the result of get_net_draw_show_mask(), so that any subtrees that
 * contain only nodes for which is_renderable() is false need not be visited.
 */
bool CallbackNode::
is_renderable() const {
  return true;
}

/**
 * Adds the node's contents to the CullResult we are building up during the
 * cull traversal, so that it will be drawn at render time.  For most nodes
 * other than GeomNodes, this is a do-nothing operation.
 */
void CallbackNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Found " << *this << " in state " << *data._state
      << " draw_mask = " << data._draw_mask << "\n";
  }

  // OK, render this node.  Rendering this node means creating a
  // CullableObject for the draw_callback, if any.  We don't need to pass any
  // Geoms, however.
  CallbackObject *cbobj = get_draw_callback();
  if (cbobj != nullptr) {
    CullableObject *object =
      new CullableObject(nullptr, data._state,
                         data.get_internal_transform(trav));
    object->set_draw_callback(cbobj);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

/**
 * Writes a brief description of the node to the indicated output stream.
 * This is invoked by the << operator.  It may be overridden in derived
 * classes to include some information relevant to the class.
 */
void CallbackNode::
output(std::ostream &out) const {
  PandaNode::output(out);
}

/**
 * Tells the BamReader how to create objects of type CallbackNode.
 */
void CallbackNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CallbackNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CallbackNode is encountered in the Bam file.  It should create the
 * CallbackNode and extract its information from the file.
 */
TypedWritable *CallbackNode::
make_from_bam(const FactoryParams &params) {
  CallbackNode *node = new CallbackNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CallbackNode.
 */
void CallbackNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
CycleData *CallbackNode::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CallbackNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CallbackNode.
 */
void CallbackNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
}
