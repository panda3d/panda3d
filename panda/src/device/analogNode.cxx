// Filename: analogNode.cxx
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "analogNode.h"
#include "config_device.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle AnalogNode::_type_handle;

TypeHandle AnalogNode::_xyz_type;
  
////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnalogNode::
AnalogNode(ClientBase *client, const string &device_name) :
  DataNode(device_name)
{
  nassertv(client != (ClientBase *)NULL);
  PT(ClientDevice) device = 
    client->get_device(ClientAnalogDevice::get_class_type(), device_name);

  if (device == (ClientDevice *)NULL) {
    device_cat.warning()
      << "Unable to open analog device " << device_name << "\n";
    return;
  }

  if (!device->is_of_type(ClientAnalogDevice::get_class_type())) {
    device_cat.error()
      << "Inappropriate device type " << device->get_type()
      << " created; expected a ClientAnalogDevice.\n";
    return;
  }

  _analog = DCAST(ClientAnalogDevice, device);

  _xyz = new Vec3DataAttribute(LPoint3f(0, 0, 0));
  _attrib.set_attribute(_xyz_type, _xyz);
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
AnalogNode::
~AnalogNode() {
  // When the _analog pointer destructs, the ClientAnalogDevice
  // disconnects itself from the ClientBase, and everything that needs
  // to get turned off does.  Magic.
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AnalogNode::
write(ostream &out, int indent_level) const {
  DataNode::write(out, indent_level);

  if (_analog != (ClientAnalogDevice *)NULL) {
    _analog->write_controls(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AnalogNode::
transmit_data(NodeAttributes &data) {
  if (is_valid()) {
    _analog->poll();

    LPoint3f out(0.0, 0.0, 0.0);

    for (int i = 0; i < max_outputs; i++) {
      if (_outputs[i]._index >= 0 && is_control_known(_outputs[i]._index)) {
	if (_outputs[i]._flip) {
	  out[i] = -get_control_state(_outputs[i]._index);
	} else {
	  out[i] = get_control_state(_outputs[i]._index);
	}
      }
    }
    _xyz->set_value(out);
  }

  data = _attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AnalogNode::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AnalogNode::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "AnalogNode",
		DataNode::get_class_type());

  Vec3DataTransition::init_type();
  register_data_transition(_xyz_type, "XYZ",
			   Vec3DataTransition::get_class_type());
}

