#ifndef	VRPN_IMAGER_STREAM_BUFFER_H
#define	VRPN_IMAGER_STREAM_BUFFER_H
#include  "vrpn_Auxiliary_Logger.h"
#include  "vrpn_Imager.h"

// This is a fairly complicated class that implements a multi-threaded
// full-rate logger and partial-rate forwarder for a vrpn_Imager_Server
// object.  It is intended to allow previewing of microscopy experiments
// at a subset of the camera's video rate while logging the video at
// full rate, possibly on a remote computer and possibly on the original
// video server computer.  Its architecture is described in the "Full-rate
// logging" section of the vrpn_Imager.html document in the VRPN web
// page.

//-------------------------------------------------------------------
// This is a helper class for the vrpn_Imager_Stream_Shared_State class
// below.  It keeps a linked list of vrpn_HANDLERPARAM types and the
// buffers to which they point.  The buffers need to be allocated by
// the one who inserts to this list and deleted by the one who pulls
// elements from this list.

class VRPN_API vrpn_Message_List {
public:
  vrpn_Message_List(void) { d_first = d_last = NULL; d_count = 0; };
  ~vrpn_Message_List(void) {
    while (d_first != NULL) {
      struct d_ELEMENT *next = d_first->next;
      delete d_first;
      d_first = next;
    }
  }

  // Give the length of the list.
  unsigned size(void) const { return d_count; }

  // Insert an element into the list.  Return false if fails.
  bool insert_back(const vrpn_HANDLERPARAM &p) {
    struct d_ELEMENT *el = new struct d_ELEMENT;
    if (el == NULL) {
      return false;
    }
    el->p = p;
    el->next = NULL;
    if (d_last != NULL) {
      d_last->next = el;
    }
    d_last = el;
    if (d_first == NULL) {
      d_first = el;
    }
    d_count++;
    return true;
  }

  // Remove an element from the list.  Return false if fails.
  bool retrieve_front(vrpn_HANDLERPARAM *p) {
    if (p == NULL) {
      return false;
    }
    if (size() == 0) {
      return false;
    }

    *p = d_first->p;
    if (d_last == d_first) {
      d_last = NULL;
    }
    struct d_ELEMENT *temp = d_first;
    d_first = d_first->next;
    delete temp;

    d_count--;
    return true;
  }

protected:
  struct d_ELEMENT {
    vrpn_HANDLERPARAM p;
    struct d_ELEMENT *next;
  };
  struct d_ELEMENT *d_first, *d_last;
  unsigned d_count;
};

//-------------------------------------------------------------------
// This is the data structure that is shared between the initial
// thread (which listens for client connections) and the non-blocking logging
// thread that sometimes exists to listen to the vrpn_Imager_Server.
// All of its methods must be thread-safe, so it creates a semaphore
// for access and uses it in all of the non-atomic methods.
// Note that some of the things in here are pointers to objects that
// are in the parent class, and they are here just to provide the
// semaphore protection.  The parent class should only access these
// things through this shared state object.

class VRPN_API vrpn_Imager_Stream_Shared_State {
public:
  vrpn_Imager_Stream_Shared_State() { init(); }

  // Reset the shared state to what it should be at
  // the time the logging thread is started.
  void init(void) {
    d_time_to_exit = false;
    d_description_updated = false;
    d_nRows = d_nCols = d_nDepth = d_nChannels = 0;
    d_new_log_request = false;
    d_request_lil = NULL;
    d_request_lol = NULL;
    d_request_ril = NULL;
    d_request_rol = NULL;
    d_new_log_result = false;
    d_result_lil = NULL;
    d_result_lol = NULL;
    d_result_ril = NULL;
    d_result_rol = NULL;
    d_new_throttle_request = false;
    d_throttle_count = -1;
    d_frames_in_queue = 0;
  }

  // Accessors for the "time to exit" flag; set by the initial thread and
  // read by the logging thread.
  bool time_to_exit(void) { d_sem.p(); bool ret = d_time_to_exit; d_sem.v(); return ret; }
  void time_to_exit(bool do_exit) { d_sem.p(); d_time_to_exit = do_exit; d_sem.v(); }

