/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnClient.cxx
 * @author jason
 * @date 2000-08-04
 */

#include "vrpnClient.h"
#include "vrpnTracker.h"
#include "vrpnTrackerDevice.h"
#include "vrpnButton.h"
#include "vrpnButtonDevice.h"
#include "vrpnAnalog.h"
#include "vrpnAnalogDevice.h"
#include "vrpnDial.h"
#include "vrpnDialDevice.h"
#include "config_vrpn.h"

#include "dcast.h"
#include "string_utils.h"
#include "indent.h"

using std::string;

TypeHandle VrpnClient::_type_handle;

/**
 *
 */
VrpnClient::
VrpnClient(const string &server_name) :
  _server_name(server_name)
{
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Attempting to connect to VRPN server " << _server_name
      << "\n";
  }
  _connection = vrpn_get_connection_by_name(_server_name.c_str());
  nassertv(_connection != nullptr);

  if (!is_valid()) {
    vrpn_cat.warning()
      << "Unable to establish connection to VRPN server " << _server_name
      << "\n";
  }
}

/**
 *
 */
VrpnClient::
~VrpnClient() {
  delete _connection;
}

/**
 * Writes a list of the active devices that the VrpnClient is currently
 * polling each frame.
 */
void VrpnClient::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "VrpnClient, server " << _server_name << "\n";

  if (!is_valid()) {
    indent(out, indent_level + 2)
      << "(error)\n";
  } else if (!is_connected()) {
    indent(out, indent_level + 2)
      << "(no connection)\n";
  }

  if (!_trackers.empty()) {
    indent(out, indent_level + 2)
      << _trackers.size() << " trackers:\n";
    Trackers::const_iterator ti;
    for (ti = _trackers.begin(); ti != _trackers.end(); ++ti) {
      VrpnTracker *vrpn_tracker = (*ti).second;
      vrpn_tracker->write(out, indent_level + 4);
    }
  }

  if (!_buttons.empty()) {
    indent(out, indent_level + 2)
      << _buttons.size() << " buttons:\n";
    Buttons::const_iterator bi;
    for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
      VrpnButton *vrpn_button = (*bi).second;
      vrpn_button->write(out, indent_level + 4);
    }
  }

  if (!_analogs.empty()) {
    indent(out, indent_level + 2)
      << _analogs.size() << " analogs:\n";
    Analogs::const_iterator ai;
    for (ai = _analogs.begin(); ai != _analogs.end(); ++ai) {
      VrpnAnalog *vrpn_analog = (*ai).second;
      vrpn_analog->write(out, indent_level + 4);
    }
  }

  if (!_dials.empty()) {
    indent(out, indent_level + 2)
      << _dials.size() << " dials:\n";
    Dials::const_iterator di;
    for (di = _dials.begin(); di != _dials.end(); ++di) {
      VrpnDial *vrpn_dial = (*di).second;
      vrpn_dial->write(out, indent_level + 4);
    }
  }
}

/**
 * Creates and returns a new ClientDevice of the appropriate type, according
 * to the requested device_type and device_name.  Returns NULL if a matching
 * device cannot be found.
 *
 * This is guaranteed not to be called twice for a given
 * device_type/device_name combination (unless disconnect_device() has already
 * been called for the same device_type/device_name).
 */
PT(ClientDevice) VrpnClient::
make_device(TypeHandle device_type, const string &device_name) {
  if (device_type == ClientTrackerDevice::get_class_type()) {
    return make_tracker_device(device_name);

  } else if (device_type == ClientButtonDevice::get_class_type()) {
    return make_button_device(device_name);

  } else if (device_type == ClientAnalogDevice::get_class_type()) {
    return make_analog_device(device_name);

  } else if (device_type == ClientDialDevice::get_class_type()) {
    return make_dial_device(device_name);

  } else {
    return nullptr;
  }
}

/**
 * Removes the device, which is presumably about to destruct, from the list of
 * connected devices, and frees any data required to support it.  This device
 * will no longer receive automatic updates with each poll.
 *
 * The return value is true if the device was disconnected, or false if it was
 * unknown (e.g.  it was disconnected previously).
 */
