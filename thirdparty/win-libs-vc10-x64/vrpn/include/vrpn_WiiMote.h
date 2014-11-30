// vrpn_WiiMote.h:  Drive for the WiiMote, based on the WiiUse library.
//  Tested on Windows.  Is supposed to also run on Linux.
//
// Russ Taylor, revised December 2008

#ifndef VRPN_WIIMOTE_H
#define VRPN_WIIMOTE_H

#include "vrpn_Configure.h"
#if defined(VRPN_USE_WIIUSE)

#include "vrpn_Shared.h"
#include "vrpn_Connection.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"
#include "vrpn_Analog_Output.h"

// maximum number of wiimotes connected to the system
#define VRPN_WIIUSE_MAX_WIIMOTES 4

// Opaque class to keep us from having to include wiiuse.h in user files.
// This is defined in vrpn_WiiMote.C.
class vrpn_Wiimote_Device;
struct wiimote_t;
#ifdef vrpn_THREADS_AVAILABLE
struct vrpn_WiiMote_SharedData;
#endif

// The buttons are as read from the bit-fields of the primary controller (bits 0-15)
//  and then a second set for any extended controller (nunchuck bits 16-31),
//  (classic controller bits 32-47), (guitar hero 3 bits 48-63).
//
// If you enable "reorderButtons" by setting it to 1, the buttons on the Wiimote
// itself are re-ordered to be reported as follows:
//    button[0] = Home
//    button[1] = "1"
//    button[2] = "2"
//    button[3] = "A"
//    button[4] = "B"
//    button[5] = "-"
//    button[6] = "+"
//    button[7] = direction pad: left
//    button[8] = direction pad: right
//    button[9] = direction pad: down
//    button[10] = direction pad: up
//    button[11] = WIIMOTE_BUTTON_ZACCEL_BIT4
//    button[12] = WIIMOTE_BUTTON_ZACCEL_BIT5
//    button[13] = WIIMOTE_BUTTON_ZACCEL_BIT6
//    button[14] = WIIMOTE_BUTTON_ZACCEL_BIT7
//    button[15] = WIIMOTE_BUTTON_UNKNOWN
//
// The Analogs are in an even more random order, both from the primary controller:
//    channel[0] = battery level (0-1)
//    channel[1] = gravity X vector calculation (1 = Earth gravity)
//    channel[2] = gravity Y vector calculation (1 = Earth gravity)
//    channel[3] = gravity Z vector calculation (1 = Earth gravity)
//    channel[4] = X of first sensor spot (0-1023, -1 if not seen)
//    channel[5] = Y of first sensor spot (0-767, -1 if not seen)
//    channel[6] = size of first sensor spot (0-15, -1 if not seen)
//    channel[7] = X of second sensor spot (0-1023, -1 if not seen)
//    channel[9] = Y of second sensor spot (0-767, -1 if not seen)
//    channel[9] = size of second sensor spot (0-15, -1 if not seen)
//    channel[10] = X of third sensor spot (0-1023, -1 if not seen)
//    channel[11] = Y of third sensor spot (0-767, -1 if not seen)
//    channel[12] = size of third sensor spot (0-15, -1 if not seen)
//    channel[13] = X of fourth sensor spot (0-1023, -1 if not seen)
//    channel[14] = Y of fourth sensor spot (0-767, -1 if not seen)
//    channel[15] = size of fourth sensor spot (0-15, -1 if not seen)
// and on the secondary controllers (skipping values to leave room for expansion)
//    channel[16] = nunchuck gravity X vector
//    channel[17] = nunchuck gravity Y vector
//    channel[18] = nunchuck gravity Z vector
//    channel[19] = nunchuck joystick angle
//    channel[20] = nunchuck joystick magnitude
//
//    channel[32] = classic L button
//    channel[33] = classic R button
//    channel[34] = classic L joystick angle
//    channel[35] = classic L joystick magnitude
//    channel[36] = classic R joystick angle
//    channel[37] = classic R joystick magnitude
//
//    channel[48] = guitar hero whammy bar
//    channel[49] = guitar hero joystick angle
//    channel[50] = guitar hero joystick magnitude
//
// Balance board data: (requires WiiUse 0.13 or newer, preferably 0.14 or newer)
//    channel[64] = Balance board: top-left sensor, kg
//    channel[65] = Balance board: top-right sensor, kg
//    channel[66] = Balance board: bottom-left sensor, kg
//    channel[67] = Balance board: bottom-right sensor, kg
//    channel[68] = Balance board: total mass, kg
//    channel[69] = Balance board: center of gravity x, in [-1, 1]
//    channel[70] = Balance board: center of gravity y, in [-1, 1]
//
// The Analog_Output 0 is a hack to enable control over the rumble, inherited from
//  the RumblePack driver.  This should eventually move to a binary output of
//  some kind (and so should the lights).  For now, if you set output 0 to a
//  value greater than or equal to 0.5, it will turn on the rumble; if less, then
//  it will disable it.
// Channel 1 sets the IR sensitivity: accepts 1, 2, 3, 4, 5 (clipping to this
// range if given other values), same as the Wii system's sensitivity settings.