  // Accessors for the parameters stored based on the
  // imager server's reports.  Returns false if nothing has
  // been set since the last time it was read, true (and fills in
  // the values) if it has.  Channel buffer must be delete [] by
  // the one calling this function iff the function returns true.
  bool get_imager_description(vrpn_int32 &nRows, vrpn_int32 &nCols,
                              vrpn_int32 &nDepth, vrpn_int32 &nChannels,
                              const char **channelBuffer) {
    d_sem.p();
    bool ret = d_description_updated;
    if (d_description_updated) {
      nRows = d_nRows; nCols = d_nCols; nDepth = d_nDepth;
      nChannels = d_nChannels; *channelBuffer = d_channel_buffer;
    }
    d_description_updated = false;
    d_sem.v();
    return ret;
  }
  bool set_imager_description(vrpn_int32 nRows, vrpn_int32 nCols,
                              vrpn_int32 nDepth, vrpn_int32 nChannels,
                              const char *channelBuffer) {
    d_sem.p();
    d_nRows = nRows; d_nCols = nCols; d_nDepth = nDepth;
    d_nChannels = nChannels; d_channel_buffer = channelBuffer;
    d_description_updated = true;
    d_sem.v();
    return true;
  }


  // Accessors for the initial thread to pass new logfile names down to the
  // logging thread, which will cause it to initiate a changeover of logging
  // connections.  Space for the return strings will be allocated in these functions
  // and must be deleted by the logging thread ONLY IF the get function fills the
  // values in (it returns true if it does).
  // NOTE:  this is used to query BOTH the presence of new logfile names 
  // AS WELL AS the names themselves.  this function will only return values
  // if new logfile names have been requested since the last time this 
  // function was called.
  bool get_logfile_request(char **lil, char **lol, char **ril, char **rol) {
    d_sem.p();
    bool ret = d_new_log_request;
    if (d_new_log_request) {
      // Allocate space to return the names in the handles passed in.
      // Copy the values from our local storage to the return values.
      if ( (*lil = new char[strlen(d_request_lil)+1]) != NULL) {
        strcpy(*lil, d_request_lil);
      }
      if ( (*lol = new char[strlen(d_request_lol)+1]) != NULL) {
        strcpy(*lol, d_request_lol);
      }
      if ( (*ril = new char[strlen(d_request_ril)+1]) != NULL) {
        strcpy(*ril, d_request_ril);
      }
      if ( (*rol = new char[strlen(d_request_rol)+1]) != NULL) {
        strcpy(*rol, d_request_rol);
      }

      // Delete and NULL the local storage pointers.
      delete [] d_request_lil; d_request_lil = NULL;
      delete [] d_request_lol; d_request_lol = NULL;
      delete [] d_request_ril; d_request_ril = NULL;
      delete [] d_request_rol; d_request_rol = NULL;
    }
    d_new_log_request = false;
    d_sem.v();
    return ret;
  }


  void set_logfile_request(const char *lil, const char *lol, const char *ril, const char *rol) {
    d_sem.p();

	// delete file names, in case the logging thread hasn't had a chance to 
	//  honor the request yet.
	if( d_request_lil) {  delete [] d_request_lil; d_request_lil = NULL;  }
	if( d_request_lol) {  delete [] d_request_lol; d_request_lol = NULL;  }
	if( d_request_ril) {  delete [] d_request_ril; d_request_ril = NULL;  }
	if( d_request_rol) {  delete [] d_request_rol; d_request_rol = NULL;  }

    // Allocate space for each string and then copy into it.
	if( lil != NULL )
	{
		if ( (d_request_lil = new char[strlen(lil)+1]) != NULL) {
			strcpy(d_request_lil, lil);
		}
	}
	if( lol != NULL )
	{
		if ( (d_request_lol = new char[strlen(lol)+1]) != NULL) {
			strcpy(d_request_lol, lol);
		}
	}
	if( ril != NULL )
	{
		if ( (d_request_ril = new char[strlen(ril)+1]) != NULL) {
			strcpy(d_request_ril, ril);
		}
	}
	if( rol != NULL )
	{
		if ( (d_request_rol = new char[strlen(rol)+1]) != NULL) {
			strcpy(d_request_rol, rol);
		}
	}

    d_new_log_request = true;
    d_sem.v();
  }


