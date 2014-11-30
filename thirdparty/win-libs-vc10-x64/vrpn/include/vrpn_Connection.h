#ifndef VRPN_CONNECTION_H
#define VRPN_CONNECTION_H

#include <stdio.h>  // for FILE
#include "vrpn_Shared.h"

// Don't complain about using sprintf() in Windows.
#ifdef _WIN32
#pragma warning ( disable : 4995 4996 )
#endif

#if defined (__ANDROID__)
#include <bitset>
#endif




// This is the list of states that a connection can be in
// (possible values for status).  doing_okay() returns VRPN_TRUE
// for connections > BROKEN.
enum vrpn_ConnectionStatus {LISTEN             = (1),
                       CONNECTED          = (0),
                       COOKIE_PENDING     = (-1),
                       TRYING_TO_CONNECT  = (-2),
                       BROKEN             = (-3),
                       LOGGING            = (-4)};

class VRPN_API	vrpn_File_Connection;  // Forward declaration for get_File_Connection()

/// This structure is what is passed to a vrpn_Connection message callback.
/// It is used by objects, but not normally by user code.
#ifndef _DEFINED_HANDLERPARAM
#define _DEFINED_HANDLERPARAM
struct vrpn_HANDLERPARAM {
	vrpn_int32	type;
	vrpn_int32	sender;
	struct timeval	msg_time;
	vrpn_int32	payload_len;
	const char	*buffer;
};
#endif

/// Type of a message handler for vrpn_Connection messages.
typedef	int (VRPN_CALLBACK *vrpn_MESSAGEHANDLER)(void *userdata, vrpn_HANDLERPARAM p);
/// Type of handler for filters on logfiles is the same as connection handler
typedef	vrpn_MESSAGEHANDLER vrpn_LOGFILTER;

/// VRPN buffers are aligned on 8 byte boundaries so that we can pack and
// unpack doubles into them on architectures that cannot handle unaligned access.
const	unsigned    vrpn_ALIGN = 8;

// Types now have their storage dynamically allocated, so we can afford
// to have large tables.  We need at least 150-200 for the microscope
// project as of Jan 98, and will eventually need two to three times that
// number.
const	int   vrpn_CONNECTION_MAX_SENDERS = 2000;
const	int   vrpn_CONNECTION_MAX_TYPES = 2000;

// vrpn_ANY_SENDER can be used to register callbacks on a given message
// type from any sender.

const	int vrpn_ANY_SENDER = -1;

// vrpn_ANY_TYPE can be used to register callbacks for any USER type of
// message from a given sender.  System messages are handled separately.

const	int vrpn_ANY_TYPE = -1;

// Buffer lengths for TCP and UDP.
// TCP is an arbitrary number that can be changed by the user
// using vrpn_Connection::set_tcp_outbuf_size().
// UDP is set based on Ethernet maximum transmission size;  trying
// to send a message via UDP which is longer than the MTU of any
// intervening physical network may cause untraceable failures,
// so for now we do not expose any way to change the UDP output
// buffer size.  (MTU = 1500 bytes, - 28 bytes of IP+UDP header)

const	int vrpn_CONNECTION_TCP_BUFLEN = 64000;
const	int vrpn_CONNECTION_UDP_BUFLEN = 1472;

/// Number of endpoints that a server connection can have.  Arbitrary limit.

const	int vrpn_MAX_ENDPOINTS = 256;

// System message types

const	vrpn_int32  vrpn_CONNECTION_SENDER_DESCRIPTION	= (-1);
const	vrpn_int32  vrpn_CONNECTION_TYPE_DESCRIPTION	= (-2);
const	vrpn_int32  vrpn_CONNECTION_UDP_DESCRIPTION	= (-3);
const	vrpn_int32  vrpn_CONNECTION_LOG_DESCRIPTION	= (-4);
const	vrpn_int32  vrpn_CONNECTION_DISCONNECT_MESSAGE	= (-5);

// Classes of service for messages, specify multiple by ORing them together
// Priority of satisfying these should go from the top down (RELIABLE will
// override all others).
// Most of these flags may be ignored, but RELIABLE is guaranteed
// to be available.

const	vrpn_uint32 vrpn_CONNECTION_RELIABLE		= (1<<0);
const	vrpn_uint32 vrpn_CONNECTION_FIXED_LATENCY	= (1<<1);
const	vrpn_uint32 vrpn_CONNECTION_LOW_LATENCY		= (1<<2);
const	vrpn_uint32 vrpn_CONNECTION_FIXED_THROUGHPUT	= (1<<3);
const	vrpn_uint32 vrpn_CONNECTION_HIGH_THROUGHPUT	= (1<<4);

// What to log
const	long	vrpn_LOG_NONE		= (0);
const	long	vrpn_LOG_INCOMING	= (1<<0);
const	long	vrpn_LOG_OUTGOING	= (1<<1);