// XXX It would be great to use the IR and accelerometer data along with
//      a description of the size of the official Wii sensor bar to compute
//      a pose and report this as a tracker report.  This could be done as
//      a second device that reads the values or (better) as a built-in that
//      exports a vrpn_Tracker report.  Best would be a Kalman filter that
//      takes all of these as inputs and produces an optimal estimate.  This
//      needs to report in meters and we'll have to define an origin (middle
//      of the bar?) and a coordinate system (X along bar's most-horizontal
//      axis, Y vertical to gravity, Z perpendicular to these?).

// XXX It would be great to have a vrpn_Sound device that could play through
//      the speaker on the WiiMote.

class VRPN_API vrpn_WiiMote: public vrpn_Analog, public vrpn_Button, public vrpn_Analog_Output {
	public:
		// If there is more than one WiiMote on the machine, the zero-indexed 'which'
		// parameter tells which one we want to open.
		vrpn_WiiMote(const char *name, vrpn_Connection *c = NULL, unsigned which = 0
		             , unsigned useMS = 1, unsigned useIR = 1, unsigned reorderButtons = 0,
		             const char *bdaddr = NULL);
		~vrpn_WiiMote();

		virtual void mainloop();

		bool isValid() const;

	protected:
		// Handle the rumble-magnitude setting (channel 0).
		static int VRPN_CALLBACK handle_request_message(void *userdata,
		        vrpn_HANDLERPARAM p);
		static int VRPN_CALLBACK handle_request_channels_message(void* userdata,
		        vrpn_HANDLERPARAM p);
		static int VRPN_CALLBACK handle_last_connection_dropped(void *selfPtr, vrpn_HANDLERPARAM data);

	private:
		/// @name Message lock functions
		/// Used to reduce the amount of in-line ifdefs: these are no-ops
		/// if threading support isn't available
		/// @{
		void acquireMessageLock();
		void releaseMessageLock();
		/// @}
#ifdef vrpn_THREADS_AVAILABLE
		/// function to (re)connect to wiimote in background:
		static void connectThreadFunc(vrpn_ThreadData &threadData);
		/// mainloop is waiting for the connectThread to reestablish the connection:
		bool waitingForConnection;
		/// the struct holding the shared data pointer and the mutex:
		vrpn_WiiMote_SharedData *sharedData;
		/// thread for asynchronous reconnection function:
		vrpn_Thread *connectThread;
#endif
		/// The WiiMote to use
		vrpn_Wiimote_Device  *wiimote;
		/// a list of available wiimotes
		wiimote_t **available_wiimotes;

		/// Error-handling procedure (spit out a message and die)
		inline void FAIL(const char *msg) {
			struct timeval now;
			vrpn_gettimeofday(&now, NULL);
			send_text_message(msg, now, vrpn_TEXT_ERROR);
			d_connection = NULL;
		}

		/** @brief send report iff changed

			@note class_of_service is only applied to vrpn_Analog values,
			not vrpn_Button
		*/
		void report_changes(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
		/** @brief send report whether or not changed

			@note class_of_service is only applied to vrpn_Analog values,
			not vrpn_Button
		*/
		void report(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);

		/// Time stamp of last read event
		struct timeval _timestamp;

		/// Helper routine to initialize state of the WiiMote.
		void initialize_wiimote_state(void);

		/// Helper functions to handle events
		void handle_event(void);

		/// Helper function to connect a wiimote
		void connect_wiimote(int timeout);

		/// Helper function that defines a mapping for button ids:
		unsigned map_button(unsigned btn);
};

#endif  // VRPN_USE_WIIUSE

#endif  // VRPN_WIIMOTE_H
