// Filename: clientBase.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "clientBase.h"
#include "config_device.h"

TypeHandle ClientBase::_type_handle;

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ClientBase::
ClientBase(const string &server) :
  _server(server), _sleep_time(1000000/60),
  _shutdown(false), _forked(false) 
{
}


////////////////////////////////////////////////////////////////////
//     Function: ClientBase::destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ClientBase::
~ClientBase()
{
  if (asynchronous_clients && _forked == true) {
    {
      //Make sure that you grab all locks before setting shutdown to
      //true.  This ensures that all polling actions on the thread
      //serving the devices have finished before we tell it to
      //shutdown
      mutex_lock t_lock(_tracker_lock);
      mutex_lock a_lock(_analog_lock);
      mutex_lock b_lock(_button_lock);
      _shutdown = true;
    }

    // Join the loader thread - calling process blocks until the loader
    // thread returns.
    void *ret;
    _client_thread->join(&ret);
  }
}
////////////////////////////////////////////////////////////////////
//     Function: ClientBase::fork_asynchronous_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ClientBase::
fork_asynchronous_thread(void) {
  if (asynchronous_clients) {
    _client_thread = thread::create(&st_callback, this);
    _forked = true;
    if (device_cat.is_debug()) {
      device_cat.debug()
	<< "fork_asynchronous_thread() - forking client thread"
	<< endl;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::set_poll_time
//       Access: Public
//  Description: Sets the time between polls.  The poll time is assumed
//               to be in seconds
////////////////////////////////////////////////////////////////////
void ClientBase::
set_poll_time(float poll_time) {
  _sleep_time = (int)(1000000 * poll_time);
}


////////////////////////////////////////////////////////////////////
//     Function: ClientBase::st_callback
//       Access: Private, static
//  Description: Call back function for thread (if thread has been
//               spawned).  A call back function must be static, so
//               this merely calls the non-static member callback In 
//               addition, the function has a void* return type even
//               though we don't actually return anything.  This is
//               necessary because ipc assumes a function that does 
//               not return anything indicates that the associated 
//               thread should  be created as unjoinable (detached).
////////////////////////////////////////////////////////////////////
void *ClientBase::
st_callback(void *arg) {
  nassertr(arg != NULL, NULL);
  ((ClientBase *)arg)->callback();
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::callback
//       Access: Private
//  Description: This is the main body of the sub-thread.  It sleeps
//               a certain time and then polls all devices currently
//               being watched
////////////////////////////////////////////////////////////////////
void ClientBase::
callback(void) {
  while(true) {
    if (_shutdown) {
      break;
    }
    poll_trackers();
    poll_analogs();
    poll_buttons();
    ipc_traits::sleep(0, _sleep_time);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::get_tracker_data
//       Access: Public, Virtual
//  Description: Returns the current data for the sensor of a particular
//               tracker
////////////////////////////////////////////////////////////////////
const TrackerData& ClientBase::
get_tracker_data(const string &tracker, int sensor) {

  //Very important that this check and associated call are made before
  //the mutex lock, otherwise if we are not forked and we lock before
  //doing poll we will get into a deadlock condition when the
  //associated push for this function is called
  if (!_forked) {
    poll_tracker(tracker);
  }

  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_tracker_lock);

  if ((find(_trackers.begin(), _trackers.end(), tracker) != _trackers.end()) ||
      (find(_sensors[tracker].begin(), _sensors[tracker].end(), sensor) 
                                                != _sensors[tracker].end())) {
    if (_tracker_datas.find(tracker) != _tracker_datas.end()) {
      if (_tracker_datas[tracker].find(sensor) != _tracker_datas[tracker].end()) {
	return _tracker_datas[tracker][sensor];
      }
    }
  }
  else {
    device_cat.error() << "Request for either unknown sensor " << sensor 
		       <<  " or unknown tracker " << tracker <<  " made" << endl;
  }

  return TrackerData::none();

}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::get_analog_data
//       Access: Public, Virtual
//  Description: Returns the current data for the sensor of a particular
//               analog
////////////////////////////////////////////////////////////////////
const AnalogData& ClientBase::
get_analog_data(const string &analog) {

  //Very important that this check and associated call are made before
  //the mutex lock, otherwise if we are not forked and we lock before
  //doing poll we will get into a deadlock condition when the
  //associated push for this function is called
  if (!_forked) {
    poll_analog(analog);
  }

  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_analog_lock);

  if (find(_analogs.begin(), _analogs.end(), analog) != _analogs.end()) {
    if (_analog_datas.find(analog) != _analog_datas.end()) {
      //Make sure to rotate the arrays to ensure access safety
      swap(_analog_datas[analog].channels, _analog_datas[analog].stored_channels);
      return _analog_datas[analog];
    }
  }
  else {
    device_cat.error() << "Request for unknown analog " << analog << " made" << endl;
  }

  return AnalogData::none();

}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::get_button_data
//       Access: Public, Virtual
//  Description: Returns the current data for the sensor of a particular
//               button
////////////////////////////////////////////////////////////////////
const ButtonData& ClientBase::
get_button_data(const string &button) {

  //Very important that this check and associated call are made before
  //the mutex lock, otherwise if we are not forked and we lock before
  //doing poll we will get into a deadlock condition when the
  //associated push for this function is called
  if (!_forked) {
    poll_button(button);
  }

  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_button_lock);

  if (find(_buttons.begin(), _buttons.end(), button) != _buttons.end()) {
    if (_button_datas.find(button) != _button_datas.end()) {
      return _button_datas[button];
    }
  }
  else {
    device_cat.error() << "Request for unknown button " << button << " made" << endl;
  }

  return ButtonData::none();

}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::get_dial_data
//       Access: Public, Virtual
//  Description: Returns the current data for the sensor of a particular
//               dial
////////////////////////////////////////////////////////////////////
const DialData& ClientBase::
get_dial_data(const string &dial) {

  //Very important that this check and associated call are made before
  //the mutex lock, otherwise if we are not forked and we lock before
  //doing poll we will get into a deadlock condition when the
  //associated push for this function is called
  if (!_forked) {
    poll_dial(dial);
  }

  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_dial_lock);

  if (find(_dials.begin(), _dials.end(), dial) != _dials.end()) {
    if (_dial_datas.find(dial) != _dial_datas.end()) {
      return _dial_datas[dial];
    }
  }
  else {
    device_cat.error() << "Request for unknown dial " << dial << " made" << endl;
  }

  return DialData::none();

}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_tracker_position
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               tracker data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_tracker_position(const string &tracker, const int &sensor, const double &ptime,
		      const LPoint3f &pos, const LVector4f &pquat) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_tracker_lock);  

  _tracker_datas[tracker][sensor].ptime = ptime;
  _tracker_datas[tracker][sensor].position = pos;
  _tracker_datas[tracker][sensor].pquat = pquat;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_tracker_position
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               tracker data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_tracker_velocity(const string &tracker, int sensor, const double &vtime, 
		      const LPoint3f &vel, const LVector4f &vquat, const float &dt) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_tracker_lock);  

  _tracker_datas[tracker][sensor].vtime = vtime;
  _tracker_datas[tracker][sensor].velocity = vel;
  _tracker_datas[tracker][sensor].vquat = vquat;
  _tracker_datas[tracker][sensor].vquat_dt = dt;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_tracker_position
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               tracker data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_tracker_acceleration(const string &tracker, const int &sensor, const double &atime, 
			  const LPoint3f &acc, const LVector4f &aquat, const float &dt) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_tracker_lock);  

  _tracker_datas[tracker][sensor].atime = atime;
  _tracker_datas[tracker][sensor].acceleration = acc;
  _tracker_datas[tracker][sensor].aquat = aquat;
  _tracker_datas[tracker][sensor].aquat_dt = dt;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_analog
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               analog data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_analog(const string &analog, const float &atime, 
	    const double *channels, int num_channels) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_analog_lock);  

  _analog_datas[analog].atime = atime;

  const double *head = channels;
  const double *end = head + num_channels;
  _analog_datas[analog].stored_channels->clear();
  vector_double::iterator start = _analog_datas[analog].stored_channels->begin();
  _analog_datas[analog].stored_channels->insert(start, head, end);
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_button
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               button data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_button(const string &button, const float &btime, const int &button_id, 
	    const int &state) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_button_lock);  

  _button_datas[button].btime = btime;
  _button_datas[button].button_id = button_id;
  _button_datas[button].state = state;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::push_dial
