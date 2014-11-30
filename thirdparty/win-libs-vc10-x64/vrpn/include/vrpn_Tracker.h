#ifndef	vrpn_TRACKER_H
#define vrpn_TRACKER_H

#ifndef _WIN32_WCE
#include <time.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#ifndef _WIN32_WCE
#include <sys/time.h>
#endif
#endif

// NOTE: a vrpn tracker must call user callbacks with tracker data (pos and
//       ori info) which represent the transformation xfSourceFromSensor.
//       This means that the pos info is the position of the origin of
//       the sensor coord sys in the source coord sys space, and the
//       quat represents the orientation of the sensor relative to the
//       source space (ie, its value rotates the source's axes so that
//       they coincide with the sensor's)
// Positions from all trackers in VRPN are reported in meters.
// Velocities are reported in meters/second.
// Accelerations are reported in meters/second/second.
// These are all reported in three-element double arrays
// in the order (X=0, Y=1, Z=2).
// They are translated into this format from the native format for each device.
// Orientations from all trackers in VRPN are reported in quaternions
// (see Quatlib for more info) in four-element double arrays
// in the order (X=0, Y=1, Z=2, W=3).
// They are translated into this format from the native format for each device.

// to use time synched tracking, just pass in a sync connection to the 
// client and the server

#include "vrpn_BaseClass.h"
#include "vrpn_Connection.h"

class VRPN_API vrpn_RedundantTransmission;

// tracker status flags
const	int vrpn_TRACKER_SYNCING	   = (3);
const	int vrpn_TRACKER_AWAITING_STATION  = (2);
const	int vrpn_TRACKER_REPORT_READY 	   = (1);
const	int vrpn_TRACKER_PARTIAL 	   = (0);
const	int vrpn_TRACKER_RESETTING	   = (-1);
const	int vrpn_TRACKER_FAIL 	 	   = (-2);

// index for the change_list that should be called for all sensors.
// Not an in-range index.
const	int vrpn_ALL_SENSORS = -1;

typedef vrpn_float64  vrpn_Tracker_Pos[3];
typedef vrpn_float64  vrpn_Tracker_Quat[4];

class VRPN_API vrpn_Tracker : public vrpn_BaseClass {
  public:
  // vrpn_Tracker.cfg, in the "local" directory, is the default config file
  // . You can specify a different config file in the constructor. When
  // you do this, you must also specify a vrpn_Connection. Pass in NULL
  // if you don't have one. This awkwardness is because C++ requires that
  //only the rightmost arguements can use the default values, and that the
  //order of arguements must match the base class :(
   vrpn_Tracker (const char * name, vrpn_Connection * c = NULL,
		 const char * tracker_cfg_file_name = NULL);

   virtual ~vrpn_Tracker (void);

   int read_config_file (FILE * config_file, const char * tracker_name);
   void print_latest_report(void);
   // a tracker server should call the following to register the
   // default xform and workspace request handlers
   int register_server_handlers(void);
   void get_local_t2r(vrpn_float64 *vec, vrpn_float64 *quat);
   void get_local_u2s(vrpn_int32 sensor, vrpn_float64 *vec, vrpn_float64 *quat);
   static int VRPN_CALLBACK handle_t2r_request(void *userdata, vrpn_HANDLERPARAM p);
   static int VRPN_CALLBACK handle_u2s_request(void *userdata, vrpn_HANDLERPARAM p);
   static int VRPN_CALLBACK handle_workspace_request(void *userdata, vrpn_HANDLERPARAM p);
   //static int VRPN_CALLBACK handle_update_rate_request (void *, vrpn_HANDLERPARAM);

  protected:
   vrpn_int32 position_m_id;		// ID of tracker position message
   vrpn_int32 velocity_m_id;		// ID of tracker velocity message
   vrpn_int32 accel_m_id;		// ID of tracker acceleration message
   vrpn_int32 tracker2room_m_id;	// ID of tracker tracker2room message
   vrpn_int32 unit2sensor_m_id;		// ID of tracker unit2sensor message
   vrpn_int32 request_t2r_m_id;		// ID of tracker2room request message
   vrpn_int32 request_u2s_m_id;		// ID of unit2sensor request message
   vrpn_int32 request_workspace_m_id;	// ID of workspace request message
   vrpn_int32 workspace_m_id;		// ID of workspace message
   vrpn_int32 update_rate_id;		// ID of update rate message
   vrpn_int32 connection_dropped_m_id;	// ID of connection dropped message
   vrpn_int32 reset_origin_m_id;	// ID of reset origin message					

