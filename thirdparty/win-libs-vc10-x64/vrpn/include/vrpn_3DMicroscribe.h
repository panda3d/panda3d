#ifndef VRPN_3DMICROSCRIBE_H
#define VRPN_3DMICROSCRIBE_H

#include "vrpn_Connection.h"
#include "vrpn_Tracker.h"
#include "vrpn_Button.h"

class VRPN_API vrpn_3DMicroscribe: public vrpn_Tracker
			,public vrpn_Button
{
  public:
	// Offset is in meters.  Scale is an abomination and should not be
	// used.  All tracker reports should be in meters in VRPN.
	vrpn_3DMicroscribe (const char * name, vrpn_Connection * c,
			const char * Port, long int BaudRate,
			float OffsetX = 0.0f, float OffsetY = 0.0f, float OffsetZ = 0.0f,
			float Scale=1.0f);

	~vrpn_3DMicroscribe () {};

	/// Called once through each main loop iteration to handle updates.
	virtual void mainloop ();

	virtual int reset(void);  ///< Set device back to starting config

  protected:
	  float m_OffSet[3];
	  float m_Scale;
	  int m_PortNumber; //!< port number
	  long int m_BaudRate; //!< baud rate


	int _numbuttons;          ///< How many buttons to open
	unsigned char buf[512];	  ///< Buffer of characters in report,
	int bufpos;               ///< Current char pos in buffer 
        int packtype;             ///< What kind of packet we are decoding
        int packlen;              ///< Expected packet length
        int escapedchar;          ///< We're processing an escaped char
        int erroroccured;         ///< A device error has occured
        int resetoccured;         ///< A reset event has occured
	struct timeval timestamp; ///< Time of the last report from the device

	void ConvertOriToQuat(float ori[3]); //< directly put the values in the quat for message sending
	virtual	void clear_values(void); ///< Set all buttons, analogs and encoders back to 0

	/// Try to read reports from the device.  Returns 1 if a complete 
        /// report received, 0 otherwise.  Sets status to current mode.
	virtual	int get_report(void);

	/// send report if changed
        virtual void report_changes (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);

        /// send report whether or not changed
        virtual void report (vrpn_uint32 class_of_service
                    = vrpn_CONNECTION_LOW_LATENCY);

        // NOTE:  class_of_service is only applied to vrpn_Tracker
        //  values, not vrpn_Button, which are always vrpn_RELIABLE
};

#endif
