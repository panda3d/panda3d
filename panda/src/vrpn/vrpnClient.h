// Filename: vrpnClient.h
// Created by:  jason (04Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VRPN_CLIENT
#define VRPN_CLIENT

#include <pandabase.h>
#include <clientBase.h>

#ifdef CPPPARSER
  // For correct interrogate parsing of UNC's vrpn library.
  #ifdef WIN32_VC
    #define _WIN32
    #define SOCKET int
  #else
    #define linux
    typedef struct timeval timeval;
  #endif
#endif

#include <vrpn_Connection.h>
#include <vrpn_Tracker.h>
#include <vrpn_Analog.h>
#include <vrpn_Button.h>
#include <vrpn_Dial.h>

class EXPCL_PANDA VrpnClient : public ClientBase {
PUBLISHED:
  INLINE VrpnClient(const string &server);

public:
  //ADD FUNCTIONS
  virtual bool add_remote_tracker(const string &tracker, int sensor);
  virtual bool add_remote_analog(const string &analog);
  virtual bool add_remote_button(const string &button);
  virtual bool add_remote_dial(const string &dial);

protected:
  virtual int max_analog_channels();

  //Device polling functions
  virtual void poll_tracker(const string &tracker);
  virtual void poll_analog(const string &analog);
  virtual void poll_button(const string &button);
  virtual void poll_dial(const string &dial);

private:
  //Private VRPN objects
  typedef map <string, vrpn_Tracker*> VrpnTrackers;
  typedef map <string, vrpn_Analog*> VrpnAnalogs;
  typedef map <string, vrpn_Button*> VrpnButtons;
  typedef map <string, vrpn_Dial*> VrpnDials;

  vrpn_Connection *_connection;
  VrpnTrackers _vrpn_trackers;
  VrpnAnalogs _vrpn_analogs;
  VrpnButtons _vrpn_buttons;
  VrpnDials _vrpn_dials;

  //VRPN Callback functions.  Each callback actually needs to be a
  //pair of two functions, one a static and one not.  The static is
  //needed because we can't set member functions as callbacks (due to
  //the implicity this pointer) and the non-static function is needed
  //so that we can set the non-static data storage variables
  //appropriately
private:
  static void st_tracker_position(void *userdata, const vrpn_TRACKERCB info);
  INLINE void tracker_position(const string &tracker, const vrpn_TRACKERCB info);

  static void st_tracker_velocity(void *userdata, const vrpn_TRACKERVELCB info);
  INLINE void tracker_velocity(const string &tracker, const vrpn_TRACKERVELCB info);

  static void st_tracker_acceleration(void *userdata, const vrpn_TRACKERACCCB info);
  INLINE void tracker_acceleration(const string &tracker, const vrpn_TRACKERACCCB info);

  static void st_analog(void *userdata, const vrpn_ANALOGCB info);
  INLINE void analog(const string &analog, const vrpn_ANALOGCB info);

  static void st_button(void *userdata, const vrpn_BUTTONCB info);
  INLINE void button(const string &button, const vrpn_BUTTONCB info);

  static void st_dial(void *userdata, const vrpn_DIALCB info);
  INLINE void dial(const string &dial, const vrpn_DIALCB info);

public:
  static TypeHandle get_class_type( void ) {
      return _type_handle;
  }
  static void init_type( void ) {
    ClientBase::init_type();
    register_type( _type_handle, "VrpnClient",
		   ClientBase::get_class_type() );
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

INLINE double convert_to_secs(struct timeval msg_time);

#include "vrpnClient.I"

#endif