   // Description of the next report to go out
   vrpn_int32 d_sensor;			// Current sensor
   vrpn_float64 pos[3], d_quat[4];	// Current pose, (x,y,z), (qx,qy,qz,qw)
   vrpn_float64 vel[3], vel_quat[4];	// Cur velocity and dQuat/vel_quat_dt
   vrpn_float64 vel_quat_dt;		// delta time (in secs) for vel_quat
   vrpn_float64 acc[3], acc_quat[4];	// Cur accel and d2Quat/acc_quat_dt2
   vrpn_float64 acc_quat_dt;		// delta time (in secs) for acc_quat
   struct timeval timestamp;		// Current timestamp

   // The timestamp that the last report was received (Used by the Liberty Driver)
   // Other trackers use timestamp as the watchdog, however due to variable USB
   // latency the Liberty driver uses the device timestamp and not the computer clock 
   // at the time the report was received. This however can drift
   // from the computer time, and hence it can cause a reset when things are
   // working fine
   struct timeval watchdog_timestamp;

   vrpn_float64 tracker2room[3], tracker2room_quat[4]; // Current t2r xform
   vrpn_int32 num_sensors;

   // Arrays of values, one per sensor.  Includes function to ensure there are
   // enough there for a specified number of sensors.
   vrpn_Tracker_Pos   *unit2sensor;
   vrpn_Tracker_Quat  *unit2sensor_quat; // Current u2s xforms
   unsigned num_unit2sensors;
   bool ensure_enough_unit2sensors(unsigned num);

   // bounding box for the tracker workspace (in tracker space)
   // these are the points with (x,y,z) minimum and maximum
   // note: we assume the bounding box edges are aligned with the tracker
   // coordinate system
   vrpn_float64 workspace_min[3], workspace_max[3];

   int status;		// What are we doing?

   virtual int register_types(void);	//< Called by BaseClass init()
   virtual int encode_to(char *buf);	 // Encodes the position report
   // Not all trackers will call the velocity and acceleration packers
   virtual int encode_vel_to(char *buf); // Encodes the velocity report
   virtual int encode_acc_to(char *buf); // Encodes the acceleration report
   virtual int encode_tracker2room_to(char *buf); // Encodes the tracker2room
   virtual int encode_unit2sensor_to(char *buf); // and unit2sensor xforms
   virtual int encode_workspace_to(char *buf); // Encodes workspace info
};

#ifndef VRPN_CLIENT_ONLY
#define VRPN_TRACKER_BUF_SIZE 100

class VRPN_API vrpn_Tracker_Serial : public vrpn_Tracker {
  public:
   vrpn_Tracker_Serial
               (const char * name, vrpn_Connection * c,
		const char * port = "/dev/ttyS1", long baud = 38400);
   virtual ~vrpn_Tracker_Serial();

  protected:
   char portname[VRPN_TRACKER_BUF_SIZE];
   long baudrate;
   int serial_fd;

   unsigned char buffer[VRPN_TRACKER_BUF_SIZE];// Characters read in from the tracker so far
   vrpn_uint32 bufcount;		// How many characters in the buffer?

   /// Gets a report if one is available, returns 0 if not, 1 if complete report.
   virtual int get_report(void) = 0;

   // Sends the report that was just read.
   virtual void send_report(void);

   /// Reset the tracker.
   virtual void reset(void) = 0;

   /// Uses the get_report, send_report, and reset routines to implement a server
   virtual void mainloop();
};
#endif  // VRPN_CLIENT_ONLY


// This is an example of a tracker server.  It basically reports the
// position at the origin with zero velocity and acceleration over and
// over again at the rate requested.  It is here mostly as an example of
// how to build a tracker server, and also serves as a test object for
// client codes and VRPN builds.

class VRPN_API vrpn_Tracker_NULL: public vrpn_Tracker {
  public:
   vrpn_Tracker_NULL (const char * name, vrpn_Connection * c,
	vrpn_int32 sensors = 1, vrpn_float64 Hz = 1.0);
   virtual void mainloop();

   void setRedundantTransmission (vrpn_RedundantTransmission *);

  protected:
   vrpn_float64	update_rate;

   vrpn_RedundantTransmission * d_redundancy;
};


// This is a tracker server that can be used by an application that
// just wants to generate tracker reports but does not really have
// a tracker device to drive.  Similar to the vrpn_Analog_Server, it
// provides a quick and easy way for an application to report things.
//
// The application creates an object of this class, specifying the
// number of sensors and the connection that is to be used.  It then
// reports poses (position + quat), pose velocities, and pose
// accelerations as desired using the provided functions.  The
// mainloop() function needs to be called periodically even when
// there is nothing to report.

class VRPN_API vrpn_Tracker_Server: public vrpn_Tracker {
  public:
   vrpn_Tracker_Server (const char * name, vrpn_Connection * c,
	vrpn_int32 sensors = 1);

   /// This function should be called each time through app mainloop.
   virtual void mainloop();