//       Access: Protected, Virtual
//  Description: Use this function in the children class to fill in
//               dial data
////////////////////////////////////////////////////////////////////
void ClientBase::
push_dial(const string &dial, const float &dtime, const int &dial_id, 
	    const float &change) {
  //Make sure to prevent simultaneous write and read of device
  mutex_lock lock(_dial_lock);  

  _dial_datas[dial].dtime = dtime;
  _dial_datas[dial].dial_id = dial_id;
  _dial_datas[dial].change = change;
}


////////////////////////////////////////////////////////////////////
//     Function: ClientBase::poll_trackers()
//       Access: Private
//  Description: Polls all current trackers
////////////////////////////////////////////////////////////////////
void ClientBase::
poll_trackers() {
  Trackers::iterator ti = _trackers.begin();
  for(; ti != _trackers.end(); ti++) {
    poll_tracker((*ti));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::poll_analogs()
//       Access: Private
//  Description: Polls all current analogs
////////////////////////////////////////////////////////////////////
void ClientBase::
poll_analogs() {
  Analogs::iterator ti = _analogs.begin();
  for(; ti != _analogs.end(); ti++) {
    poll_analog((*ti));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::poll_buttons()
//       Access: Private
//  Description: Polls all current buttons
////////////////////////////////////////////////////////////////////
void ClientBase::
poll_buttons() {
  Buttons::iterator ti = _buttons.begin();
  for(; ti != _buttons.end(); ti++) {
    poll_button((*ti));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientBase::poll_dials()
//       Access: Private
//  Description: Polls all current dials
////////////////////////////////////////////////////////////////////
void ClientBase::
poll_dials() {
  Dials::iterator ti = _dials.begin();
  for(; ti != _dials.end(); ti++) {
    poll_dial((*ti));
  }
}




