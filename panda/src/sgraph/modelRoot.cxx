// Filename: modelRoot.cxx
// Created by:  drose (09Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "modelRoot.h"

TypeHandle ModelRoot::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *ModelRoot::
make_copy() const {
  return new ModelRoot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
void ModelRoot::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_ModelRoot);
}

////////////////////////////////////////////////////////////////////
//     Function: ModelRoot::make_ModelRoot
//       Access: Protected, Static
//  Description: Factory method to generate a node object
////////////////////////////////////////////////////////////////////
TypedWriteable* ModelRoot::
make_ModelRoot(const FactoryParams &params) {
  ModelRoot *me = new ModelRoot;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}
