// Filename: modelNode.cxx
// Created by:  drose (09Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "modelNode.h"

#include <bamReader.h>
#include <datagram.h>
#include <datagramIterator.h>

TypeHandle ModelNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ModelNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *ModelNode::
make_copy() const {
  return new ModelNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
void ModelNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_ModelNode);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::make_ModelNode
//       Access: Protected, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
TypedWriteable* ModelNode::
make_ModelNode(const FactoryParams &params) {
  ModelNode *me = new ModelNode;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}
