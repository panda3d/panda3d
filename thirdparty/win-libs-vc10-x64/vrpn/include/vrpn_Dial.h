// vrpn_Dial.h
//	This implements a Dial class. A dial is an object that spins,
// possibly without bound. It returns the fraction of a revolution that
// it has turned as its message type.

#ifndef VRPN_DIAL_H
#define VRPN_DIAL_H

const	int vrpn_DIAL_MAX = 128;

#include "vrpn_Connection.h"
#include "vrpn_BaseClass.h"

class VRPN_API vrpn_Dial : public vrpn_BaseClass {
public:
	vrpn_Dial (const char * name, vrpn_Connection * c = NULL);

  protected:
	vrpn_float64	dials[vrpn_DIAL_MAX];
	vrpn_int32	num_dials;
	struct timeval	timestamp;
	vrpn_int32 change_m_id;		// change message id

	virtual int register_types(void);
	virtual vrpn_int32 encode_to(char *buf, vrpn_int32 buflen,
		vrpn_int32 dial, vrpn_float64 delta);
        virtual void report_changes (void);  // send report iff changed
	virtual void report (void);  // send report
};


//----------------------------------------------------------
// Example server for an array of dials
//	This will generate an array of dials that all spin at the same
// rate (revolutions/second), and which send reports at a different rate
// (updates/second). A real server would send reports whenever it saw
// dials changing, and would not have the spin_rate or update_rate parameters.
//	This server can be used for testing to make sure a client is
// working correctly, and to ensure that a connection to a remote server
// is working (by running the example server with the name of the device that
// the real server would use).

class VRPN_API vrpn_Dial_Example_Server: public vrpn_Dial {
public:
	vrpn_Dial_Example_Server(const char * name, vrpn_Connection * c,
		vrpn_int32 numdials = 1, vrpn_float64 spin_rate = 1.0,
		vrpn_float64 update_rate = 10.0);
	virtual void mainloop();
protected:
	vrpn_float64	_spin_rate;	// The rate at which to spin (revolutions/sec)
	vrpn_float64	_update_rate;	// The rate at which to update (reports/sec)
	// The dials[] array within the parent is used for the values
	// The num_dials within the parent is used
	// The timestamp field within the parent structure is used for timing
	// The report_changes() or report() functions within the parent are used
};

//----------------------------------------------------------
//************** Users deal with the following *************

// User routine to handle a change in dial values.  This is called when
// the dial callback is called (when a message from its counterpart
// across the connetion arrives).


typedef	struct _vrpn_DIALCB {
	struct timeval	msg_time;	// Timestamp when change happened
	vrpn_int32	dial;		// which dial changed
	vrpn_float64	change;		// Fraction of a revolution it changed
} vrpn_DIALCB;

typedef void (VRPN_CALLBACK *vrpn_DIALCHANGEHANDLER) (void * userdata,
					  const vrpn_DIALCB info);

// Open a dial device that is on the other end of a connection
// and handle updates from it.  This is the type of device
// that user code will deal with.

class VRPN_API vrpn_Dial_Remote: public vrpn_Dial {
  public:
	// The name of the device to connect to.
        // Optional argument to be used when the Remote MUST listen on
        // a connection that is already open.
	vrpn_Dial_Remote (const char * name, vrpn_Connection * c = NULL);
	~vrpn_Dial_Remote();

	// This routine calls the mainloop of the connection it's on
	virtual void mainloop();

	// (un)Register a callback handler to handle dial updates
	virtual int register_change_handler(void *userdata,
		vrpn_DIALCHANGEHANDLER handler) {
	  return d_callback_list.register_handler(userdata, handler);
	};
	virtual int unregister_change_handler(void *userdata,
		vrpn_DIALCHANGEHANDLER handler) {
	  return d_callback_list.unregister_handler(userdata, handler);
	}

  protected:
	vrpn_Callback_List<vrpn_DIALCB> d_callback_list;

	static int VRPN_CALLBACK handle_change_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