// If defined, will filter out messages:  if the remote side hasn't
// registered a type, messages of that type won't be sent over the
// link.  WARNING:  auto-type-registration breaks this.
//#define vrpn_FILTER_MESSAGES

// These are the strings that define the system-generated message
// types that tell when connections are received and dropped.
extern	VRPN_API const char *vrpn_got_first_connection;
extern	VRPN_API const char *vrpn_got_connection;
extern	VRPN_API const char *vrpn_dropped_connection;
extern	VRPN_API const char *vrpn_dropped_last_connection;

// vrpn_CONTROL is the sender used for notification messages sent to the user
// from the local VRPN implementation (got_first_connection, etc.)
// and for control messages sent by auxiliary services.  (Such as
// class vrpn_Controller, which will be introduced in a future revision.)

extern	VRPN_API const char *vrpn_CONTROL;

/// Length of names within VRPN
typedef char cName [100];

// Placed here so vrpn_FileConnection can use it too.
struct VRPN_API vrpn_LOGLIST {
  vrpn_HANDLERPARAM data;
  vrpn_LOGLIST * next;
  vrpn_LOGLIST * prev;
};

// HACK
// These structs must be declared outside of vrpn_Connection
// (although we'd like to make them protected/private members)
// because aCC on PixelFlow doesn't handle nested classes correctly.

// Description of a callback entry for a user type.
#ifndef _DEFINED_MSGCALLBACKENTRY
#define _DEFINED_MSGCALLBACKENTRY
struct vrpnMsgCallbackEntry {
  vrpn_MESSAGEHANDLER	handler;	// Routine to call
  void			* userdata;	// Passed along
  vrpn_int32		sender;		// Only if from sender
  vrpnMsgCallbackEntry	* next;		// Next handler
};

struct vrpnLogFilterEntry {
  vrpn_LOGFILTER filter;   // routine to call
  void * userdata;         // passed along
  vrpnLogFilterEntry * next;
};
#endif

class VRPN_API	vrpn_Connection;
class VRPN_API	vrpn_Log;
class VRPN_API	vrpn_TranslationTable;
class VRPN_API	vrpn_TypeDispatcher;

// Encapsulation of the data and methods for a single generic connection
// to take care of one part of many clients talking to a single server.
// This will only be used from within the vrpn_Connection class;  it should
// not be instantiated by users or devices.
// Should not be visible!

class VRPN_API vrpn_Endpoint {

  public:

    vrpn_Endpoint (vrpn_TypeDispatcher * dispatcher,
                   vrpn_int32 * connectedEndpointCounter);
    virtual ~vrpn_Endpoint (void);

    // ACCESSORS

    /// Returns the local mapping for the remote type (-1 if none).
    int local_type_id (vrpn_int32 remote_type) const;

    /// Returns the local mapping for the remote sender (-1 if none).
    int local_sender_id (vrpn_int32 remote_sender) const;

    virtual vrpn_bool doing_okay (void) const = 0;

    // MANIPULATORS

    void init (void);

    virtual int mainloop (timeval * timeout) = 0;

    // Clear out the remote mapping list. This is done when a
    // connection is dropped and we want to try and re-establish
    // it.
    void clear_other_senders_and_types (void);

    // A new local sender or type has been established; set
    // the local type for it if the other side has declared it.
    // Return 1 if the other side has one, 0 if not.
    int newLocalSender (const char * name, vrpn_int32 which);
    int newLocalType (const char * name, vrpn_int32 which);

    // Adds a new remote type/sender and returns its index.
    // Returns -1 on error.
    int newRemoteType (cName type_name, vrpn_int32 remote_id,
                       vrpn_int32 local_id);
    int newRemoteSender (cName sender_name, vrpn_int32 remote_id,
                         vrpn_int32 local_id);

    // Pack a message that will be sent the next time mainloop() is called.
    // Turn off the RELIABLE flag if you want low-latency (UDP) send.
    virtual int pack_message (vrpn_uint32 len, struct timeval time,
            vrpn_int32 type, vrpn_int32 sender, const char * buffer,
            vrpn_uint32 class_of_service) = 0;

    // send pending report, clear the buffer.
    // This function was protected, now is public, so we can use it
    // to send out intermediate results without calling mainloop
    virtual int send_pending_reports (void) = 0;

    int pack_log_description (void);
      ///< Packs the log description set by setup_new_connection().

    virtual int setup_new_connection (void) = 0;
      ///< Sends the magic cookie and other information to its
      ///< peer.  It is called by both the client and server setup routines.

    virtual void poll_for_cookie (const timeval * timeout = NULL) = 0;
    virtual int finish_new_connection_setup (void) = 0;

   virtual void drop_connection (void) = 0;
      ///< Should only be called by vrpn_Connection::drop_connection(),
      ///< since there's more housecleaning to do at that level.  I suppose
      ///< that argues against separating this function out.