  // Accessors for the logfile thread to pass new logfile names back up to the
  // initial thread, reporting a changeover of logging connections.  
  // Space for the return strings will be allocated in these functions
  // and must be deleted by the initial thread ONLY IF the get function fills the
  // values in (it returns true if it does).
  // NOTE:  this function is intended to query BOTH the logfile names AS WELL AS 
  // the change in logging status.  it ONLY returns filenames if logging has
  // changed since the last time this function was called).
  bool get_logfile_result(char **lil, char **lol, char **ril, char **rol) {
    d_sem.p();
    bool ret = d_new_log_result;
    if (d_new_log_result) {
		// Allocate space to return the names in the handles passed in.
		// Copy the values from our local storage to the return values.
		if( d_result_lil == NULL )  *lil = NULL;
		else
		{
			if ( (*lil = new char[strlen(d_result_lil)+1]) != NULL) {
				strcpy(*lil, d_result_lil);
			}
		}
		if( d_result_lol == NULL )  *lol = NULL;
		else
		{
			if ( (*lol = new char[strlen(d_result_lol)+1]) != NULL) {
				strcpy(*lol, d_result_lol);
			}
		}
		if( d_result_ril == NULL )  *ril = NULL;
		else
		{
			if ( (*ril = new char[strlen(d_result_ril)+1]) != NULL) {
				strcpy(*ril, d_result_ril);
			}
		}
		if( d_result_rol == NULL )  *rol = NULL;
		else
		{
			if ( (*rol = new char[strlen(d_result_rol)+1]) != NULL) {
				strcpy(*rol, d_result_rol);
			}
		}

      // do not Delete and NULL the local storage pointers.
	  // someone may request the filenames later.
    }
    d_new_log_result = false;
    d_sem.v();
    return ret;
  }


  void set_logfile_result(const char *lil, const char *lol, const char *ril, const char *rol) {
    d_sem.p();

	if( d_result_lil ) delete [] d_result_lil;  d_result_lil = NULL;
	if( d_result_lol ) delete [] d_result_lol;  d_result_lol = NULL;
	if( d_result_ril ) delete [] d_result_ril;  d_result_ril = NULL;
	if( d_result_rol ) delete [] d_result_rol;  d_result_rol = NULL;

    // Allocate space for each string and then copy into it.
	if( lil != NULL )
	{
		if ( (d_result_lil = new char[strlen(lil)+1]) != NULL) {
			strcpy(d_result_lil, lil);
		}
	}
	if( lol != NULL )
	{
		if ( (d_result_lol = new char[strlen(lol)+1]) != NULL) {
			strcpy(d_result_lol, lol);
		}
	}
	if( ril != NULL )
	{
		if ( (d_result_ril = new char[strlen(ril)+1]) != NULL) {
			strcpy(d_result_ril, ril);
		}
	}
	if( rol != NULL )
	{
		if ( (d_result_rol = new char[strlen(rol)+1]) != NULL) {
			strcpy(d_result_rol, rol);
		}
	}

    d_new_log_result = true;
    d_sem.v();
  }


  // fills in the arguments with the logfile names currently in use
  // for a particular log, the value will be NULL if that log is not being collected.
  // NOTE:  this function allocates memory for each string returned.  IT IS THE
  // RESPONSIBILITY OF THE CALLING FUNCTION TO FREE THIS MEMORY.
  void get_logfile_names( char** local_in, char** local_out, char** remote_in, char** remote_out )
  {
	  d_sem.p();
	  if( d_result_lil == NULL ) 
		  *local_in = NULL;
	  else
	  {
		  *local_in = new char[ strlen( d_result_lil ) + 1 ];
		  strcpy( *local_in, d_result_lil );
	  }
	  if( d_result_lol == NULL ) 
		  *local_out = NULL;
	  else
	  {
		  *local_out = new char[ strlen( d_result_lol ) + 1 ];
		  strcpy( *local_out, d_result_lol );
	  }
	  if( d_result_ril == NULL ) 
		  *remote_in = NULL;
	  else
	  {
		  *remote_in = new char[ strlen( d_result_ril ) + 1 ];
		  strcpy( *remote_in, d_result_ril );
	  }
	  if( d_result_rol == NULL ) 
		  *remote_out = NULL;
	  else
	  {
		  *remote_out = new char[ strlen( d_result_rol ) + 1 ];
		  strcpy( *remote_out, d_result_rol );
	  }
	  d_sem.v();
  }