bool VrpnClient::
disconnect_device(TypeHandle device_type, const string &device_name,
                  ClientDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Disconnecting device " << *device << "\n";
  }

  if (ClientBase::disconnect_device(device_type, device_name, device)) {
    if (device->is_of_type(VrpnTrackerDevice::get_class_type())) {
      disconnect_tracker_device(DCAST(VrpnTrackerDevice, device));

    } else if (device->is_of_type(VrpnButtonDevice::get_class_type())) {
      disconnect_button_device(DCAST(VrpnButtonDevice, device));

    } else if (device->is_of_type(VrpnAnalogDevice::get_class_type())) {
      disconnect_analog_device(DCAST(VrpnAnalogDevice, device));

    } else if (device->is_of_type(VrpnDialDevice::get_class_type())) {
      disconnect_dial_device(DCAST(VrpnDialDevice, device));

    }
    return true;
  }

  return false;
}

/**
 * Implements the polling and updating of connected devices, if the ClientBase
 * requires this.  This may be called in a sub-thread if
 * fork_asynchronous_thread() was called; otherwise, it will be called once
 * per frame.
 */
void VrpnClient::
do_poll() {
  ClientBase::do_poll();

  if (vrpn_cat.is_spam()) {
    vrpn_cat.spam()
      << "VrpnClient " << _server_name << " polling "
      << _trackers.size() + _buttons.size() + _analogs.size() + _dials.size()
      << " devices.\n";
  }

  Trackers::iterator ti;
  for (ti = _trackers.begin(); ti != _trackers.end(); ++ti) {
    VrpnTracker *vrpn_tracker = (*ti).second;
    vrpn_tracker->poll();
  }

  Buttons::iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    VrpnButton *vrpn_button = (*bi).second;
    vrpn_button->poll();
  }

  Analogs::iterator ai;
  for (ai = _analogs.begin(); ai != _analogs.end(); ++ai) {
    VrpnAnalog *vrpn_analog = (*ai).second;
    vrpn_analog->poll();
  }

  Dials::iterator di;
  for (di = _dials.begin(); di != _dials.end(); ++di) {
    VrpnDial *vrpn_dial = (*di).second;
    vrpn_dial->poll();
  }
}

/**
 * Creates a new tracker device.  The device_name is parsed for sensor and
 * data_type information.
 *
 * The device_name may be one of the following:
 *
 * tracker_name tracker_name:N tracker_name:N[pva]
 *
 * Where N is an integer sensor number, and [pva] is one of the lowercase
 * letters p, v, or a.
 *
 * In the first form, the device connects to the indicated tracker, and
 * reports position information on sensor number 0.
 *
 * In the second form, the device connects to the indicated tracker, and
 * reports position information on the indicated sensor number.
 *
 * In the third form, the device connects to the indicated tracker, and
 * reports either position, velocity, or acceleration information on the
 * indicated sensor number.
 */
PT(ClientDevice) VrpnClient::
make_tracker_device(const string &device_name) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Making tracker device for " << device_name << "\n";
  }

  string tracker_name = device_name;
  int sensor = 0;
  VrpnTrackerDevice::DataType data_type = VrpnTrackerDevice::DT_position;

  size_t colon = device_name.rfind(':');
  if (colon != string::npos && colon + 1 < device_name.length()) {
    size_t begin = colon + 1;
    size_t end = device_name.length();
    VrpnTrackerDevice::DataType maybe_data_type = data_type;

    switch (device_name[end - 1]) {
    case 'p':
      maybe_data_type = VrpnTrackerDevice::DT_position;
      end--;
      break;

    case 'v':
      maybe_data_type = VrpnTrackerDevice::DT_velocity;
      end--;
      break;

    case 'a':
      maybe_data_type = VrpnTrackerDevice::DT_acceleration;
      end--;
      break;
    }
    int maybe_sensor;
    if (string_to_int(device_name.substr(begin, end - begin), maybe_sensor)) {
      // It seems to be a legitimate integer!
      sensor = maybe_sensor;
      data_type = maybe_data_type;
      tracker_name = device_name.substr(0, colon);
    }
  }

  VrpnTracker *tracker = get_tracker(tracker_name);

  VrpnTrackerDevice *device =
    new VrpnTrackerDevice(this, device_name, sensor, data_type, tracker);

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating " << *device << "\n";
  }

  tracker->mark(device);
  return device;
}

