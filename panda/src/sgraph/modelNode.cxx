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
//     Function: ModelNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool ModelNode::
safe_to_transform() const {
  return !_preserve_transform;
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
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::write_datagram
//       Access: Public, Virtual
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ModelNode::
write_datagram(BamWriter *manager, Datagram &me) {
  NamedNode::write_datagram(manager, me);

  me.add_bool(_preserve_transform);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void ModelNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  NamedNode::fillin(scan, manager);

  if (manager->get_file_minor_ver() < 2) {
    // No _preserve_transform before bams 3.2.
    _preserve_transform = false;
  } else {
    _preserve_transform = scan.get_bool();
  }
  
}