   /// These functions should be called to report changes in state, once per sensor.
   virtual int report_pose(const int sensor, const struct timeval t,
			   const vrpn_float64 position[3],
			   const vrpn_float64 quaternion[4],
			   const vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
   virtual int report_pose_velocity(const int sensor, const struct timeval t,
			   const vrpn_float64 position[3],
			   const vrpn_float64 quaternion[4],
			   const vrpn_float64 interval,
			   const vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
   virtual int report_pose_acceleration(const int sensor, const struct timeval t,
			   const vrpn_float64 position[3],
			   const vrpn_float64 quaternion[4],
			   const vrpn_float64 interval,
			   const vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

};



//----------------------------------------------------------
// ************** Users deal with the following *************

// User routine to handle a tracker position update.  This is called when
// the tracker callback is called (when a message from its counterpart
// across the connection arrives).

typedef	struct _vrpn_TRACKERCB {
	struct timeval	msg_time;	// Time of the report
	vrpn_int32	sensor;		// Which sensor is reporting
	vrpn_float64	pos[3];		// Position of the sensor
	vrpn_float64	quat[4];	// Orientation of the sensor
} vrpn_TRACKERCB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERCHANGEHANDLER)(void *userdata,
					  const vrpn_TRACKERCB info);

// User routine to handle a tracker velocity update.  This is called when
// the tracker callback is called (when a message from its counterpart
// across the connetion arrives).

typedef	struct _vrpn_TRACKERVELCB {
	struct timeval	msg_time;	// Time of the report
	vrpn_int32	sensor;		// Which sensor is reporting
	vrpn_float64	vel[3];		// Velocity of the sensor
	vrpn_float64	vel_quat[4];	// Future Orientation of the sensor
        vrpn_float64	vel_quat_dt;    // delta time (in secs) for vel_quat
} vrpn_TRACKERVELCB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERVELCHANGEHANDLER)(void *userdata,
					     const vrpn_TRACKERVELCB info);

// User routine to handle a tracker acceleration update.  This is called when
// the tracker callback is called (when a message from its counterpart
// across the connetion arrives).

typedef	struct _vrpn_TRACKERACCCB {
	struct timeval	msg_time;	// Time of the report
	vrpn_int32	sensor;		// Which sensor is reporting
	vrpn_float64	acc[3];		// Acceleration of the sensor
	vrpn_float64	acc_quat[4];	// ?????
        vrpn_float64	acc_quat_dt;    // delta time (in secs) for acc_quat
        
} vrpn_TRACKERACCCB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERACCCHANGEHANDLER)(void *userdata,
					     const vrpn_TRACKERACCCB info);

// User routine to handle a tracker room2tracker xform update. This is called
// when the tracker callback is called (when a message from its counterpart
// across the connection arrives).

typedef struct _vrpn_TRACKERTRACKER2ROOMCB {
	struct timeval	msg_time;		// Time of the report
	vrpn_float64	tracker2room[3];	// position offset
	vrpn_float64	tracker2room_quat[4];	// orientation offset
} vrpn_TRACKERTRACKER2ROOMCB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERTRACKER2ROOMCHANGEHANDLER)(void *userdata,
					const vrpn_TRACKERTRACKER2ROOMCB info);

typedef struct _vrpn_TRACKERUNIT2SENSORCB {
        struct timeval  msg_time;       	// Time of the report
	vrpn_int32	sensor;			// Which sensor this is for
        vrpn_float64  unit2sensor[3];		// position offset
        vrpn_float64  unit2sensor_quat[4];	// orientation offset
} vrpn_TRACKERUNIT2SENSORCB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERUNIT2SENSORCHANGEHANDLER)(void *userdata,
                                        const vrpn_TRACKERUNIT2SENSORCB info);

typedef struct _vrpn_TRACKERWORKSPACECB {
	struct timeval  msg_time;       // Time of the report
	vrpn_float64 workspace_min[3];	// minimum corner of box (tracker CS)
	vrpn_float64 workspace_max[3];	// maximum corner of box (tracker CS)
} vrpn_TRACKERWORKSPACECB;
typedef void (VRPN_CALLBACK *vrpn_TRACKERWORKSPACECHANGEHANDLER)(void *userdata,
					const vrpn_TRACKERWORKSPACECB info);

// Structure to hold all of the callback lists for one sensor
// (also used for the "all sensors" sensor).
class vrpn_Tracker_Sensor_Callbacks {
public:
    vrpn_Callback_List<vrpn_TRACKERCB>	  d_change;
    vrpn_Callback_List<vrpn_TRACKERVELCB> d_velchange;
    vrpn_Callback_List<vrpn_TRACKERACCCB> d_accchange;
    vrpn_Callback_List<vrpn_TRACKERUNIT2SENSORCB>   d_unit2sensorchange;

