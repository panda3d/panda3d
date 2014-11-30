// This is a base class interface that has been designed for use by
// scientific data-collection applications that make use of VRPN to
// connect to microscope imagers and tracking system for nanoscale
// science research at UNC.

// The idea of this interface is to enable a client GUI to start and
// stop logging of full-rate data on the server while receiving only
// a subset of the data during the experiment for preview; this keeps
// from overloading the network bandwidth with data and also keeps
// the client-side log files from filling up.  When new log file(s)
// are requested, the old log files are closed.

// Note that a particular implementation of the auxiliary logger server
// may need to know about a second connection (not the one it talks
// to its client over) in case that is where it is doing its logging. 

#ifndef	VRPN_AUXILIARY_LOGGER_H
#define	VRPN_AUXILIARY_LOGGER_H
#include <string.h>	// For memcpy()
#include  "vrpn_Connection.h"
#include  "vrpn_BaseClass.h"

class VRPN_API vrpn_Auxiliary_Logger : public vrpn_BaseClass {
public:

  vrpn_Auxiliary_Logger(const char * name, vrpn_Connection * c);

protected:
  // Handle registration of all message types we're going to deal with.
  virtual int register_types(void);
  vrpn_int32 request_logging_m_id;   // ID of remote->server request message
  vrpn_int32 report_logging_m_id;    // ID of server->client response message
  vrpn_int32 request_logging_status_m_id; // ID of remote->server status-request message

  // Pack a log description into the message whose type is passed
  // as the parameter (this is used to pack both the request and
  // report messages.
  bool pack_log_message_of_type(vrpn_int32 type,
                               const char *local_in_logfile_name,
                               const char *local_out_logfile_name,
                               const char *remote_in_logfile_name,
                               const char *remote_out_logfile_name);

  // Unpack a log description from a message into the four strings that
  // were passed in (this is used to unpack both the request and the
  // report messages).
  // NOTE: This routine will allocate space for the strings.  The caller
  // must delete [] this space when they are done with it to avoid
  // memory leaks.
  bool unpack_log_message_from_buffer(const char *buf, vrpn_int32 buflen,
                               char **local_in_logfile_name,
                               char **local_out_logfile_name,
                               char **remote_in_logfile_name,
                               char **remote_out_logfile_name);
};


// Virtual base server class for an auxiliiary logger.  An implementation must
// implement the specified message-handling functions and must call the base-
// class constructor to set up the calling of them.

class VRPN_API vrpn_Auxiliary_Logger_Server : public vrpn_Auxiliary_Logger {
public:
  vrpn_Auxiliary_Logger_Server(const char * name, vrpn_Connection * c);

  // Required for servers.
  virtual void mainloop(void) { server_mainloop(); }

protected:
  // Handle a logging-request message.  The request contains four file
  // names, two for local (to the Auxiliary server itself) and two for
  // remote (the far side of its connection to the server).  It must
  // also respond to the client with a message saying what logging has
  // been set up (using the send_logging_response function).  Logging is
  // turned off on a particular file by sending an empty-string name ("").
  // The in/out local/remote are with respect to the connection that the
  // logging is to occur on, which may or may not be the same one that the
  // client has connected to the object on using the constructor above.
  // Make sure to send a response saying what you did.
  virtual void handle_request_logging(const char *local_in_logfile_name,
                                      const char *local_out_logfile_name,
                                      const char *remote_in_logfile_name,
                                      const char *remote_out_logfile_name) = 0;

  // Send a response to the client telling it what logging has been
  // established.
  bool send_report_logging(const char *local_in_logfile_name,
                           const char *local_out_logfile_name,
                           const char *remote_in_logfile_name,
                           const char *remote_out_logfile_name)
    { if (!d_connection) { return false; }
      return pack_log_message_of_type(report_logging_m_id,
        local_in_logfile_name, local_out_logfile_name,
        remote_in_logfile_name, remote_out_logfile_name);
    }

  // Handle dropped last connection on server object by turning off
  // logging.  The static method basically looks up the this
  // pointer and calls the virtual method.  A derived class should re-implement
  // the non-static method below if it doesn't want to drop all logging or if
  // it wants to do something else in addition.  The static method basically
  // just calls the non-static method.
  virtual void handle_dropped_last_connection(void);
  vrpn_int32 dropped_last_connection_m_id;    // ID of message that all connections dropped
  static int VRPN_CALLBACK static_handle_dropped_last_connection(void *userdata, vrpn_HANDLERPARAM p );

  // Static portion of handling (unpacking) the request_logging message.  It
  // then calls the non-static virtual method above.
  static int VRPN_CALLBACK static_handle_request_logging(void *userdata, vrpn_HANDLERPARAM p );

  // Handle request for logging status.
  virtual void handle_request_logging_status( ) = 0;
  static int VRPN_CALLBACK static_handle_request_logging_status( void* userdata, vrpn_HANDLERPARAM p );
};


