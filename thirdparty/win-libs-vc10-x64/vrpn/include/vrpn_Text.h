/* vrpn_Text.h
    Definition of user-level access to the text sending and retrieving
    functions within VRPN. These are wrappers around the vrpn_BaseClass
    routines, since basic text functions have been pulled into these
    classes.
*/

#ifndef	VRPN_TEXT_H

#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Connection.h"
#include "vrpn_BaseClass.h"

// text-message time value meaning "go find out what time it is right now"
const struct timeval vrpn_TEXT_NOW = { 0 , 0 };

/// Structure passed back to user-level code from a vrpn_Text_Receiver.
typedef	struct _vrpn_TEXTCB {
	struct timeval	msg_time;	// Time of the message
	char		message[vrpn_MAX_TEXT_LEN];	// The message
	vrpn_TEXT_SEVERITY	type;
	vrpn_uint32		level;
} vrpn_TEXTCB;

/// Description of the callback function type.
typedef void (VRPN_CALLBACK *vrpn_TEXTHANDLER)(void *userdata, const vrpn_TEXTCB info);

//----------------------------------------------------------
//************** Users deal with the following *************

/// Allows a user to send text messages from a device (usually,
// the send_text_message() function is protected).  It provides
// the needed function definitions for vrpn_BaseClass.

class VRPN_API vrpn_Text_Sender: public vrpn_BaseClass {
  public:
	vrpn_Text_Sender(const char *name, vrpn_Connection *c = NULL) :
		vrpn_BaseClass(name, c) { init(); };

	/// Mainloop the connection to send the message.
	void mainloop(void) { server_mainloop(); if (d_connection) d_connection->mainloop(); };

	/// Send a text message.
	int send_message(const char *msg, 
			    vrpn_TEXT_SEVERITY type = vrpn_TEXT_NORMAL,
			    vrpn_uint32 level = 0,
			    const struct timeval time = vrpn_TEXT_NOW);
  protected:
      /// No types to register beyond the text, which is done in BaseClass.
      virtual int register_types(void) { return 0; };
};

/// Allows a user to handle text messages directly, in addition too having the
// standard VRPN printing functions handle them.

class VRPN_API vrpn_Text_Receiver: public vrpn_BaseClass {
  public:
	vrpn_Text_Receiver (const char * name, vrpn_Connection * c = NULL);
	virtual ~vrpn_Text_Receiver (void);
	virtual int register_message_handler(void *userdata,
		vrpn_TEXTHANDLER handler) {
	  return d_callback_list.register_handler(userdata, handler);
	};

        virtual int unregister_message_handler(void *userdata,
		vrpn_TEXTHANDLER handler) {
	  return d_callback_list.unregister_handler(userdata, handler);
	}

	virtual void mainloop(void) { if (d_connection) { d_connection->mainloop(); }; client_mainloop(); };

  protected:
	static int VRPN_CALLBACK handle_message (void * userdata, vrpn_HANDLERPARAM p);
	vrpn_Callback_List<vrpn_TEXTCB>	d_callback_list;

	/// No types to register beyond the text, which is done in BaseClass.
	virtual int register_types(void) { return 0; };
};

#define	VRPN_TEXT_H
#endif
