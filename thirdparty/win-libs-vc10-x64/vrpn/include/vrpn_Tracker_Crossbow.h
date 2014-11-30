// vrpn_Tracker_Crossbow.h
//	This file contains the class header for a Crossbow RGA300CA Tracker.
//  This version was written in the summer of 2005 by Chris VanderKnyff.

#ifndef VRPN_TRACKER_CROSSBOW_H
#define VRPN_TRACKER_CROSSBOW_H

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "quat.h"
#include "vrpn_Tracker.h"

class VRPN_API vrpn_Tracker_Crossbow: public vrpn_Tracker_Serial {
public:
	/// The constructor is given the name of the tracker (the name of
	/// the sender it should use), the connection on which it is to
	/// send its messages, the name of the serial port it is to open
	/// (default is /dev/ttyS0 (first serial port in Linux), the baud
	/// rate at which it is to communicate (default 38400), the linear
	/// acceleration range of the sensor in Gs (default 2), and the 
	/// angular acceleration range of the sensor in degrees per second
	/// (default 100).

	vrpn_Tracker_Crossbow(const char *name, vrpn_Connection *c, 
				const char *port = "/dev/ttyS0", long baud = 38400,
				float g_range = 2.0f, float ar_range = 100.0f);

	~vrpn_Tracker_Crossbow();

	// Run through the main loop once, sending notification messages as necessary
	virtual void mainloop();

	// Reset the tracker. 
	void reset();

	// Get reports from the serial port
	int get_report();

	// Retrieve the device's serial number. This is cached between resets.
	vrpn_uint32 get_serial_number();

	// Retrieve the device's version string. This is cached between resets.
	const char *get_version_string();

	// Recalibrates and zeros the device's rate sensors over a specified # of samples.
	// This takes about 3-4ms per sample, during which time the process blocks.
	// The device does not have to be level when zeroing, but must be still.
	void recalibrate(vrpn_uint16 num_samples = 20000);

protected:
	struct raw_packet {
		vrpn_uint16 header;
		vrpn_int16 roll_angle;
		vrpn_int16 pitch_angle;
		vrpn_int16 yaw_rate;
		vrpn_int16 accel_x;
		vrpn_int16 accel_y;
		vrpn_int16 accel_z;
		vrpn_uint16 timer;
		vrpn_int16 temp_voltage;
		vrpn_int16 part_number;
		vrpn_int16 status;
		vrpn_uint16 checksum;
	};

	struct timeval init_time;
	float lin_accel_range;
	float ang_accel_range;

	vrpn_uint32 device_serial; // Device serial number
	char *device_version; // Device version string

	int just_read_something;

	void unbuffer_packet(raw_packet &dest, unsigned char *buffer);
	int validate_packet(const raw_packet &packet);
	void process_packet(const raw_packet &packet);

	float convert_scalar(vrpn_int16 data, float scale) const;
	void xb_quat_from_euler(q_type destQuat, double pitch, double roll) const;

	void send_report();

	void ping();

};

#endif