/**
 * Creates a new button device.  The device_name is sent verbatim to the VRPN
 * library.
 */
PT(ClientDevice) VrpnClient::
make_button_device(const string &device_name) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Making button device for " << device_name << "\n";
  }

  VrpnButton *button = get_button(device_name);

  VrpnButtonDevice *device =
    new VrpnButtonDevice(this, device_name, button);

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating " << *device << "\n";
  }

  button->mark(device);
  return device;
}

/**
 * Creates a new analog device.  The device_name is sent verbatim to the VRPN
 * library.
 */
PT(ClientDevice) VrpnClient::
make_analog_device(const string &device_name) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Making analog device for " << device_name << "\n";
  }

  VrpnAnalog *analog = get_analog(device_name);

  VrpnAnalogDevice *device =
    new VrpnAnalogDevice(this, device_name, analog);

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating " << *device << "\n";
  }

  analog->mark(device);
  return device;
}

/**
 * Creates a new dial device.  The device_name is sent verbatim to the VRPN
 * library.
 */
PT(ClientDevice) VrpnClient::
make_dial_device(const string &device_name) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Making dial device for " << device_name << "\n";
  }

  VrpnDial *dial = get_dial(device_name);

  VrpnDialDevice *device =
    new VrpnDialDevice(this, device_name, dial);

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating " << *device << "\n";
  }

  dial->mark(device);
  return device;
}

/**
 * Removes the tracker device from the list of things to be updated.
 */
void VrpnClient::
disconnect_tracker_device(VrpnTrackerDevice *device) {
  VrpnTracker *vrpn_tracker = device->get_vrpn_tracker();
  vrpn_tracker->unmark(device);
  if (vrpn_tracker->is_empty()) {
    free_tracker(vrpn_tracker);
  }
}

/**
 * Removes the button device from the list of things to be updated.
 */
void VrpnClient::
disconnect_button_device(VrpnButtonDevice *device) {
  VrpnButton *vrpn_button = device->get_vrpn_button();
  vrpn_button->unmark(device);
  if (vrpn_button->is_empty()) {
    free_button(vrpn_button);
  }
}

/**
 * Removes the analog device from the list of things to be updated.
 */
void VrpnClient::
disconnect_analog_device(VrpnAnalogDevice *device) {
  VrpnAnalog *vrpn_analog = device->get_vrpn_analog();
  vrpn_analog->unmark(device);
  if (vrpn_analog->is_empty()) {
    free_analog(vrpn_analog);
  }
}

/**
 * Removes the dial device from the list of things to be updated.
 */
void VrpnClient::
disconnect_dial_device(VrpnDialDevice *device) {
  VrpnDial *vrpn_dial = device->get_vrpn_dial();
  vrpn_dial->unmark(device);
  if (vrpn_dial->is_empty()) {
    free_dial(vrpn_dial);
  }
}

/**
 * Finds a VrpnTracker of the indicated name, and returns it if one already
 * exists, or creates a new one if it does not.
 */
VrpnTracker *VrpnClient::
get_tracker(const string &tracker_name) {
  Trackers::iterator ti;
  ti = _trackers.find(tracker_name);

  if (ti != _trackers.end()) {
    return (*ti).second;
  }

  VrpnTracker *vrpn_tracker = new VrpnTracker(tracker_name, _connection);
  _trackers.insert(Trackers::value_type(tracker_name, vrpn_tracker));

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating tracker " << *vrpn_tracker << "\n";
  }

  return vrpn_tracker;
}

/**
 * Removes and deletes the indicated VrpnTracker, which is no longer
 * referenced by any VrpnTrackerDevices.
 */
void VrpnClient::
free_tracker(VrpnTracker *vrpn_tracker) {
  nassertv(vrpn_tracker->is_empty());

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Deleting tracker " << *vrpn_tracker << "\n";
  }

  Trackers::iterator ti;
  ti = _trackers.find(vrpn_tracker->get_tracker_name());
  nassertv(ti != _trackers.end());
  nassertv((*ti).second == vrpn_tracker);

  _trackers.erase(ti);
  delete vrpn_tracker;
}

