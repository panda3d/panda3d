// Filename: dataNode.cxx
// Created by:  drose (26Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "dataNode.h"

TypeHandle DataNode::_type_handle;
  
////////////////////////////////////////////////////////////////////
//     Function: DataNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DataNode::
DataNode(const string &name) : NamedNode(name) {
  _spam_mode = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DataNode::transmit_data_per_child
//       Access: Public, Virtual
//  Description: Should be overridden in a derived class that wants to
//               send a different data stream to each child.
//               Normally, a node only overrides transmit_data(),
//               which takes a set of input data attributes and
//               generates a set of output data attributes.  A node
//               may also override transmit_data_per_child(), which is
//               called after transmit_data(), once per child; this
//               function may be used to send individual data
//               attributes to each child.
////////////////////////////////////////////////////////////////////
void DataNode::
transmit_data_per_child(NodeAttributes &, int) {
}
  
////////////////////////////////////////////////////////////////////
//     Function: DataNode::set_spam_mode
//       Access: Public
//  Description: Sets the "spam" flag, which if set indicates that all
//               data produced from this DataNode, along with all data
//               derived from it downstream, should be reported to the
//               user.  Handy for figuring out where things are going
//               wrong.
////////////////////////////////////////////////////////////////////
void DataNode::
set_spam_mode(bool flag) {
  _spam_mode = flag;
}
  
////////////////////////////////////////////////////////////////////
//     Function: DataNode::get_spam_mode
//       Access: Public
//  Description: Gets the current state of the "spam" flag.  See
//               set_spam_mode.
////////////////////////////////////////////////////////////////////
bool DataNode::
get_spam_mode() const {
  return _spam_mode;
}


  
////////////////////////////////////////////////////////////////////
//     Function: register_data_transition
//  Description: Defines a new data transition of the indicated name
//               and inheriting from the indicated transition type.
//               This will represent a channel of data, for instance a
//               3-component position from a tracker.
//
//               All data transitions that have a common name and base
//               class will share the same TypeHandle.  This makes it
//               possible to unify data producers and consumers based
//               on the TypeHandle of the data they share.
////////////////////////////////////////////////////////////////////
void
register_data_transition(TypeHandle &type_handle, const string &name, 
                         TypeHandle derived_from) {
  // Make sure the user gave us a transition type as the base.
  nassertv(derived_from.is_derived_from(NodeTransition::get_class_type()));

  string actual_name = name + "_" + derived_from.get_name();
  type_handle = TypeRegistry::ptr()->find_type(actual_name);

  if (type_handle == TypeHandle::none()) {
    register_type(type_handle, actual_name, derived_from);
  }
}

  
