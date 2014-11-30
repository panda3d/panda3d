#ifndef	vrpn_POSER_H
#define vrpn_POSER_H

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

// NOTE: the poser class borrows heavily from the vrpn_Tracker code.
//       The poser is basically the inverse of a tracker.  
//       We are only handling pose and velocity updates for now...acceleration
//       will come later, as needed.

#include "vrpn_Connection.h"
#include "vrpn_BaseClass.h"

class VRPN_API vrpn_Poser : public vrpn_BaseClass {
    public:
        vrpn_Poser (const char * name, vrpn_Connection * c = NULL );

        virtual ~vrpn_Poser (void);

        void p_print();          // print the current pose
        void p_print_vel();      // print the current velocity

        // a poser server should call the following to register the
        // default xform and workspace request handlers
//        int register_server_handlers(void);

    protected:
        // client-->server
        vrpn_int32 req_position_m_id;				// ID of poser position message
        vrpn_int32 req_position_relative_m_id;	    // ID of poser position delta message
        vrpn_int32 req_velocity_m_id;				// ID of poser velocity message
        vrpn_int32 req_velocity_relative_m_id;		// ID of poser velocity delta message

        // Description of current state
        vrpn_float64 p_pos[3], p_quat[4];	// Current pose, (x,y,z), (qx,qy,qz,qw)
        vrpn_float64 p_vel[3], p_vel_quat[4];   // Current velocity and dQuat/vel_quat_dt
        vrpn_float64 p_vel_quat_dt;             // delta time (in secs) for vel_quat
        struct timeval p_timestamp;		// Current timestamp

        // Minimum and maximum values available for the position and velocity values
        // of the poser.
        vrpn_float64    p_pos_min[3], p_pos_max[3], p_pos_rot_min[3], p_pos_rot_max[3],
                        p_vel_min[3], p_vel_max[3], p_vel_rot_min[3], p_vel_rot_max[3];

        virtual int register_types(void);	    // Called by BaseClass init()

        virtual int encode_to(char* buf);       // Encodes the position
        virtual int encode_vel_to(char* buf);   // Encodes the velocity

        virtual void set_pose( const struct timeval t,                 // Sets the pose internally
							   const vrpn_float64 position[3], 
                               const vrpn_float64 quaternion[4] );
        virtual void set_pose_relative( const struct timeval t,                 // Increments the pose internally
										const vrpn_float64 position_delta[3],	// pos_new = position_delta + pos_old
										const vrpn_float64 quaternion[4] );		// q_new = quaternion * q_old
        virtual void set_pose_velocity( const struct timeval t,        // Sets the velocity internally
										const vrpn_float64 position[3], 
										const vrpn_float64 quaternion[4], 
										const vrpn_float64 interval );
        virtual void set_pose_velocity_relative( const struct timeval t,        // Increments the velocity internally
												 const vrpn_float64 velocity_delta[3],	// vel_new = velocity_delta + vel_old
												 const vrpn_float64 quaternion[4],		// q_new = quaternion * q_old
												 const vrpn_float64 interval_delta );	// interval_new = interval_delta + interval_old
};

//------------------------------------------------------------------------------------
// Server Code

///A structure for Call-Backs related to Vrpn Poser Server
typedef	struct _vrpn_POSERCB {
	struct timeval	msg_time;	// Timestamp
	///NOTE: I think since we have different routines for handling velocity and position poser requests, 
	/// putting poser and quaternions for both doesn't make sense. Instead, the change handler should
	/// take care of packing correct poser and quaternion.
	vrpn_float64	pos[3];
	vrpn_float64	quat[4];	
} vrpn_POSERCB;

typedef void (VRPN_CALLBACK *vrpn_POSERHANDLER) (void * userdata,
					  const vrpn_POSERCB info);


//------------------------------------------------------------------------------------
// Server Code
// Users supply the routines to handle requests from the client

// This is a sample basic poser server
// 

class VRPN_API vrpn_Poser_Server: public vrpn_Poser {
public:
	vrpn_Poser_Server (const char* name, vrpn_Connection* c);
	
	/// This function should be called each time through app mainloop.
	virtual void mainloop();
	
	int register_change_handler(void *userdata, vrpn_POSERHANDLER handler) {
		return d_callback_list.register_handler(userdata, handler);
	};
	int unregister_change_handler(void *userdata, vrpn_POSERHANDLER handler) {
		return d_callback_list.unregister_handler(userdata, handler);
	}

	int register_relative_change_handler( void* userdata, vrpn_POSERHANDLER handler )
	{  return d_relative_callback_list.register_handler( userdata, handler );  }
	int unregister_relative_change_handler( void* userdata, vrpn_POSERHANDLER handler )
	{  return d_relative_callback_list.unregister_handler( userdata, handler );  }

protected:
	static int VRPN_CALLBACK handle_change_message(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_relative_change_message(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_vel_change_message(void *userdata, vrpn_HANDLERPARAM p);
	static int VRPN_CALLBACK handle_relative_vel_change_message(void *userdata, vrpn_HANDLERPARAM p);
	vrpn_Callback_List<vrpn_POSERCB> d_callback_list;
	vrpn_Callback_List<vrpn_POSERCB> d_relative_callback_list;
};	

//------------------------------------------------------------------------------------
// Client Code

// Open a poser that is on the other end of a connection for sending updates to it.  
class VRPN_API vrpn_Poser_Remote: public vrpn_Poser {
public:
	// The name of the poser to connect to, including connection name,
	// for example "poser@magnesium.cs.unc.edu". If you already
	// have the connection open, you can specify it as the second parameter.
	// This allows both servers and clients in the same thread, for example.
	// If it is not specified, then the connection will be looked up based
	// on the name passed in.
	vrpn_Poser_Remote (const char* name, vrpn_Connection* c = NULL);

	// unregister all of the handlers registered with the connection
	virtual ~vrpn_Poser_Remote (void);

	// This routine calls the mainloop of the connection it's on
	virtual void mainloop();
   
	// Routines to set the state of the poser
	int request_pose(const struct timeval t, const vrpn_float64 position[3], const vrpn_float64 quaternion[4]);
	int request_pose_relative(const struct timeval t, const vrpn_float64 position_delta[3], const vrpn_float64 quaternion[4]);
	int request_pose_velocity( const struct timeval t, const vrpn_float64 velocity[3], 
							   const vrpn_float64 quaternion[4], const vrpn_float64 interval );
	int request_pose_velocity_relative( const struct timeval t, const vrpn_float64 velocity_delta[3], 
										const vrpn_float64 quaternion[4], const vrpn_float64 interval_delta );
	
protected:
	virtual int client_send_pose();				// Sends the current pose.  Called by request_pose
	virtual int client_send_pose_relative();	// Sends the current pose delta.  Called by request_pose_relative
	virtual int client_send_pose_velocity();	// Sends the current velocity.  Called by request_pose_velocity
	virtual int client_send_pose_velocity_relative();	// Sends the current velocity delta.  Called by request_pose_velocity_relative
};

#endif