/**
 * Finds a VrpnButton of the indicated name, and returns it if one already
 * exists, or creates a new one if it does not.
 */
VrpnButton *VrpnClient::
get_button(const string &button_name) {
  Buttons::iterator bi;
  bi = _buttons.find(button_name);

  if (bi != _buttons.end()) {
    return (*bi).second;
  }

  VrpnButton *vrpn_button = new VrpnButton(button_name, _connection);
  _buttons.insert(Buttons::value_type(button_name, vrpn_button));

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating button " << *vrpn_button << "\n";
  }

  return vrpn_button;
}

/**
 * Removes and deletes the indicated VrpnButton, which is no longer referenced
 * by any VrpnButtonDevices.
 */
void VrpnClient::
free_button(VrpnButton *vrpn_button) {
  nassertv(vrpn_button->is_empty());

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Deleting button " << *vrpn_button << "\n";
  }

  Buttons::iterator bi;
  bi = _buttons.find(vrpn_button->get_button_name());
  nassertv(bi != _buttons.end());
  nassertv((*bi).second == vrpn_button);

  _buttons.erase(bi);
  delete vrpn_button;
}

/**
 * Finds a VrpnAnalog of the indicated name, and returns it if one already
 * exists, or creates a new one if it does not.
 */
VrpnAnalog *VrpnClient::
get_analog(const string &analog_name) {
  Analogs::iterator ai;
  ai = _analogs.find(analog_name);

  if (ai != _analogs.end()) {
    return (*ai).second;
  }

  VrpnAnalog *vrpn_analog = new VrpnAnalog(analog_name, _connection);
  _analogs.insert(Analogs::value_type(analog_name, vrpn_analog));

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating analog " << *vrpn_analog << "\n";
  }

  return vrpn_analog;
}

/**
 * Removes and deletes the indicated VrpnAnalog, which is no longer referenced
 * by any VrpnAnalogDevices.
 */
void VrpnClient::
free_analog(VrpnAnalog *vrpn_analog) {
  nassertv(vrpn_analog->is_empty());

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Deleting analog " << *vrpn_analog << "\n";
  }

  Analogs::iterator ai;
  ai = _analogs.find(vrpn_analog->get_analog_name());
  nassertv(ai != _analogs.end());
  nassertv((*ai).second == vrpn_analog);

  _analogs.erase(ai);
  delete vrpn_analog;
}

/**
 * Finds a VrpnDial of the indicated name, and returns it if one already
 * exists, or creates a new one if it does not.
 */
VrpnDial *VrpnClient::
get_dial(const string &dial_name) {
  Dials::iterator di;
  di = _dials.find(dial_name);

  if (di != _dials.end()) {
    return (*di).second;
  }

  VrpnDial *vrpn_dial = new VrpnDial(dial_name, _connection);
  _dials.insert(Dials::value_type(dial_name, vrpn_dial));

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Creating dial " << *vrpn_dial << "\n";
  }

  return vrpn_dial;
}

/**
 * Removes and deletes the indicated VrpnDial, which is no longer referenced
 * by any VrpnDialDevices.
 */
void VrpnClient::
free_dial(VrpnDial *vrpn_dial) {
  nassertv(vrpn_dial->is_empty());

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << "Deleting dial " << *vrpn_dial << "\n";
  }

  Dials::iterator di;
  di = _dials.find(vrpn_dial->get_dial_name());
  nassertv(di != _dials.end());
  nassertv((*di).second == vrpn_dial);

  _dials.erase(di);
  delete vrpn_dial;
}


#if 0

#include "datagram.h"
#include "datagramIterator.h"

typedef struct {
  string device_name;
  void *self;
} VrpnClientInfo;


/**
 * Creates a new vrpn remote tracker object and registers a callback with it.
 */