  // Accessors for the initial thread to pass new throttle values down to the
  // logging thread, which will cause it to throttle as needed.
  bool get_throttle_request(vrpn_int32 *throttle_count) {
    d_sem.p();
    bool ret = d_new_throttle_request;
    if (d_new_throttle_request) {
      *throttle_count = d_throttle_count;
    }
    d_new_throttle_request = false;
    d_sem.v();
    return ret;
  }
  void set_throttle_request(vrpn_int32 throttle_count) {
    d_sem.p();
    d_throttle_count = throttle_count;
    d_new_throttle_request = true;
    d_sem.v();
  }

  // Accessors for the logging thread to increment and read the number of
  // frames in the queue and for the initial thread to decrement them.  The
  // increment/decrement is done when a begin_frame message is found.  The
  // increment/decrement routines return the new value.
  vrpn_int32 get_frames_in_queue(void) {
    d_sem.p();
    vrpn_int32 ret = d_frames_in_queue;
    d_sem.v();
    return ret;
  }
  vrpn_int32 increment_frames_in_queue(void) {
    d_sem.p();
    d_frames_in_queue++;
    vrpn_int32 ret = d_frames_in_queue;
    d_sem.v();
    return ret;
  }
  vrpn_int32 decrement_frames_in_queue(void) {
    d_sem.p();
    d_frames_in_queue--;
    vrpn_int32 ret = d_frames_in_queue;
    d_sem.v();
    return ret;
  }

  // Accessors for the logging thread to add messages to the queue
  // and for the initial thread to retrieve and count them.
  vrpn_int32 get_logger_to_client_queue_size(void) {
    d_sem.p();
    vrpn_int32 ret = d_logger_to_client_messages.size();
    d_sem.v();
    return ret;
  }
  bool insert_logger_to_client_message(const vrpn_HANDLERPARAM &p) {
    d_sem.p();
    bool ret = d_logger_to_client_messages.insert_back(p);
    d_sem.v();
    return ret;
  }
  bool retrieve_logger_to_client_message(vrpn_HANDLERPARAM *p) {
    d_sem.p();
    bool ret = d_logger_to_client_messages.retrieve_front(p);
    d_sem.v();
    return ret;
  }

protected:
  vrpn_Semaphore  d_sem;  // Semaphore to control access to data items.

  // Is it time for the logging thread to exit?
  bool  d_time_to_exit;

  // Stored copies of the value in the vrpn_Imager_Remote and a flag telling
  // whether they have changed since last read.
  bool  d_description_updated; // Do we have a new description from imager server?
  vrpn_int32  d_nRows;
  vrpn_int32  d_nCols;
  vrpn_int32  d_nDepth;
  vrpn_int32  d_nChannels;
  const char  *d_channel_buffer;  //< Allocated by sender, freed by receiver

  // Names of the log files passed from the initial thread to the logging
  // thread and a flag telling whether they have been changed since last
  // read.
  bool  d_new_log_request;
  char *d_request_lil;
  char *d_request_lol;
  char *d_request_ril;
  char *d_request_rol;

  // Names of the log files passed from the logging thread to the initial
  // thread and a flag telling whether they have been changed since last
  // read.  NOTE:  we maintain a copy of the log file names here, instead
  // of using the accessor of vrpn_Connection to query the names.  Only
  // the logging thread is supposed to have access to the logging connection,
  // but the logging thread is banned from calling methods on the client
  // connection.
  bool  d_new_log_result;
  char *d_result_lil;
  char *d_result_lol;
  char *d_result_ril;
  char *d_result_rol;

  // New throttle request passed on by the client-handling thread
  bool  d_new_throttle_request;
  vrpn_int32  d_throttle_count;

  // Records the number of frames in the queue.  This is incremented
  // by the non-blocking thread and decremented by the initial thread
  // as the begin_frame() messages are queued and dequeued.
  vrpn_int32  d_frames_in_queue;

  // List of messages passing from the logging thread to the initial
  // thread.
  vrpn_Message_List d_logger_to_client_messages;
};