    virtual void clearBuffers (void) = 0;
      ///< Empties out the TCP and UDP send buffers.
      ///< Needed by vrpn_FileConnection to get at {udp,tcp}NumOut.

    int pack_sender_description (vrpn_int32 which);
      ///< Packs a sender description over our socket.

    int pack_type_description (vrpn_int32 which);
      ///< Packs a type description.

    int status;

//XXX These should be protected; making them so will lead to making
//    the code split the functions between Endpoint and Connection
//    protected:

    long d_remoteLogMode;	// Mode to put the remote logging in
    char * d_remoteInLogName;	// Name of the remote log file
    char * d_remoteOutLogName;	// Name of the remote log file

    // Name of the remote host we are connected to.  This is kept for
    // informational purposes.  It is printed by the ceiling server,
    // for example.
    char rhostname [150];

    // Logging - TCH 19 April 00;  changed into two logs 16 Feb 01

    vrpn_Log * d_inLog;
    vrpn_Log * d_outLog;

    void setLogNames (const char * inName, const char * outName);
    int openLogs (void);

    // Routines that handle system messages
    // Visible so that vrpn_Connection can pass them to the Dispatcher
    static int VRPN_CALLBACK handle_sender_message (void * userdata, vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_type_message (void * userdata, vrpn_HANDLERPARAM p);
	
    // Routines to inform the endpoint of the connection of 
    // which it is a part.
    void setConnection( vrpn_Connection* conn ) {  d_parent = conn;  }
    vrpn_Connection* getConnection( ) {  return d_parent;  }

  protected:

    virtual int dispatch (vrpn_int32 type, vrpn_int32 sender,
                  timeval time, vrpn_uint32 payload_len,
                  char * bufptr);

    int tryToMarshall (char * outbuf, vrpn_int32 &buflen, vrpn_int32 &numOut,
                       vrpn_uint32 len, timeval time,
                       vrpn_int32 type, vrpn_int32 sender,
                       const char * buffer, vrpn_uint32 classOfService);
      ///< Calls marshall_message();  if that fails, calls
      ///< send_pending_reports() and then marshalls again.
      ///< Returns the number of characters successfully marshalled.

    int marshall_message (char * outbuf,vrpn_uint32 outbuf_size,
                          vrpn_uint32 initial_out,
                          vrpn_uint32 len, struct timeval time,
                          vrpn_int32 type, vrpn_int32 sender,
                          const char * buffer,
                          vrpn_uint32 sequenceNumber);

    // The senders and types we know about that have been described by
    // the other end of the connection.  Also, record the local mapping
    // for ones that have been described with the same name locally.
    // The arrays are indexed by the ID from the other side, and store
    // the name and local ID that corresponds to each.

    vrpn_TranslationTable * d_senders;
    vrpn_TranslationTable * d_types;

    vrpn_TypeDispatcher * d_dispatcher;
    vrpn_int32 * d_connectionCounter;

    vrpn_Connection * d_parent;
};

// Encapsulation of the data and methods for a single IP-based connection
// to take care of one part of many clients talking to a single server.
// This will only be used from within the vrpn_Connection_IP class;  it should
// not be instantiated by users or devices.
// Should not be visible!

class VRPN_API vrpn_Endpoint_IP : public vrpn_Endpoint {

  public:

    vrpn_Endpoint_IP (vrpn_TypeDispatcher * dispatcher,
                   vrpn_int32 * connectedEndpointCounter);
    virtual ~vrpn_Endpoint_IP (void);

    // ACCESSORS
    virtual vrpn_bool doing_okay (void) const;

    /// True if the UDP outbound is open, False if not.
    vrpn_bool outbound_udp_open (void) const;

    vrpn_int32 tcp_outbuf_size (void) const;
    vrpn_int32 udp_outbuf_size (void) const;

    // MANIPULATORS

    void init (void);

    int mainloop (timeval * timeout);

    // Pack a message that will be sent the next time mainloop() is called.
    // Turn off the RELIABLE flag if you want low-latency (UDP) send.
    int pack_message (vrpn_uint32 len, struct timeval time,
            vrpn_int32 type, vrpn_int32 sender, const char * buffer,
            vrpn_uint32 class_of_service);

    // send pending report, clear the buffer.
    // This function was protected, now is public, so we can use it
    // to send out intermediate results without calling mainloop
    virtual int send_pending_reports (void);

    int pack_udp_description (int portno);

    int handle_tcp_messages (const timeval * timeout);
    int handle_udp_messages (const timeval * timeout);

    int connect_tcp_to (const char * msg);
    int connect_tcp_to (const char * addr, int port);
      ///< Connects d_tcpSocket to the specified address (msg = "IP port");
      ///< sets status to COOKIE_PENDING;  returns 0 on success, -1 on failure
    int connect_udp_to (const char * addr, int port);
      ///< Connects d_udpSocket to the specified address and port;
      ///< returns 0 on success, sets status to BROKEN and returns -1
      ///< on failure.

