// Filename: vrpnClient.cxx
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "vrpnClient.h"

TypeHandle VrpnClient::_type_handle;

#include <datagram.h>
#include <datagramIterator.h>

typedef struct {
  string device_name;
  void *self;
} VrpnClientInfo;


////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::add_remote_tracker
//       Access: Public, Virtual
//  Description: Creates a new vrpn remote tracker object and registers
//               a callback with it.
////////////////////////////////////////////////////////////////////
bool VrpnClient::
add_remote_tracker(const string &tracker, int sensor) {

  vrpn_Tracker_Remote *vrpn_tracker = new vrpn_Tracker_Remote(tracker.c_str(), _connection);
  if (vrpn_tracker == (vrpn_Tracker_Remote *)NULL) {
    return false;
  }

  //Now package up the information that needs to be passed to the
  //callback function to allow it to determine for which tracker we
  //are receiving information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = tracker;
  data->self = this;

  vrpn_tracker->register_change_handler((void*)data, st_tracker_position);
  vrpn_tracker->register_change_handler((void*)data, st_tracker_velocity);
  vrpn_tracker->register_change_handler((void*)data, st_tracker_acceleration);

  _vrpn_trackers[tracker] = vrpn_tracker;
  _trackers.push_back(tracker);
  _sensors[tracker].push_back(sensor);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::add_remote_analog
//       Access: Public, Virtual
//  Description: Creates a new vrpn remote analog object and registers
//               a callback with it.
////////////////////////////////////////////////////////////////////
bool VrpnClient::
add_remote_analog(const string &analog) {

  vrpn_Analog_Remote *vrpn_analog = new vrpn_Analog_Remote(analog.c_str(), _connection);
  if (vrpn_analog == (vrpn_Analog_Remote *)NULL) {
    return false;
  }

  //Now package up the information that needs to be passed to the
  //callback function to allow it to determine for which analog we
  //are receiving information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = analog;
  data->self = this;

  vrpn_analog->register_change_handler((void*)data, st_analog);

  _vrpn_analogs[analog] = vrpn_analog;
  _analogs.push_back(analog);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::add_remote_button
//       Access: Public, Virtual
//  Description: Creates a new vrpn remote button object and registers
//               a callback with it.
////////////////////////////////////////////////////////////////////
bool VrpnClient::
add_remote_button(const string &button) {

  vrpn_Button_Remote *vrpn_button = new vrpn_Button_Remote(button.c_str(), _connection);
  if (vrpn_button == (vrpn_Button_Remote *)NULL) {
    return false;
  }

  //Now package up the information that needs to be passed to the
  //callback function to allow it to determine for which button we
  //are receiving information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = button;
  data->self = this;

  vrpn_button->register_change_handler((void*)data, st_button);

  _vrpn_buttons[button] = vrpn_button;
  _buttons.push_back(button);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::add_remote_dial
//       Access: Public, Virtual
//  Description: Creates a new vrpn remote dial object and registers
//               a callback with it.
////////////////////////////////////////////////////////////////////
bool VrpnClient::
add_remote_dial(const string &dial) {

  vrpn_Dial_Remote *vrpn_dial = new vrpn_Dial_Remote(dial.c_str(), _connection);
  if (vrpn_dial == (vrpn_Dial_Remote *)NULL) {
    return false;
  }

  //Now package up the information that needs to be passed to the
  //callback function to allow it to determine for which dial we
  //are receiving information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = dial;
  data->self = this;

  vrpn_dial->register_change_handler((void*)data, st_dial);

  _vrpn_dials[dial] = vrpn_dial;
  _dials.push_back(dial);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::max_analog_channels
//       Access: Public, Virtual
//  Description: Max number of analog channels
////////////////////////////////////////////////////////////////////
int VrpnClient::
max_analog_channels() {
  return vrpn_CHANNEL_MAX;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::poll_trackers
//       Access: Public, Virtual
//  Description: Calls mainloop for the registered tracker object
//         Note: In a non-threaded case, this may need to come up with
//               some kind of cacheing scheme so we don't call mainloop
//               multiple times when a user is just asking for the data
//               of multiple sensors on 1 tracker (as that is the interface
//               supported).  This is a non-trivial problem as it is 
//               difficult to know when we should and shouldn't cache.
////////////////////////////////////////////////////////////////////
void VrpnClient::
poll_tracker(const string &tracker) {
  _vrpn_trackers[tracker]->mainloop();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::poll_analog
//       Access: Public, Virtual
//  Description: Calls mainloop for the registered analog object
////////////////////////////////////////////////////////////////////
void VrpnClient::
poll_analog(const string &analog) {
  _vrpn_analogs[analog]->mainloop();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::poll_button
//       Access: Public, Virtual
//  Description: Calls mainloop for the registered button object
////////////////////////////////////////////////////////////////////
void VrpnClient::
poll_button(const string &button) {
  _vrpn_buttons[button]->mainloop();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::poll_dial
//       Access: Public, Virtual
//  Description: Calls mainloop for the registered dial object
////////////////////////////////////////////////////////////////////
void VrpnClient::
poll_dial(const string &dial) {
  _vrpn_dials[dial]->mainloop();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_tracker_position
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_tracker_position(void *userdata, const vrpn_TRACKERCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_position(data->device_name, info);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_tracker_velocity
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_tracker_velocity(void *userdata, const vrpn_TRACKERVELCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_velocity(data->device_name, info);
}
////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_tracker_acceleration
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_tracker_acceleration(void *userdata, const vrpn_TRACKERACCCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_acceleration(data->device_name, info);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_analog
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_analog(void *userdata, const vrpn_ANALOGCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->analog(data->device_name, info);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_button
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_button(void *userdata, const vrpn_BUTTONCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->button(data->device_name, info);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnClient::st_dial
//       Access: Private, Static
//  Description: Callback function that merely passes the data down
//               to the appropriate non-static function
////////////////////////////////////////////////////////////////////
void VrpnClient::
st_dial(void *userdata, const vrpn_DIALCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->dial(data->device_name, info);
}