    // This class requires deep copies.
    void operator =(const vrpn_Tracker_Sensor_Callbacks &from) {
      d_change = from.d_change;
      d_velchange = from.d_velchange;
      d_accchange = from.d_accchange;
      d_unit2sensorchange = from.d_unit2sensorchange;
    };
};

// Open a tracker that is on the other end of a connection
// and handle updates from it.  This is the type of tracker that user code will
// deal with.

class VRPN_API vrpn_Tracker_Remote: public vrpn_Tracker {
  public:
	// The name of the tracker to connect to, including connection name,
	// for example "Ceiling_tracker@ceiling.cs.unc.edu". If you already
	// have the connection open, you can specify it as the second parameter.
	// This allows both servers and clients in the same thread, for example.
	// If it is not specified, then the connection will be looked up based
	// on the name passed in.
	vrpn_Tracker_Remote (const char * name, vrpn_Connection *c = NULL);

        // unregister all of the handlers registered with the connection
        virtual ~vrpn_Tracker_Remote (void);

	// request room from tracker xforms
	int request_t2r_xform(void);
	// request all available sensor from unit xforms
	int request_u2s_xform(void);
	// request workspace bounding box
	int request_workspace(void);

	// set rate of p/v/a updates from the tracker
	int set_update_rate (vrpn_float64 samplesPerSecond);

	// reset origin to current tracker location (e.g. - to reinitialize
	// a PHANToM in its reset position)
	int reset_origin(void);

	// This routine calls the mainloop of the connection it's on
	virtual void mainloop();

	// **** to register handlers for sensor-specific messages: ****
	// Default is to register them for all sensors.

        // (un)Register a callback handler to handle a position change
        virtual int register_change_handler(void *userdata,
                vrpn_TRACKERCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);
        virtual int unregister_change_handler(void *userdata,
                vrpn_TRACKERCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);

        // (un)Register a callback handler to handle a velocity change
        virtual int register_change_handler(void *userdata,
                vrpn_TRACKERVELCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);
        virtual int unregister_change_handler(void *userdata,
                vrpn_TRACKERVELCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);

        // (un)Register a callback handler to handle an acceleration change
        virtual int register_change_handler(void *userdata,
                vrpn_TRACKERACCCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);
        virtual int unregister_change_handler(void *userdata,
                vrpn_TRACKERACCCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);

        // (un)Register a callback handler to handle a unit2sensor change
        virtual int register_change_handler(void *userdata,
                vrpn_TRACKERUNIT2SENSORCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);
        virtual int unregister_change_handler(void *userdata,
                vrpn_TRACKERUNIT2SENSORCHANGEHANDLER handler, vrpn_int32 sensor = vrpn_ALL_SENSORS);

	// **** to get workspace information ****
	// (un)Register a callback handler to handle a workspace change
	virtual int register_change_handler(void *userdata,
		vrpn_TRACKERWORKSPACECHANGEHANDLER handler) {
	  return d_workspacechange_list.register_handler(userdata, handler);
	};
	virtual int unregister_change_handler(void *userdata,
		vrpn_TRACKERWORKSPACECHANGEHANDLER handler) {
	  return d_workspacechange_list.unregister_handler(userdata, handler);
	}

	// (un)Register a callback handler to handle a tracker2room change
	virtual int register_change_handler(void *userdata,
		vrpn_TRACKERTRACKER2ROOMCHANGEHANDLER handler) {
	  return d_tracker2roomchange_list.register_handler(userdata, handler);
	};
	virtual int unregister_change_handler(void *userdata,
		vrpn_TRACKERTRACKER2ROOMCHANGEHANDLER handler) {
	  return d_tracker2roomchange_list.unregister_handler(userdata, handler);
	};

  protected:
    // Callbacks with one per sensor (plus one for "all")
    vrpn_Tracker_Sensor_Callbacks   all_sensor_callbacks;
    vrpn_Tracker_Sensor_Callbacks   *sensor_callbacks;
    unsigned num_sensor_callbacks;
    bool  ensure_enough_sensor_callbacks(unsigned num);

    // Callbacks that are one per tracker
    vrpn_Callback_List<vrpn_TRACKERTRACKER2ROOMCB>  d_tracker2roomchange_list;
    vrpn_Callback_List<vrpn_TRACKERWORKSPACECB>	    d_workspacechange_list;

    static int VRPN_CALLBACK handle_change_message(void *userdata,
		    vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_vel_change_message(void *userdata,
		    vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_acc_change_message(void *userdata,
		    vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_tracker2room_change_message(void *userdata,
		    vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_unit2sensor_change_message(void *userdata,
                    vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_workspace_change_message(void *userdata,
		    vrpn_HANDLERPARAM p);
};

#endif