// Generic server that will start auxiliary logs on the connection whose name
// is passed in (which can be the same as the name of the connection it is created
// on, but does not have to be).  The "local" in and out are with respect to the
// new connection that is made; the "remote" in and out are with respect to the
// named connection.  No logging is started in the constructor.

class VRPN_API vrpn_Auxiliary_Logger_Server_Generic : public vrpn_Auxiliary_Logger_Server {
public:
  // Does not start logging, just records what to log when it is started.
  vrpn_Auxiliary_Logger_Server_Generic(const char *logger_name, const char *connection_to_log,
                                        vrpn_Connection *c = NULL);
  ~vrpn_Auxiliary_Logger_Server_Generic();

  // Close an existing logging connection, then (if any of the file
  // names are non-empty) open a new logging connection to the
  // connection we are to log (even if this process already has a
  // connection to it) and then send back the report that we've started
  // logging if we are able.  If we cannot open it, then fill in all
  // blank names for the return report.
  virtual void handle_request_logging(const char *local_in_logfile_name,
                                      const char *local_out_logfile_name,
                                      const char *remote_in_logfile_name,
                                      const char *remote_out_logfile_name);

  virtual void handle_request_logging_status( );

  // If we have an active logging connection, mainloop it and save all of its
  // pending messages in addition to handling the base-class functions.
  // Then call the parent class mainloop().
  virtual void mainloop(void) {
    if (d_logging_connection) {
      d_logging_connection->mainloop();
      d_logging_connection->save_log_so_far();
    }
    vrpn_Auxiliary_Logger_Server::mainloop();
  }

protected:
  char              *d_connection_name;     // Name to connect to when logging.
  vrpn_Connection   *d_logging_connection;  // Connection to use for logging.
};

//-----------------------------------------------------------
//************** Client code uses the following *************

// Type of a client routine to request new logging and to handle a
// report of changed logging.  This callback is called when the
// logging server reports a new set of files, which should happen
// after each request is made.

typedef	struct _vrpn_AUXLOGGERCB {
  struct timeval	msg_time;	// Timestamp of new logging
  const char *local_in_logfile_name;    // Name of the incoming local log ("" if none).
  const char *local_out_logfile_name;
  const char *remote_in_logfile_name;
  const char *remote_out_logfile_name;
} vrpn_AUXLOGGERCB;

typedef void (VRPN_CALLBACK *vrpn_AUXLOGGERREPORTHANDLER) (void * userdata,
					  const vrpn_AUXLOGGERCB info);


class VRPN_API vrpn_Auxiliary_Logger_Remote : public vrpn_Auxiliary_Logger {
public:
  vrpn_Auxiliary_Logger_Remote(const char * name, vrpn_Connection * c = NULL);

  // Send a request to the server asking it to log the following.  Each of these
  // is with respect to the connection that the auxiliary logger server is
  // handling, which may or may not be the one that it is connected to to
  // receive this message; it refers to the other side of the new connection
  // that the server establishes to do its logging.  Passing a NULL or empty
  // string ("") to any of the entries disables that log.
  // WARNING: If the server is set to connect to its own connection and log
  // it, then you must explicitly request a set of empty log files to stop
  // it logging the last time because otherwise it never gets the message
  // that it dropped the last connection and will continue logging after the
  // object is destroyed.
  bool send_logging_request(const char *local_in_logfile_name,
                            const char *local_out_logfile_name = "",
                            const char *remote_in_logfile_name = "",
                            const char *remote_out_logfile_name = "")
    { if (!d_connection) { return false; }
      return pack_log_message_of_type(request_logging_m_id,
        local_in_logfile_name, local_out_logfile_name,
        remote_in_logfile_name, remote_out_logfile_name);
    }

  bool send_logging_status_request( )
  {
	  if( !d_connection ) {  return false;  }
	  return pack_log_message_of_type( request_logging_status_m_id, NULL, NULL, NULL, NULL );
  }

  // Register/unregister a callback handler for the logging response.
  virtual int register_report_handler(void *userdata,
	  vrpn_AUXLOGGERREPORTHANDLER handler) {
    return d_callback_list.register_handler(userdata, handler);
  };
  virtual int unregister_report_handler(void *userdata,
	  vrpn_AUXLOGGERREPORTHANDLER handler) {
    return d_callback_list.unregister_handler(userdata, handler);
  }


  // This routine calls the mainloop of the connection it's on
  virtual void mainloop(void);

protected:
  // Static handler for the logging report message.
  // Use the base-class unpack method to convert the data into strings.
  vrpn_Callback_List<vrpn_AUXLOGGERCB> d_callback_list;

  static int VRPN_CALLBACK handle_report_message(void *userdata, vrpn_HANDLERPARAM p);
};

#endif