    vrpn_int32 set_tcp_outbuf_size (vrpn_int32 bytecount);

    int setup_new_connection (void);
      ///< Sends the magic cookie and other information to its
      ///< peer.  It is called by both the client and server setup routines.

    void poll_for_cookie (const timeval * timeout = NULL);
    int finish_new_connection_setup (void);

    void drop_connection (void);
      ///< Should only be called by vrpn_Connection::drop_connection(),
      ///< since there's more housecleaning to do at that level.  I suppose
      ///< that argues against separating this function out.

    void clearBuffers (void);
      ///< Empties out the TCP and UDP send buffers.
      ///< Needed by vrpn_FileConnection to get at {udp,tcp}NumOut.

    void setNICaddress (const char *);

//XXX These should be protected; making them so will lead to making
//    the code split the functions between Endpoint and Connection
//    protected:

    SOCKET d_tcpSocket;

    // This section deals with when a client connection is trying to
    // establish (or re-establish) a connection with its server. It
    // keeps track of what we need to know to make this happen.

    SOCKET d_tcpListenSocket;
    int	d_tcpListenPort;
      ///< Socket and port that the client listens on
      ///< when lobbing datagrams at the server and
      ///< waiting for it to call back.

    char *d_remote_machine_name;	// Machine to call
    int	d_remote_port_number;	// Port to connect to on remote machine
    timeval d_last_connect_attempt;	// When the last UDP lob occured

    vrpn_bool	d_tcp_only;
      ///< For connections made through firewalls or NAT with the
      ///< tcp: URL, we do not want to allow the endpoints on either
      ///< end to open a UDP link to their counterparts.  If this is
      ///< the case, then this flag should be set to true.

  protected:

    int getOneTCPMessage (int fd, char * buf, int buflen);
    int getOneUDPMessage (char * buf, int buflen);

    SOCKET d_udpOutboundSocket;
    SOCKET d_udpInboundSocket;
      ///< Inbound unreliable messages come here.
      ///< Need one for each due to different
      ///< clock synchronization for each; we
      ///< need to know which server each message is from.
      ///< XXX Now that we don't need multiple clocks, can we collapse this?

    char * d_tcpOutbuf;
    char * d_udpOutbuf;
    vrpn_int32 d_tcpBuflen;
    vrpn_int32 d_udpBuflen;
    vrpn_int32 d_tcpNumOut;
    vrpn_int32 d_udpNumOut;

    vrpn_int32 d_tcpSequenceNumber;
    vrpn_int32 d_udpSequenceNumber;

    vrpn_float64 d_tcpAlignedInbuf
         [vrpn_CONNECTION_TCP_BUFLEN / sizeof(vrpn_float64) + 1];
    vrpn_float64 d_udpAlignedInbuf
         [vrpn_CONNECTION_UDP_BUFLEN / sizeof(vrpn_float64) + 1];
    char * d_tcpInbuf;
    char * d_udpInbuf;

    char * d_NICaddress;
};

// Generic connection class not specific to the transport mechanism.
// It abstracts all of the common functions.  Specific implementations
// for IP, MPI, and other transport mechanisms follow.
class VRPN_API vrpn_Connection {

  protected:
    // Constructor for server connection.  This cannot be called
    // directly any more because vrpn_Connection is an abstract base
    // class.  Call vrpn_create_server_connection() to make a server
    // of arbitrary type based on a name.
    vrpn_Connection (const char * local_in_logfile_name,
                     const char * local_out_logfile_name,
                     vrpn_Endpoint_IP * (* epa) (vrpn_Connection *,
                       vrpn_int32 *) = allocateEndpoint);

    // Constructor for client connection.  This cannot be called
    // directly because vrpn_Connection is an abstract base class.
    // Call vrpn_get_connection_by_name() to create a client connection.
    vrpn_Connection (const char * local_in_logfile_name,
		     const char * local_out_logfile_name,
		     const char * remote_in_logfile_name,
		     const char * remote_out_logfile_name,
		     vrpn_Endpoint_IP * (* epa) (vrpn_Connection *,
		          vrpn_int32 *) = allocateEndpoint);

  public:

    virtual ~vrpn_Connection (void);

    // Returns 1 if the connection is okay, 0 if not
    virtual vrpn_bool doing_okay (void) const;
    virtual vrpn_bool connected (void) const;

    // This function returns the logfile names of this connection in
    // the parameters.  It will allocate memory for the name of each 
    // log file in use.  If no logging of a particular type is happening, 
    // then *(X_Y_logname) will be set to NULL.
    // IMPORTANT:  code calling this function is responsible for freeing
    // the memory allocated for these strings.
    void get_log_names( char** local_in_logname, char** local_out_logname,
			char** remote_in_logname, char** remote_out_logname );

