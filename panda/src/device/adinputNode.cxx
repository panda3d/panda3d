// Filename: adinputNode.cxx
// Created by:  jason (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "adinputNode.h"
#include "config_device.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle ADInputNode::_type_handle;

TypeHandle ADInputNode::_dtime_type;
TypeHandle ADInputNode::_dial_id_type;
TypeHandle ADInputNode::_change_type;

TypeHandle ADInputNode::_btime_type;
TypeHandle ADInputNode::_button_id_type;
TypeHandle ADInputNode::_state_type;

TypeHandle ADInputNode::_atime_type;
TypeHandle ADInputNode::_channels_type;

  
////////////////////////////////////////////////////////////////////
//     Function: ADInputNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ADInputNode::
ADInputNode(PT(ClientBase) client, const string &adinput) :
  DataNode(adinput), _client(client), _adinput(adinput)
{
  _client->add_remote_analog(_adinput);
  _client->add_remote_button(_adinput);
  _client->add_remote_dial(_adinput);
  
  _dtime = new DoubleDataAttribute();
  _dial_id = new IntDataAttribute();
  _change = new DoubleDataAttribute();
  
  _btime = new DoubleDataAttribute();
  _button_id = new IntDataAttribute();
  _state = new IntDataAttribute();

  _atime = new DoubleDataAttribute();
  _channels = new DoublePtrDataAttribute();

  _adinput_attrib.set_attribute(_dtime_type, _dtime);
  _adinput_attrib.set_attribute(_dial_id_type, _dial_id);
  _adinput_attrib.set_attribute(_change_type, _change);

  _adinput_attrib.set_attribute(_btime_type, _btime);
  _adinput_attrib.set_attribute(_button_id_type, _button_id);
  _adinput_attrib.set_attribute(_state_type, _state);

  _adinput_attrib.set_attribute(_atime_type, _atime);
  _adinput_attrib.set_attribute(_channels_type, _channels);
}

////////////////////////////////////////////////////////////////////
//     Function: ADInputNode::transmit_data
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ADInputNode::
transmit_data(NodeAttributes &data) {
  AnalogData new_analog = _client->get_analog_data(_adinput);
  ButtonData new_button = _client->get_button_data(_adinput);
  DialData new_dial = _client->get_dial_data(_adinput);

  _dtime->set_value(new_dial.dtime);
  _dial_id->set_value(new_dial.dial_id);
  _change->set_value(new_dial.change);

  _btime->set_value(new_button.btime);
  _button_id->set_value(new_button.button_id);
  _state->set_value(new_button.state);

  _atime->set_value(new_analog.atime);
  _channels->set_value((double*)new_analog.channels);

  data = _adinput_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ADInputNode::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ADInputNode::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "ADInputNode",
		DataNode::get_class_type());

  DoubleDataTransition::init_type();
  IntDataTransition::init_type();
  DoublePtrDataTransition::init_type();

  register_data_transition(_dtime_type, "Dial Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_dial_id_type, "Dial ID",
			   IntDataTransition::get_class_type());
  register_data_transition(_change_type, "Dial Change",
			   DoubleDataTransition::get_class_type());

  register_data_transition(_btime_type, "Button Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_button_id_type, "Button ID",
			   IntDataTransition::get_class_type());
  register_data_transition(_state_type, "Button State",
			   IntDataTransition::get_class_type());

  register_data_transition(_atime_type, "Analog Time",
			   DoubleDataTransition::get_class_type());
  register_data_transition(_channels_type, "Analog Channels",
			   DoublePtrDataTransition::get_class_type());
}




