// Filename: trackerNode.cxx
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "trackerNode.h"
#include "config_device.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle TrackerNode::_type_handle;

TypeHandle TrackerNode::_transform_type;
  
////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TrackerNode::
TrackerNode(ClientBase *client, const string &device_name) :
  DataNode(device_name)
{
  nassertv(client != (ClientBase *)NULL);
  PT(ClientDevice) device = 
    client->get_device(ClientTrackerDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
    device_cat.warning()
      << "Unable to open tracker device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientTrackerDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientTrackerDevice.\n";
    return;
  }

  _tracker = DCAST(ClientTrackerDevice, device);

  if (_tracker != (ClientTrackerDevice *)NULL) {
    _transform_attrib = new MatrixDataAttribute;
    _attrib.set_attribute(_transform_type, _transform_attrib);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TrackerNode::
~TrackerNode() {
  // When the _tracker pointer destructs, the ClientTrackerDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TrackerNode::
transmit_data(NodeAttributes &data) {
  if (is_valid()) {
    _tracker->poll();
    _tracker->lock();
    _data = _tracker->get_data();
    _tracker->unlock();

    _data.get_orient().extract_to_matrix(_transform);
    _transform.set_row(3, _data.get_pos());

    _transform_attrib->set_value(_transform);

    if (device_cat.is_debug()) {
      device_cat.debug() << "TrackerNode:attributes" << endl;
      _attrib.write(device_cat.debug(false), 3);
    }
  }

  data = _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TrackerNode::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "TrackerNode",
		DataNode::get_class_type());

  MatrixDataTransition::init_type();
  register_data_transition(_transform_type, "Transform",
			   MatrixDataTransition::get_class_type());
}