    // Call each time through program main loop to handle receiving any
    // incoming messages and sending any packed messages.
    // Returns -1 when connection dropped due to error, 0 otherwise.
    // (only returns -1 once per connection drop).
    // Optional argument is TOTAL time to block on select() calls;
    // there may be multiple calls to select() per call to mainloop(),
    // and this timeout will be divided evenly between them.
    virtual int mainloop (const struct timeval * timeout = NULL) = 0;

    // Get a token to use for the string name of the sender or type.
    // Remember to check for -1 meaning failure.
    virtual vrpn_int32 register_sender (const char * name);
    virtual vrpn_int32 register_message_type (const char * name);

    // Set up (or remove) a handler for a message of a given type.
    // Optionally, specify which sender to handle messages from.
    // Handlers will be called during mainloop().
    // Your handler should return 0 or a communication error is assumed
    // and the connection will be shut down.
    virtual int register_handler(vrpn_int32 type,
	    vrpn_MESSAGEHANDLER handler, void *userdata,
	    vrpn_int32 sender = vrpn_ANY_SENDER);
    virtual	int unregister_handler(vrpn_int32 type,
	    vrpn_MESSAGEHANDLER handler, void *userdata,
	    vrpn_int32 sender = vrpn_ANY_SENDER);

    // Pack a message that will be sent the next time mainloop() is called.
    // Turn off the RELIABLE flag if you want low-latency (UDP) send.
    virtual int pack_message(vrpn_uint32 len, struct timeval time,
	    vrpn_int32 type, vrpn_int32 sender, const char * buffer,
	    vrpn_uint32 class_of_service);

    // send pending report, clear the buffer.
    // This function was protected, now is public, so we can use it
    // to send out intermediate results without calling mainloop
    virtual int send_pending_reports (void) = 0;

    // Returns the time since the connection opened.
    // Some subclasses may redefine time.
    virtual int time_since_connection_open( struct timeval * elapsed_time );

    // returns the current time in the connection (since the epoch -- UTC time).
    virtual timeval get_time( );

    // Returns the name of the specified sender/type, or NULL
    // if the parameter is invalid.  Only works for user
    // messages (type >= 0).
    virtual const char * sender_name (vrpn_int32 sender);
    virtual const char * message_type_name (vrpn_int32 type);

    // Sets up a filter function for logging.
    // Any user message to be logged is first passed to this function,
    // and will only be logged if the function returns zero (XXX).
    // NOTE:  this only affects local logging - remote logging
    // is unfiltered!  Only user messages are filtered;  all system
    // messages are logged.
    // Returns nonzero on failure.
    virtual int register_log_filter (vrpn_LOGFILTER filter,
                                     void * userdata);

    // Save any messages on any endpoints which have been logged so far.
    virtual int save_log_so_far();

    // vrpn_File_Connection implements this as "return this" so it
    // can be used to detect a File_Connection and get the pointer for it
    virtual vrpn_File_Connection * get_File_Connection (void);

    // This function should be seldom used.  It is here for the case of
    // the vrpn_Imager, whose servers do not follow "The VRPN Way" because
    // they try to jam more data into the network than there is bandwidth
    // to support it.  As a result, a client may call mainloop() on the
    // connection and have it never return -- there is always more data
    // in the network to read, so we never hand control back to the main
    // program.  The reason for the name comes from an old U.S. cartoon
    // called "The Jetsons".  In it, George Jetson is running on a
    // treadmill when it goes out of control and starts spinning so fast
    // that he can't even run fast enough to reach the controls and turn
    // it off.  He cries out to his wife, "Jane!  Stop this crazy thing!"
    // The parameter specifies a trigger: if more than the specified number
    // of messages come in on a given input channel during one mainloop()
    // call, the connection should stop looking for more messages.  NOTE:
    // this does not guarantee that only this many messages will be received,
    // only that the connection will stop looking for new ones on a given
    // channel once that many have been recived (for example, UDP channels
    // will parse all the rest of the messages in a packet before stopping).
    // A value of 0 turns off the limit, and will cause all incoming messages
    // to be handled before returning.
    void Jane_stop_this_crazy_thing(vrpn_uint32 stop_looking_after) {
      d_stop_processing_messages_after = stop_looking_after;
    };
    vrpn_uint32 get_Jane_value(void) { return d_stop_processing_messages_after; };

  protected:

    // If this value is greater than zero, the connection should stop
    // looking for new messages on a given endpoint after this many
    // are found.
    vrpn_uint32 d_stop_processing_messages_after;

    int connectionStatus;		// Status of the connection

    static vrpn_Endpoint_IP * allocateEndpoint (vrpn_Connection *,
                                             vrpn_int32 * connectedEC);
    ///< Redefining this and passing it to constructors
    ///< allows a subclass to use a different subclass of Endpoint.
    ///< It should do NOTHING but return an endpoint
    ///< of the appropriate class;  it may not access subclass data,
    ///< since it'll be called from a constructor

