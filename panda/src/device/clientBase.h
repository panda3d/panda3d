// Filename: clientBase.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _CLIENT_BASE
#define _CLIENT_BASE

#include <pandabase.h>

#include "trackerData.h"
#include "analogData.h"
#include "buttonData.h"
#include "dialData.h"

#include <typedReferenceCount.h>
#include <luse.h>
#include <vector_string.h>
#include <vector_int.h>

#ifdef HAVE_IPC
#include <ipc_mutex.h>
#include <ipc_condition.h>
#include <ipc_thread.h>
#endif

#include <map>

class EXPCL_PANDA ClientBase : public TypedReferenceCount {
public:
  ClientBase(const string &server);
  ~ClientBase();
  void fork_asynchronous_thread(void);
  void set_poll_time(float poll_time);

  //ADD FUNCTIONS
  virtual bool add_remote_tracker(const string &tracker, int sensor) = 0;
  virtual bool add_remote_analog(const string &analog) = 0;
  virtual bool add_remote_button(const string &button) = 0;
  virtual bool add_remote_dial(const string &dial) = 0;

  //GET FUNCTIONS
  virtual const TrackerData &get_tracker_data(const string &tracker, int sensor);
  virtual const AnalogData &get_analog_data(const string &analog);
  virtual const ButtonData &get_button_data(const string &button);
  virtual const DialData &get_dial_data(const string &dial);

protected:
  void push_tracker_position(const string &tracker, const int &sensor, const double &ptime, 
			     const LPoint3f &pos, const LVector4f &pquat);
  void push_tracker_velocity(const string &tracker, int sensor, const double &vtime, 
			     const LPoint3f &vel, const LVector4f &vquat, const float &dt);
  void push_tracker_acceleration(const string &tracker, const int &sensor, const double &atime, 
			     const LPoint3f &acc, const LVector4f &aquat, const float &dt);

  void push_analog(const string &analog, const float &atime, 
		   const double *channels, int num_channels);
  void push_button(const string &button, const float &btime, const int &button_id, 
		   const int &state);
  void push_dial(const string &dial, const float &dtime, const int &dial_id, 
		   const float &change);

private:
  int _sleep_time;
  const string _server;
  bool _forked;

#ifdef HAVE_IPC
  //Device locks and conditionals
  mutex _tracker_lock; 
  mutex _analog_lock; 
  mutex _button_lock; 
  mutex _dial_lock; 

  //Thread variables and functions
  thread *_client_thread;
  bool _shutdown;

  static void* st_callback(void *arg);
  void callback(void);
#endif

protected:
 //Device polling functions
  void poll_trackers();
  virtual void poll_tracker(const string &tracker) = 0;
  
  void poll_analogs();
  virtual void poll_analog(const string &analog) = 0;
  virtual int max_analog_channels(void) = 0;
  
  void poll_buttons();
  virtual void poll_button(const string &button) = 0;
  
  void poll_dials();
  virtual void poll_dial(const string &dial) = 0;


protected:
  typedef map< int, TrackerData > SensorDatas;
  typedef map< const string, SensorDatas > TrackerDatas;
  typedef map< const string, AnalogData > AnalogDatas;
  typedef map< const string, ButtonData > ButtonDatas;
  typedef map< const string, DialData > DialDatas;

  typedef vector_string Trackers;
  typedef vector_int Sensors;
  typedef map< string, Sensors > TrackerSensors;
  typedef vector_string Analogs;
  typedef vector_string Buttons;
  typedef vector_string Dials;

  TrackerDatas _tracker_datas;
  Trackers _trackers;
  TrackerSensors _sensors;

  AnalogDatas _analog_datas;
  Analogs _analogs;

  ButtonDatas _button_datas;
  Buttons _buttons;

  DialDatas _dial_datas;
  Dials _dials;

public:
  static TypeHandle get_class_type( void ) {
      return _type_handle;
  }
  static void init_type( void ) {
    TypedReferenceCount::init_type();
    register_type( _type_handle, "ClientBase",
		   TypedReferenceCount::get_class_type() );
  }
  virtual TypeHandle get_type( void ) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type(); 
    return get_class_type();
  }

private:
  static TypeHandle		_type_handle;
};
#endif
