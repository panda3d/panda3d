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

TypeHandle TrackerNode::_ptime_type;
TypeHandle TrackerNode::_position_type;
TypeHandle TrackerNode::_pquat_type;

TypeHandle TrackerNode::_vtime_type;
TypeHandle TrackerNode::_velocity_type;
TypeHandle TrackerNode::_vquat_type;
TypeHandle TrackerNode::_vquat_dt_type;

TypeHandle TrackerNode::_atime_type;
TypeHandle TrackerNode::_acceleration_type;
TypeHandle TrackerNode::_aquat_type;
TypeHandle TrackerNode::_aquat_dt_type;
  
////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TrackerNode::
TrackerNode(PT(ClientBase) client, const string &tracker, 
	    int sensor) :
  DataNode(tracker), _client(client), _tracker(tracker), _sensor(sensor)
{
  _client->add_remote_tracker(_tracker, _sensor);
  
  _ptime = new DoubleDataAttribute();
  _position = new Vec3DataAttribute(LPoint3f(0,0,0));
  _pquat = new Vec4DataAttribute(LVector4f(0,0,0,0));

  _vtime = new DoubleDataAttribute();
  _velocity = new Vec3DataAttribute(LPoint3f(0,0,0));
  _vquat = new Vec4DataAttribute(LVector4f(0,0,0,0));
  _vquat_dt = new DoubleDataAttribute();

  _atime = new DoubleDataAttribute();
  _acceleration = new Vec3DataAttribute(LPoint3f(0,0,0));
  _aquat = new Vec4DataAttribute(LVector4f(0,0,0,0));
  _aquat_dt = new DoubleDataAttribute();

  _tracker_attrib.set_attribute(_ptime_type, _ptime);
  _tracker_attrib.set_attribute(_position_type, _position);
  _tracker_attrib.set_attribute(_pquat_type, _pquat);

  _tracker_attrib.set_attribute(_vtime_type, _vtime);
  _tracker_attrib.set_attribute(_velocity_type, _velocity);
  _tracker_attrib.set_attribute(_vquat_type, _vquat);
  _tracker_attrib.set_attribute(_vquat_dt_type, _vquat_dt);

  _tracker_attrib.set_attribute(_atime_type, _atime);
  _tracker_attrib.set_attribute(_acceleration_type, _acceleration);
  _tracker_attrib.set_attribute(_aquat_type, _aquat);
  _tracker_attrib.set_attribute(_aquat_dt_type, _aquat_dt);
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TrackerNode::
transmit_data(NodeAttributes &data) {
  TrackerData new_data = _client->get_tracker_data(_tracker, _sensor);

  if (device_cat.is_debug()) {
    device_cat.debug() << "TrackerNode:transmit_data" << endl;
  }

  _ptime->set_value(new_data.ptime);
  _position->set_value(new_data.position);
  _pquat->set_value(new_data.pquat);

  _vtime->set_value(new_data.vtime);
  _velocity->set_value(new_data.velocity);
  _vquat->set_value(new_data.vquat);
  _vquat_dt->set_value(new_data.vquat_dt);

  _atime->set_value(new_data.atime);
  _acceleration->set_value(new_data.acceleration);
  _aquat->set_value(new_data.aquat);
  _aquat_dt->set_value(new_data.aquat_dt);

  if (device_cat.is_debug()) {
    device_cat.debug() << "TrackerNode:attributes" << endl;
    _tracker_attrib.write(device_cat.debug(false), 3);
  }

  data = _tracker_attrib;
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

  DoubleDataTransition::init_type();
  Vec3DataTransition::init_type();
  Vec4DataTransition::init_type();

  register_data_transition(_ptime_type, "Position Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_position_type, "Position",
			   Vec3DataTransition::get_class_type());
  register_data_transition(_pquat_type, "Position Quat",
			   Vec4DataTransition::get_class_type());

  register_data_transition(_vtime_type, "Velocity Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_velocity_type, "Velocity",
			   Vec3DataTransition::get_class_type());
  register_data_transition(_vquat_type, "Velocity Quat",
			   Vec4DataTransition::get_class_type());
  register_data_transition(_vquat_dt_type, "Velocity Quat dt",
			   DoubleDataTransition::get_class_type());

  register_data_transition(_atime_type, "Acceleration Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_acceleration_type, "Acceleration",
			   Vec3DataTransition::get_class_type());
  register_data_transition(_aquat_type, "Acceleration Quat",
			   Vec4DataTransition::get_class_type());
  register_data_transition(_aquat_dt_type, "Acceleration Quat dt",
			   DoubleDataTransition::get_class_type());
}