    // Sockets used to talk to remote Connection(s)
    // and other information needed on a per-connection basis
    vrpn_Endpoint_IP * d_endpoints [vrpn_MAX_ENDPOINTS];
    vrpn_int32 d_numEndpoints;

    vrpn_int32 d_numConnectedEndpoints;
      ///< We need to track the number of connected endpoints separately
      ///< to properly send out got-first-connection/dropped-last-connection
      ///< messages.  This value is *managed* by the Endpoints, but we
      ///< need exactly one copy per Connection, so it's on the Connection.

    // Routines that handle system messages
    static int VRPN_CALLBACK handle_log_message (void * userdata, vrpn_HANDLERPARAM p);
    static int VRPN_CALLBACK handle_disconnect_message (void * userdata,
		vrpn_HANDLERPARAM p);

    virtual void init(void);            // Base initialization for all constructors.

    int delete_endpoint (int whichEndpoint);
    int compact_endpoints (void);

    virtual int pack_sender_description (vrpn_int32 which);
      ///< Send the sender description to ALL endpoints.

    virtual int pack_type_description (vrpn_int32 which);
      ///< Send the type description to ALL endpoints.

    virtual int do_callbacks_for (vrpn_int32 type, vrpn_int32 sender,
				struct timeval time, vrpn_uint32 len,
	                        const char * buffer);

    // Returns message type ID, or -1 if unregistered
    int message_type_is_registered (const char *) const;

    // Timekeeping - TCH 30 June 98
    timeval start_time;

    //
    // Counting references to this connection.
  public:
    void addReference();
    void removeReference();

  private:
    int d_references;

    //
    // Specify whether this connection should be deleted automatically when
    //  it is no longer need (reference count reaches zero).
    // For connections created by the VRPN code (as is done in 
    //  get_connection_by_name) these should be auto-deleted.
    //  Connections created by user code should not be auto-deleted;
    //  that is up to the user to decide when finished.
    // By default, the constructor sets this to FALSE.
    // VRPN code (or user code) can set this to TRUE if it wants the
    //  connection to be deleted automatically when the last service on it
    //  is deleted 
  public:
    void setAutoDeleteStatus(bool setvalue) { d_autoDeleteStatus=setvalue; }
  private:
    bool d_autoDeleteStatus;	// FALSE by default.


  public:

    // Derived classes need access to d_dispatcher in their
    // allocateEndpoint() routine.  Several compilers won't give it to
    // them, even if they do inherit publically.  Until we figure that
    // out, d_dispatcher needs to be public.

    vrpn_TypeDispatcher * d_dispatcher;

  protected:

    int doSystemCallbacksFor (vrpn_HANDLERPARAM, void *);

    // Server logging w. multiconnection - TCH July 00
    // Use one "hidden" endpoint for outgoing logs (?),
    // standard per-endpoint logs with augmented names for incoming.
    // To make a hidden endpoint we create d_endpoints[0] and increment
    // the d_numEndpoints, but DON'T pass it d_numConnectedEndpoints
    // (although it should be safe to do so, since it should never truly
    // become connected, but we might have to "fake" it to get it to log
    // correctly).

    //vrpn_Endpoint * d_serverLogEndpoint;
    int d_serverLogCount;
    vrpn_int32 d_serverLogMode;
    char * d_serverLogName;

    vrpn_Endpoint_IP * (* d_endpointAllocator) (vrpn_Connection *,
                                             vrpn_int32 *);
    vrpn_bool d_updateEndpoint;

    virtual void updateEndpoints (void);
      ///< This function will be called on the mainloop() iteration
      ///< after *d_endpointAllocator is called, which lets subclasses
      ///< do initialization.  (They can't do so during allocateEndpoint
      ///< because it's called during the Connection constructor when
      ///< their constructors haven't executed yet.)
};

class VRPN_API vrpn_Connection_IP : public vrpn_Connection {

  protected:
    // Make a client connection.  To access this from user code,
    // call vrpn_get_connection_by_name().
    //   Create a connection -  if server_name is not a file: name,
    // makes an SDI-like connection to the named remote server
    // (otherwise functions as a non-networked messaging hub).
    // Port less than zero forces default.
    //   Currently, server_name is an extended URL that defaults
    // to VRPN connections at the port, but can be file:: to read
    // from a file.  Other extensions should maintain this, so
    // that VRPN uses URLs to name things that are to be connected
    // to.
    vrpn_Connection_IP (const char * server_name,
		 int port = vrpn_DEFAULT_LISTEN_PORT_NO,
		 const char * local_in_logfile_name = NULL,
		 const char * local_out_logfile_name = NULL,
		 const char * remote_in_logfile_name = NULL,
		 const char * remote_out_logfile_name = NULL,
		 const char * NIC_IPaddress = NULL,
		 vrpn_Endpoint_IP * (* epa) (vrpn_Connection *,
		   vrpn_int32 *) = allocateEndpoint);