bool VrpnClient::
add_remote_tracker(const string &tracker, int sensor) {

  vrpn_Tracker_Remote *vrpn_tracker = new vrpn_Tracker_Remote(tracker.c_str(), _connection);
  if (vrpn_tracker == nullptr) {
    return false;
  }

  // Now package up the information that needs to be passed to the callback
  // function to allow it to determine for which tracker we are receiving
  // information for
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

/**
 * Creates a new vrpn remote analog object and registers a callback with it.
 */
bool VrpnClient::
add_remote_analog(const string &analog) {

  vrpn_Analog_Remote *vrpn_analog = new vrpn_Analog_Remote(analog.c_str(), _connection);
  if (vrpn_analog == nullptr) {
    return false;
  }

  // Now package up the information that needs to be passed to the callback
  // function to allow it to determine for which analog we are receiving
  // information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = analog;
  data->self = this;

  vrpn_analog->register_change_handler((void*)data, st_analog);

  _vrpn_analogs[analog] = vrpn_analog;
  _analogs.push_back(analog);

  return true;
}

/**
 * Creates a new vrpn remote button object and registers a callback with it.
 */
bool VrpnClient::
add_remote_button(const string &button) {

  vrpn_Button_Remote *vrpn_button = new vrpn_Button_Remote(button.c_str(), _connection);
  if (vrpn_button == nullptr) {
    return false;
  }

  // Now package up the information that needs to be passed to the callback
  // function to allow it to determine for which button we are receiving
  // information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = button;
  data->self = this;

  vrpn_button->register_change_handler((void*)data, st_button);

  _vrpn_buttons[button] = vrpn_button;
  _buttons.push_back(button);

  return true;
}

/**
 * Creates a new vrpn remote dial object and registers a callback with it.
 */
bool VrpnClient::
add_remote_dial(const string &dial) {

  vrpn_Dial_Remote *vrpn_dial = new vrpn_Dial_Remote(dial.c_str(), _connection);
  if (vrpn_dial == nullptr) {
    return false;
  }

  // Now package up the information that needs to be passed to the callback
  // function to allow it to determine for which dial we are receiving
  // information for
  VrpnClientInfo *data = new VrpnClientInfo;
  data->device_name = dial;
  data->self = this;

  vrpn_dial->register_change_handler((void*)data, st_dial);

  _vrpn_dials[dial] = vrpn_dial;
  _dials.push_back(dial);

  return true;
}

/**
 * Max number of analog channels
 */
int VrpnClient::
max_analog_channels() {
  return vrpn_CHANNEL_MAX;
}

/**
 * Calls mainloop for the registered tracker object Note: In a non-threaded
 * case, this may need to come up with some kind of cacheing scheme so we
 * don't call mainloop multiple times when a user is just asking for the data
 * of multiple sensors on 1 tracker (as that is the interface supported).
 * This is a non-trivial problem as it is difficult to know when we should and
 * shouldn't cache.
 */
void VrpnClient::
poll_tracker(const string &tracker) {
  _vrpn_trackers[tracker]->mainloop();
}

/**
 * Calls mainloop for the registered analog object
 */
void VrpnClient::
poll_analog(const string &analog) {
  _vrpn_analogs[analog]->mainloop();
}

/**
 * Calls mainloop for the registered button object
 */
void VrpnClient::
poll_button(const string &button) {
  _vrpn_buttons[button]->mainloop();
}

/**
 * Calls mainloop for the registered dial object
 */
void VrpnClient::
poll_dial(const string &dial) {
  _vrpn_dials[dial]->mainloop();
}

/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VRPN_CALLBACK VrpnClient::
st_tracker_position(void *userdata, const vrpn_TRACKERCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_position(data->device_name, info);
}

/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VRPN_CALLBACK VrpnClient::
st_tracker_velocity(void *userdata, const vrpn_TRACKERVELCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_velocity(data->device_name, info);
}
/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VRPN_CALLBACK VrpnClient::
st_tracker_acceleration(void *userdata, const vrpn_TRACKERACCCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->tracker_acceleration(data->device_name, info);
}

/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VrpnClient::
st_analog(void *userdata, const vrpn_ANALOGCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->analog(data->device_name, info);
}

/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VrpnClient::
st_button(void *userdata, const vrpn_BUTTONCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->button(data->device_name, info);
}

/**
 * Callback function that merely passes the data down to the appropriate non-
 * static function
 */
void VrpnClient::
st_dial(void *userdata, const vrpn_DIALCB info) {
  VrpnClientInfo *data = (VrpnClientInfo *)userdata;
  ((VrpnClient *)data->self)->dial(data->device_name, info);
}

#endif