//-------------------------------------------------------------------
// This class is a vrpn_Imager_Server; it has one or two instances of
// vrpn_Imager_Clients to talk to the server it is forwarding packets
// to.  It does not use their callback parsers, but rather hooks its own
// callbacks directly to the connection object for the server it is
// buffering.

class VRPN_API vrpn_Imager_Stream_Buffer :
               public vrpn_Auxiliary_Logger_Server,
               public vrpn_Imager_Server {
public:
  // Name of this object (the server side of the vrpn_Imager that is
  // buffered and the vrpn_Auxiliary_Logger that the client will connect to).
  // (Optional, can be NULL) pointer to the server connection on which to
  // communicate.
  // Name of the vrpn_Imager_Server to connect to (packets from this server will
  // be forwarded to the main connection, and logging will occur on the connection
  // to this imager_server).  This server may be local or remote; if local,
  // include "@localhost" in the name because new connections will be made to it.
  vrpn_Imager_Stream_Buffer(const char * name, const char * imager_server_name, vrpn_Connection * c);

  // Get rid of any logging thread and then clean up.
  virtual ~vrpn_Imager_Stream_Buffer();

  // Required for servers.
  virtual void mainloop(void);

protected:

  // Handle a logging-request message.  The request contains four file
  // names, two for local (to the Auxiliary server itself) and two for
  // remote (the far side of its connection to the server).  It must
  // also respond to the client with a message saying what logging has
  // been set up (using the send_logging_response function).  Logging is
  // turned off on a particular file by sending an empty-string name ("").
  // The in/out local/remote are with respect to the connection that the
  // logging is to occur on, which is to the imager server whose name is
  // passed in to the constructor, not the connection that the client has
  // sent the request on.
  // Make sure to send a response saying what you did.
  virtual void handle_request_logging(const char *local_in_logfile_name,
                                      const char *local_out_logfile_name,
                                      const char *remote_in_logfile_name,
                                      const char *remote_out_logfile_name);

  // Static portion of handling (unpacking) the request_logging message.  It
  // then calls the non-static virtual method above.
  static int VRPN_CALLBACK static_handle_request_logging(void *userdata, vrpn_HANDLERPARAM p );

  virtual void handle_request_logging_status( );


  // Handle dropped last connection on our primary connection by shutting down the
  // connection to the imager server.  The static method in the base class looks up this
  // pointer and calls the virtual method.
  virtual void handle_dropped_last_connection(void);

  // Handles a throttle request by passing it on down to the non-blocking
  // thread to deal with.
  static  int VRPN_CALLBACK static_handle_throttle_message(void *userdata, vrpn_HANDLERPARAM p);

  // Handle got first connection request by (having the second thread) create
  // a connection to the server and waiting until we get a description message
  // from the imager server we're listening to.  Timeout after a while if the
  // connection cannot be made or the server does not respond.
  virtual void handle_got_first_connection(void);
  vrpn_int32 got_first_connection_m_id;    // ID of message that we got the first connection
  static int VRPN_CALLBACK static_handle_got_first_connection(void *userdata, vrpn_HANDLERPARAM p );

  // State shared between the initial thread and the logging thread.
  vrpn_Imager_Stream_Shared_State d_shared_state;

  //----------------------------------------------------------------------
  // The section below includes methods and member variables that should
  // only be used by the logging thread.  They are not protected by
  // semaphores and so should not be accessed except within the
  // logging_thread_func().

  // This class spawns a new thread to handle uninterrupted communication
  // and logging with the vrpn_Imager_Server that we are forwarding messages
  // for.  This is created in the constructor and shut down (hopefully gently)
  // in the destructor.  There are a number of semaphores that are used by
  // the initial thread and the logging thread to communicate.
  vrpn_Thread   *d_logging_thread;

  // The function that is called to become the logging thread.  It is passed
  // a pointer to "this" so that it can acces the object that created it.
  // Note that it must use semaphores to get at the data that will be shared
  // between the main thread and itself.  The static function basically just
  // pulls the "this" pointer out and then calls the non-static function.
  static void static_logging_thread_func(vrpn_ThreadData &threadData);
  void logging_thread_func(void);

  // Stop the logging thread function, cleanly if possible.  Returns true if
  // the function stopped cleanly, false if it had to be killed.
  bool stop_logging_thread(void);

  // Name of the vrpn_Imager_Server object we are to connect to and
  // log/pass messages from.
  char *d_imager_server_name;

  // Are we ready to drop the old connection (new one has received its
  // descriptor message)?
  bool d_ready_to_drop_old_connection;

  // The connection that is used to talk to the client.
  vrpn_Connection *d_log_connection;
  vrpn_Connection *open_new_log_connection(
                  const char *local_in_logfile_name,
                  const char *local_out_logfile_name,
                  const char *remote_in_logfile_name,
                  const char *remote_out_logfile_name);

  // These will create/destroy the d_imager_remote and other callback handlers
  // needed to provide the handling of messages from the logging connection
  // passed in; they are used by the initial-connection code and by the
  // code that handles handing off from an old connection to a new connection
  // when a new logging message is received.
  bool setup_handlers_for_logging_connection(vrpn_Connection *c);
  bool teardown_handlers_for_logging_connection(vrpn_Connection *c);

  // This is yet another "create me some logs" function; it handles the
  // hand-off from one log file to another within the logging thread.
  // It is called by the main logging thread function when a request comes in
  // from the initial thread to perform logging.
  bool make_new_logging_connection(const char *local_in_logfile_name,
                                    const char *local_out_logfile_name,
                                    const char *remote_in_logfile_name,
                                    const char *remote_out_logfile_name);

  // The imager remote to listen to the vrpn_Imager_Server, along
  // with the callback functions that support its operation.  The
  // generic VRPN callback handler that receives all messages from
  // the imager server and either queues them or handles them, there
  // is a static version that just gets a this pointer and calls the
  // non-static function.
  vrpn_Imager_Remote *d_imager_remote;
  static void VRPN_CALLBACK handle_image_description(void *pvISB, const struct timeval msg_time);
  static int VRPN_CALLBACK static_handle_server_messages(void *pvISB, vrpn_HANDLERPARAM p);
  int handle_server_messages(const vrpn_HANDLERPARAM &p);

  // Types of messages we expect to be coming from the server.
  vrpn_int32	d_server_description_m_id;	//< ID of the message type describing the range and channels
  vrpn_int32	d_server_begin_frame_m_id;	//< ID of the message type describing the start of a region
  vrpn_int32	d_server_end_frame_m_id;	//< ID of the message type describing the start of a region
  vrpn_int32	d_server_discarded_frames_m_id; //< ID of the message type describing the discarding of one or more regions
  vrpn_int32	d_server_regionu8_m_id;	        //< ID of the message type describing a region with 8-bit unsigned entries
  vrpn_int32	d_server_regionu12in16_m_id;    //< ID of the message type describing a region with 12-bit unsigned entries packed in 16 bits
  vrpn_int32	d_server_regionu16_m_id;	//< ID of the message type describing a region with 16-bit unsigned entries
  vrpn_int32	d_server_regionf32_m_id;	//< ID of the message type describing a region with 32-bit float entries
  vrpn_int32	d_server_text_m_id;             //< ID of the system text message
  vrpn_int32	d_server_ping_m_id;             //< ID of the system ping message
  vrpn_int32	d_server_pong_m_id;             //< ID of the system pong message

  // Transcode the sender and type fields from the logging server connection to
  // the initial client connection and pack the resulting message into the queue
  // from the logging thread to the initial thread.  The data buffer is copied;
  // this space is allocated by the logging thread and must be freed by the initial thread.
  // Returns true on success and false on failure.  The sender is set to the
  // d_sender_id of our server object.
  bool transcode_and_send(const vrpn_HANDLERPARAM &p);

  // Transcode the type from the logging thread's connection type to
  // the initial thread's connection type.  Return -1 if we don't
  // recognize the type.
  vrpn_int32 transcode_type(vrpn_int32 type);

  // Handling throttling on the non-blocking thread.  This is a shadow
  // copy of the structures in the vrpn_Image_Server base class; we cannot
  // use those directly because they will be adjusted by their own callbacks
  // in the initial thread.
  vrpn_uint16 d_server_dropped_due_to_throttle;
  vrpn_int32  d_server_frames_to_send;
};

//-----------------------------------------------------------
// Client code should connect to the server twice, once as
// a vrpn_Imager_Server and once as a vrpn_Auxiliary_Logger_Server.
// There is not a special remote class for this.

#endif