  public:

    // Make a server that listens for client connections.
    // DEPRECATED: Call vrpn_create_server_connection() with the
    // NIC name and port number you want.
    vrpn_Connection_IP (unsigned short listen_port_no =
		     vrpn_DEFAULT_LISTEN_PORT_NO,
                     const char * local_in_logfile_name = NULL,
                     const char * local_out_logfile_name = NULL,
                     const char * NIC_IPaddress = NULL,
                     vrpn_Endpoint_IP * (* epa) (vrpn_Connection *,
                       vrpn_int32 *) = allocateEndpoint);

    virtual ~vrpn_Connection_IP (void);

    // This is similar to check connection except that it can be
    // used to receive requests from before a server starts up
    virtual int connect_to_client (const char * machine, int port);

    // Call each time through program main loop to handle receiving any
    // incoming messages and sending any packed messages.
    // Returns -1 when connection dropped due to error, 0 otherwise.
    // (only returns -1 once per connection drop).
    // Optional argument is TOTAL time to block on select() calls;
    // there may be multiple calls to select() per call to mainloop(),
    // and this timeout will be divided evenly between them.
    virtual int mainloop (const struct timeval * timeout = NULL);

  protected:

    // If this value is greater than zero, the connection should stop
    // looking for new messages on a given endpoint after this many
    // are found.
    vrpn_uint32 d_stop_processing_messages_after;

  protected:

    friend VRPN_API vrpn_Connection * vrpn_get_connection_by_name (
        const char * cname,
        const char * local_in_logfile_name,
        const char * local_out_logfile_name,
        const char * remote_in_logfile_name,
        const char * remote_out_logfile_name,
        const char * NIC_IPaddress,
        bool force_connection);
    friend VRPN_API vrpn_Connection * vrpn_create_server_connection (
	const char * cname,
	const char * local_in_logfile_name,
	const char * local_out_logfile_name);

    // Only used for a vrpn_Connection that awaits incoming connections
    int listen_udp_sock;	// UDP Connect requests come here
    int listen_tcp_sock;	// TCP Connection requests come here

    // Routines that handle system messages
    static int VRPN_CALLBACK handle_UDP_message (void * userdata, vrpn_HANDLERPARAM p);

    virtual void init (void);	// Called by all constructors

    // send pending report, clear the buffer.
    // This function was protected, now is public, so we can use it
    // to send out intermediate results without calling mainloop
    virtual int send_pending_reports (void);

    /// This is called by a server-side process to see if there have
    /// been any UDP packets come in asking for a connection. If there
    /// are, it connects the TCP port and then calls handle_connection().
    virtual void server_check_for_incoming_connections
                  (const struct timeval * timeout = NULL);

    // This routine is called by a server-side connection when a
    // new connection has just been established, and the tcp port
    // has been connected to it.
    virtual void handle_connection (int whichEndpoint);

    virtual void drop_connection (int whichEndpoint);

