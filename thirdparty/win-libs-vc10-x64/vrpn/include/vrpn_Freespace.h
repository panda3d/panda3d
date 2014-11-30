/**
 * This provides an interface to devices powered by Hillcrest Lab's Freespace.  
 * Freespace is a sensor technology for tracking relative movement of a handheld devic.
 * A freespace device exposes itself as a combination of 3 VRPN interfaces.
 * as a tracker, with linear acceleration and angular velocity,
 * as a Dial, with a single dial for an onboard scrollwheel (loop pointer)
 * and as a Button for the various buttons on the device.
 * 
 * The implementation here uses the synchronous API to read data from the device, 
 * but could fairly easily be modified to use the asynchronous API.  This may be desired as 
 * a cleaner way to provide access to multiple freespace devices and/or handle hot-plugging
 * of devices (such as the loop, which uses a USB dongle for communications).
 * 
 * More info on libfreespace can be found at http://libfreespace.hillcrestlabs.com
 * 
 */

#ifndef VRPN_FREESPACE_H
#define VRPN_FREESPACE_H

#include "vrpn_Configure.h"

#ifdef VRPN_USE_FREESPACE

#include "vrpn_Tracker.h"
#include "vrpn_Button.h"
#include "vrpn_Dial.h"

#include <freespace/freespace.h>


class VRPN_API vrpn_Freespace :
	public vrpn_Tracker_Server,   // for the positional data
	public vrpn_Button,           // for the actual buttons
	public vrpn_Dial              // for the scroll wheel
{
public:
	/**
	 * Create a freespace server using the given FreespaceDeviceId.  This 
	 * factory will automatically initialize the libfreespace library as needed.
	 * This method will open the device and configure it to receive relavant 
	 * messages.  See the test_freespace.C, or libfreespace-examples, for steps to 
	 * initialize the library and enumerate devices.
	 */

    // Freespace devices may report User frames (position and orientation in a quaternion),
    // Body frames (angular velocity and linear acceleration), Mouse reports (delta X, delta Y)
    // or some combination of them.  You will probably want at least body or user frame
    // messages.
    // Another thing to note is that for some devices, enabling multiple reports dimishes 
    // the effective rate of data since the device can only send so many bits per second.
    // This could fairly easily get added as a configuration setting.

	static vrpn_Freespace* create(const char *name, 
                                  vrpn_Connection *conn, 
				                  int device_index = 0,
                                  bool send_body_frames = false,
                                  bool send_user_frames = true);
	virtual ~vrpn_Freespace(void);
	/**
	 * Main loop.  This will try to read data from the loop, and send 
	 * appropriate messages
	 * to a VRPN client.
	 */
	virtual void mainloop(void);

private:
	static void freespaceInit();
	/**
	 * private constructor since opening of the device can fail.
	 */
	vrpn_Freespace(FreespaceDeviceId freespaceId,
		struct FreespaceDeviceInfo* deviceInfo,
		const char *name, 
        vrpn_Connection *c);

    void handleUserFrame(const struct freespace_UserFrame&);
    void handleBodyFrame(const struct freespace_BodyFrame&);
	void handleLinkStatus(const struct freespace_LinkStatus&);

    void deviceSetConfiguration(bool send_body_frames, bool send_user_frames);
    void deviceConfigure();
    void deviceUnconfigure();

    bool _sendBodyFrames;
    bool _sendUserFrames;
    struct timeval _timestamp;

protected:
    FreespaceDeviceId _freespaceDevice;
    FreespaceDeviceInfo _deviceInfo;
    vrpn_float64 _lastBodyFrameTime;
};
#endif //VRPN_USE_FREESPACE

#endif // VRPN_FREESPACE_H