    char * d_NIC_IP;
};

// Create a client connection of arbitrary type (VRPN UDP/TCP, TCP,
// File, MPI).
// WARNING:  May not be thread safe.
// If no IP address for the NIC to use is specified, uses the default
// NIC.  If the force_reopen flag is set, a new connection will be
// made even if there was already one to that server.
// When done with the object, call removeReference() on it (which will
// delete it if there are no other references).
VRPN_API vrpn_Connection * vrpn_get_connection_by_name (
    const char * cname,
    const char * local_in_logfile_name = NULL,
    const char * local_out_logfile_name = NULL,
    const char * remote_in_logfile_name = NULL,
    const char * remote_out_logfile_name = NULL,
    const char * NIC_IPaddress = NULL,
    bool force_reopen = false);

// Create a server connection of arbitrary type (VRPN UDP/TCP, MPI).
// Returns NULL if the name is not understood or the connection cannot
// be created.
// WARNING:  May not be thread safe.
// To create a VRPN TCP/UDP server, use a name like:
//    vrpn:machine_name_or_ip:port
//    machine_name_or_ip:port
//    machine_name_or_ip
//    :port       (This port on any network card.)
// To create an MPI server, use a name like:
//    mpi:MPI_COMM_WORLD
//    mpi:comm_number
// When done with the object, call removeReference() on it (which will
// delete it if there are no other references).
VRPN_API vrpn_Connection *vrpn_create_server_connection (
    const char * cname,
    const char * local_in_logfile_name = NULL,
    const char * local_out_logfile_name = NULL);

// Lets you make one with the default settings, or just ask for a specific
// port number on the default NIC on this machine.  This matches the
// signature on the old constructor to make it easier to port existing
// servers.
inline VRPN_API vrpn_Connection *vrpn_create_server_connection (
    int port = vrpn_DEFAULT_LISTEN_PORT_NO,
    const char * local_in_logfile_name = NULL,
    const char * local_out_logfile_name = NULL,
    const char * NIC_NAME = NULL)
{
  char  name[256];
  if (NIC_NAME == NULL) {
    sprintf(name, ":%d", port);
  } else {
    sprintf(name, "%s:%d", NIC_NAME, port);
  }
  return vrpn_create_server_connection(name, local_in_logfile_name, local_out_logfile_name);
}

// Utility routines to parse names (<service>@<location specifier>)
// Both return new char [], and it is the caller's responsibility
// to delete this memory!
VRPN_API char * vrpn_copy_service_name (const char * fullname);
VRPN_API char * vrpn_copy_service_location (const char * fullname);

// Utility routines to parse file specifiers FROM service locations
//   file:<filename>
//   file://<hostname>/<filename>
//   file:///<filename>
VRPN_API char * vrpn_copy_file_name (const char * filespecifier);

// Utility routines to parse host specifiers FROM service locations
//   <hostname>
//   <hostname>:<port number>
//   x-vrpn://<hostname>
//   x-vrpn://<hostname>:<port number>
//   x-vrsh://<hostname>/<server program>,<comma-separated server arguments>
// The caller is responsible for calling delete [] on the returned character
// pointer if it is not NULL.
VRPN_API char * vrpn_copy_machine_name (const char * hostspecifier);
VRPN_API int vrpn_get_port_number (const char * hostspecifier);
VRPN_API char * vrpn_copy_rsh_program (const char * hostspecifier);
VRPN_API char * vrpn_copy_rsh_arguments (const char * hostspecifier);

// Utility routine to rename the service name of a given host specifier.
char * vrpn_set_service_name(const char * specifier, const char * newServiceName);

// Checks the buffer to see if it is a valid VRPN header cookie.
// Returns -1 on total mismatch,
// 1 on minor version mismatch or other acceptable difference,
// and 0 on exact match.
VRPN_API int check_vrpn_cookie (const char * buffer);
VRPN_API int check_vrpn_file_cookie (const char * buffer);

// Returns the size of the magic cookie buffer, plus any alignment overhead.
VRPN_API int vrpn_cookie_size (void);

VRPN_API int write_vrpn_cookie (char * buffer, int length, long remote_log_mode);

// Utility routines for reading from and writing to sockets/file descriptors
#ifndef VRPN_USE_WINSOCK_SOCKETS
 int VRPN_API vrpn_noint_block_write (int outfile, const char buffer[], int length);
 int VRPN_API vrpn_noint_block_read(int infile, char buffer[], int length);
 int VRPN_API vrpn_noint_select(int width, fd_set *readfds, fd_set *writefds, 
		     fd_set *exceptfds, struct timeval * timeout);
#else /* winsock sockets */
 int VRPN_API vrpn_noint_block_write(SOCKET outsock, char *buffer, int length);
 int VRPN_API vrpn_noint_block_read(SOCKET insock, char *buffer, int length);
#endif /* VRPN_USE_WINSOCK_SOCKETS */

/**
 * @class vrpn_ConnectionManager
 * Singleton class that keeps track of all known VRPN connections
 * and makes sure they're deleted on shutdown.
 * We make it static to guarantee that the destructor is called
 * on program close so that the destructors of all the vrpn_Connections
 * that have been allocated are called so that all open logs are flushed
 * to disk.  Each connection should add itself to this list in its
 * constructor and should remove itself from this list in its
 * destructor.
 */

//      This section holds data structures and functions to open
// connections by name.
//      The intention of this section is that it can open connections for
// objects that are in different libraries (trackers, buttons and sound),
// even if they all refer to the same connection.


class vrpn_ConnectionManager {

  public:

    ~vrpn_ConnectionManager (void);

    static vrpn_ConnectionManager & instance (void);
      // The only way to get access to an instance of this class.
      // Guarantees that there is only one, global object.
      // Also guarantees that it will be constructed the first time
      // this function is called, and (hopefully?) destructed when
      // the program terminates.

    void addConnection (vrpn_Connection *, const char * name);
    void deleteConnection (vrpn_Connection *);
      // NB implementation is not particularly efficient;  we expect
      // to have O(10) connections, not O(1000).

    vrpn_Connection * getByName (const char * name);
      // Searches through d_kcList but NOT d_anonList
      // (Connections constructed with no name)

  private:

    struct knownConnection {
      char name [1000];
      vrpn_Connection * connection;
      knownConnection * next;
    };

    knownConnection * d_kcList;
      // named connections

    knownConnection * d_anonList;
      // unnamed (server) connections

    vrpn_ConnectionManager (void);

    vrpn_ConnectionManager (const vrpn_ConnectionManager &);
      // copy constructor undefined to prevent instantiations

    static void deleteConnection (vrpn_Connection *, knownConnection **);
};


#endif // VRPN_CONNECTION_H
