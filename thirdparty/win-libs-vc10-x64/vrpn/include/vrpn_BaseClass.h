/** @file vrpn_BaseClass.h

  All types of client/server/peer objects in VRPN should be derived from the
  vrpn_BaseClass type described here.  This includes Tracker, Button, Analog,
  Clock, Dial, ForceDevice, Sound, and Text; it should include any user-defined
  objects as well.

  This class both implements code that will be shared by most (if not all)
  objects in the system and forms a skeleton for the definition of new objects
  by requiring certain virtual member functions to be defined.

  See the VRPN web pages or another simple type (such as vrpn_Analog) for an
  example of how to create a new VRPN object type using this as a base class.
*/

/*
Things to do to base objects to convert from VRPN version 4.XX to 5.00:
    In the header file:
	Include the BaseClass header
	Derive from the BaseClass
	Remove mainloop() pure virtual from the base class
	Remove connectionPtr from the base class
	Remove connection and my_id from the data objects
	Declare register_types()
    In the source file:
	Call the base-class constructor
	Call the base-class init() routine.
	Remove parts of the constructor
		Dealing with service and connection set up
		Dealing with registering the sender
		deleting the servicename
	Move the constructor code to register the types into a separate function
	Replace the message registration commands with calls to autodelete ones
	Delete the unregister commands for the message handlers
	Remove the connectionPtr function
	Remove the vrpn_get_connection_by_name clause from the remote
          constructor
	Change connection-> to d_connection->
	Change my_id to d_sender_id
	Remove the timeout parameter to all mainloop() functions
	Put a call to client_mainloop() in the Remote object mainloop() function
Things to do in the server object (server device) files to convert from 4.XX
  to 5.00:
	Replace the message registration commands with calls to autodelete ones
		(Note that the handler for update rate has been removed from
                the tracker class -- it should not have been there in the first place.
                This saves the derived class from having to unregister the old one before
                registering its	own).
	Delete the unregister commands for the message handlers
	Change connection-> to d_connection->
	Change my_id to d_sender_id
	Remove the timeout parameter to all mainloop() functions
	Put a call to server_mainloop() in each server mainloop()
*/

#ifndef VRPN_BASECLASS
#define VRPN_BASECLASS

#include "vrpn_Shared.h"

#include "vrpn_Connection.h"

const int vrpn_MAX_BCADRS =	100;
///< Internal value for number of BaseClass addresses

/// Since the sending of text messages has been pulled into the base class (so
/// that every object can send error/warning/info messages this way), these
/// definitions have been pulled in here as well.
typedef enum {vrpn_TEXT_NORMAL = 0, vrpn_TEXT_WARNING = 1, vrpn_TEXT_ERROR = 2} vrpn_TEXT_SEVERITY;
const unsigned	vrpn_MAX_TEXT_LEN = 1024;

class	VRPN_API vrpn_BaseClass;

/// Class that handles text/warning/error printing for all objects in the
/// system.
// It is a system class, with one instance of it in existence.  Each object in
// the system registers with this class when it is constructed.  By default,
// this class prints all Warning and Error messages to stdout, prefaced by
// "vrpn Warning(0) from MUMBLE: ", where the 0 indicates the level of the
// message and Warning the severity, and MUMBLE the
// name of the object that sent the message.
//  The user could create their own TextPrinter, and attach whatever objects
// they want to it.

class VRPN_API vrpn_TextPrinter {
  public:
    vrpn_TextPrinter();
    ~vrpn_TextPrinter();

    /// Adds an object to the list of watched objects (multiple registration
    /// of the same object will result in only one printing for each message
    /// from the object). Returns 0 on success and -1 on failure.
    int    add_object(vrpn_BaseClass *o);

    /// Remove an object from the list of watched objects (multiple deletions
    /// of the object will not cause any error condition; deletions of
    /// unregistered objects will not cause errors).
    void    remove_object(vrpn_BaseClass *o);

    /// Change the level of printing for the object (sets the minimum level to
    /// print). Default is Warnings and Errors of all levels.
    void    set_min_level_to_print(vrpn_TEXT_SEVERITY severity, vrpn_uint32 level = 0)
		{ d_severity_to_print = severity; d_level_to_print = level; };

    /// Change the ostream that will be used to print messages.  Setting a
    /// NULL ostream results in no printing.
    void   set_ostream_to_use(FILE *o) { d_ostream = o; };

  protected:
    /// Structure to hold the objects that are being watched.
    class VRPN_API vrpn_TextPrinter_Watch_Entry {
      public:
	  vrpn_BaseClass    *obj;		///< Object being watched
	  vrpn_TextPrinter  *me;
 		///< Pointer to this, because used in a static function
	  vrpn_TextPrinter_Watch_Entry	*next;
		///< Pointer to the next one in the list
    };
    vrpn_TextPrinter_Watch_Entry	*d_first_watched_object;   
		///< Head of list of objects being watched

    FILE		*d_ostream;		///< Output stream to use
    vrpn_TEXT_SEVERITY	d_severity_to_print;	///< Minimum severity to print
    vrpn_uint32		d_level_to_print;	///< Minimum level to print

    /// Handles the text messages that come from the connections for
    /// objects we are watching.
    static  int	VRPN_CALLBACK text_message_handler(void *userdata, vrpn_HANDLERPARAM p);
};
extern VRPN_API	vrpn_TextPrinter	vrpn_System_TextPrinter;

/// INTERNAL class to hold members that there should only be one copy of
/// even when a class inherits from multiple vrpn_BaseClasses because it
/// inherits from multiple user-level classes.  Note that not everything in
/// vrpnBaseClass should be here, because (for example) the registration of
/// types should be done for each parent class.
class VRPN_API vrpn_BaseClassUnique {
  friend class VRPN_API vrpn_TextPrinter;
  public:
	vrpn_BaseClassUnique();
	virtual ~vrpn_BaseClassUnique();

	bool shutup;	// if True, don't print the "No response from server" messages.

	friend class SendTextMessageBoundCall;
	class SendTextMessageBoundCall {
		private:
			vrpn_BaseClassUnique * _p;
			vrpn_TEXT_SEVERITY _severity;

		public:
			SendTextMessageBoundCall(vrpn_BaseClassUnique * device, vrpn_TEXT_SEVERITY type)
				: _p(device)
				, _severity(type)
				{}

			SendTextMessageBoundCall(SendTextMessageBoundCall const& other)
				: _p(other._p)
				, _severity(other._severity)
				{}

			int operator()(const char * msg) const {
				struct timeval timestamp;
				vrpn_gettimeofday(&timestamp, NULL);
				return _p->send_text_message(msg, timestamp, _severity);
			}
	};

  protected:
        vrpn_Connection *d_connection;  ///< Connection that this object talks to
        char *d_servicename;            ///< Name of this device, not including the connection part

        vrpn_int32 d_sender_id;		///< Sender ID registered with the connection
        vrpn_int32 d_text_message_id;	///< ID for text messages
        vrpn_int32 d_ping_message_id;	///< Ask the server if they are there
        vrpn_int32 d_pong_message_id;	///< Server telling that it is there

	/// Registers a handler with the connection, and remembers to delete at destruction.
	// This is a wrapper for the vrpn_Connection call that registers
	// message handlers.  It should be used rather than the connection's
	// function because this one will remember to unregister all of its handlers
	// at object deletion time.
	int register_autodeleted_handler(vrpn_int32 type,
		vrpn_MESSAGEHANDLER handler, void *userdata,
		vrpn_int32 sender = vrpn_ANY_SENDER);

	/// Encodes the body of the text message into a buffer, preparing for sending
	static	int encode_text_message_to_buffer(
		char *buf, vrpn_TEXT_SEVERITY severity, vrpn_uint32 level, const char *msg);

	/// Decodes the body of the text message from a buffer from the connection
	static	int decode_text_message_from_buffer(
		char *msg, vrpn_TEXT_SEVERITY *severity, vrpn_uint32 *level, const char *buf);

	/// Sends a NULL-terminated text message from the device d_sender_id
	int send_text_message(const char *msg, struct timeval timestamp,
		vrpn_TEXT_SEVERITY type = vrpn_TEXT_NORMAL, vrpn_uint32 level = 0);

	/// Returns an object you can stream into to send a text message from the device
	/// like send_text_message(vrpn_TEXT_WARNING) << "Value of i is: " << i;
	/// This use requires including vrpn_SendTextMessageStreamProxy.h
	SendTextMessageBoundCall send_text_message(vrpn_TEXT_SEVERITY type = vrpn_TEXT_NORMAL) {
		return SendTextMessageBoundCall(this, type);
	}

	/// Handles functions that all servers should provide in their mainloop() (ping/pong, for example)
	/// Should be called by all servers in their mainloop()
	void	server_mainloop(void);

	/// Handles functions that all clients should provide in their mainloop() (warning of no server, for example)
	/// Should be called by all clients in their mainloop()
	void	client_mainloop(void);

  private:
      struct {
	  vrpn_MESSAGEHANDLER	handler;
	  vrpn_int32		sender;
	  vrpn_int32		type;
	  void			*userdata;
      } d_handler_autodeletion_record[vrpn_MAX_BCADRS];
      int   d_num_autodeletions;

      int	d_first_mainloop;		///< First time client_mainloop() or server_mainloop() called?
      struct	timeval	d_time_first_ping;	///< When was the first ping of this unanswered group sent?
      struct	timeval	d_time_last_warned;	///< When is the last time we sent a warning?
      int	d_unanswered_ping;		///< Do we have an outstanding ping request?
      int	d_flatline;			///< Has it been 10+ seconds without a response?

      /// Used by client/server code to request/send "server is alive" (pong) message
      static	int VRPN_CALLBACK handle_ping(void *userdata, vrpn_HANDLERPARAM p);
      static	int VRPN_CALLBACK handle_pong(void *userdata, vrpn_HANDLERPARAM p);
      static	int VRPN_CALLBACK handle_connection_dropped(void *userdata, vrpn_HANDLERPARAM p);
      void	initiate_ping_cycle(void);
};

//---------------------------------------------------------------
/// Class from which all user-level (and other) classes that communicate
/// with vrpn_Connections should derive.

class VRPN_API vrpn_BaseClass : virtual public vrpn_BaseClassUnique {

  public:

	/// Names the device and assigns or opens connection,
        /// calls registration methods
	vrpn_BaseClass (const char * name, vrpn_Connection * c = NULL);

	virtual ~vrpn_BaseClass();

	/// Called once through each main loop iteration to handle updates.
	/// Remote object mainloop() should call client_mainloop() and
        /// then call d_connection->mainloop().
	/// Server object mainloop() should service the device and then
        /// call server_mainloop(), but should not normally call
        /// d_connection->mainloop().
	virtual void mainloop () = 0;

	/// Returns a pointer to the connection this object is using
        virtual	vrpn_Connection *connectionPtr() {return d_connection;};

  protected:

	/// Initialize things that the constructor can't. Returns 0 on
        /// success, -1 on failure.
	virtual int init(void);

	/// Register the sender for this device (by default, the name of the
        /// device). Return 0 on success, -1 on fail.
	virtual int register_senders(void);

	/// Register the types of messages this device sends/receives.
        /// Return 0 on success, -1 on fail.
	virtual int register_types(void) = 0;
};

// End of defined VRPN_BASECLASS for vrpn_BaseClass.h


/*
-----------------------------------------------------------------------------
Answer to the question:
   "Why is there both a UNIQUE and NON-UNIQUE base class?",
   or
   "Why can't everything from vrpn_BaseClass be moved into vrpn_BaseClassUnique?"

   The first reason is that removing vrpn_BaseClass would require the
   vrpn_BaseClassUnique constructor to take a name and connection object as
   parameters, which would cause some problems due to the way virtual base
   classes are implemented in C++.

   Any class that inherits from a virtual base (either directly or several
   generations removed) must provide an explicit call to the constructor
   of the virtual base.  This is done because the virtual base constructor
   is invoked from the very first class in the constructor chain.

   Take for example vrpn_Tng3, which inherits vrpn_Button and vrpn_Serial_Analog
   (and thus vrpn_Analog).  Creating a new instance of a vrpn_Tng3 object will call
   the constructors in this order:
       Tng3
       BaseClassUnique  (because it is a virtual base)
       Button
       BaseClass   (coming from Button)
       Serial_Analog
       Analog
       BaseClass   (coming from Analog)

   Right now, BaseClassUnique's constructor has no parameters.  So the
   Tng3 constructor does not have to explicitly invoke BaseClassUnique, although
   implicitly it will call BaseClassUnique's 0-parameter constructor before doing
   anything else.  But if BaseClass is eliminated, then BaseClassUnique's
   constructor must do the work of creating the connection and copying the
   service name.  So BassClassUnique's constructor must now take a couple
   parameters, which means that every class (including Tng3, Button, Analog,
   and Serial_Analog) would have to explicitly name the constructor for
   BaseClassUnique in the code and specify parameters for connection and
   service-name, even though only one such call to the BaseClassUnique's
   constructor would ever actually occur at runtime (that of Tng3 since it's
   located at the lowest level of the family tree; the rest of the calls
   would be ignored).  This would mean inserting
   "vrpn_BaseClassUnique(name,connection)" into the initializer section of
   every constructor in *every* class under the BaseClassUnique subtree.

   The second reason we have both a unique and non-unique base class is that
   the "register_types" virtual function must be called several times for
   multiply-inherited devices, with a different virtual target in each case.
   Presently, register_types() is called from vrpn_BaseClass::init().
   init() may be called multiple times using a different vftable entry for
   register_types() each time (e.g. for the Tng3 it will refer once to
   vrpn_Analog::register_types() and once to vrpn_Button::register_types()).
   Both init() and the pure-virtual declaration of register_types() are found
   in BaseClass.  Moving init() up into BaseClassUnique instead of BaseClass
   means that register_types() would have to move up as well.  And if
   register_types() is declared in the virtual base class, BaseClassUnique,
   it can only have one virtual target.

   So it might appear that vrpn_BaseClass has no data members and would
   therefore be easy to eliminate.  However it actually does have a data
   member: the vftable entry for "register_types".  And this data member
   *must* be duplicated in the case of multiply-inherited device because a
   single object will need several distinct virtual targets for
   "register_types".

   [Jeff Feasel  19 May 2005]
-----------------------------------------------------------------------------
*/


//---------------------------------------------------------------
// Within VRPN (and other libraries), it is wise to avoid using the
// Standard Template Library.  This is very annoying, but required
// by the fact that some systems have incompatible versions of STL.
// This caused problems with any program that uses the GHOST library
// (which had its own STL on Windows), and I've heard tell of problems
// with other systems as well.  On the other hand, nothing says that
// we can't have our OWN template types and use them.  This next type
// is used to handle callback lists within objects.  It is templated
// over the struct that is passed to the user callback.
// See vrpn_Button.h's usage for an example.

// Disables a warning that the class requires DLL linkage to be
// used by clients of classes that include one: The classes themselves
// have DLL linkage, the code below asks for (but apparently does not
// get) DLL linkage, and the DLL-linked test programs work when things
// are as they are.  Do not use this class outside of a derived class.
#ifdef	_WIN32
#pragma warning( disable : 4251 )
#endif
template<class CALLBACK_STRUCT> class VRPN_API vrpn_Callback_List {
public:
  typedef void (VRPN_CALLBACK *HANDLER_TYPE)(void *userdata, const CALLBACK_STRUCT info);

  /// This class requires deep copies.
  void operator =(const vrpn_Callback_List &from) {
    // Delete any existing elements in the list.
    CHANGELIST_ENTRY  *current, *next;
    current = d_change_list;
    while (current != NULL) {
      next = current->next;
      delete current;
      current = next;
    }

    // Copy all elements from the other list.  XXX Side effect, this inverts the order
    current = from.d_change_list;
    while (current != NULL) {
      register_handler(current->userdata, current->handler);
      current = current->next;
    }
  }

  /// Call this to add a handler to the list.
  int register_handler(void *userdata, HANDLER_TYPE handler) {
	CHANGELIST_ENTRY  *new_entry;

	// Ensure that the handler is non-NULL
	if (handler == NULL) {
		fprintf(stderr,"vrpn_Callback_List::register_handler(): NULL handler\n");
		return -1;
	}

	// Allocate and initialize the new entry
	if ( (new_entry = new CHANGELIST_ENTRY) == NULL) {
		fprintf(stderr,"vrpn_Callback_List::register_handler(): Out of memory\n");
		return -1;
	}
	new_entry->handler = handler;
	new_entry->userdata = userdata;

	// Add this handler to the chain at the beginning (don't check to see
	// if it is already there, since duplication is okay).
	new_entry->next = d_change_list;
	d_change_list = new_entry;

	return 0;
  };

  /// Call this to remove a handler from the list (if it exists)
  int unregister_handler(void *userdata, HANDLER_TYPE handler) {
	// The pointer at *snitch points to victim
	CHANGELIST_ENTRY	*victim, **snitch;

	// Find a handler with this registry in the list (any one will do,
	// since all duplicates are the same).
	snitch = &d_change_list;
	victim = *snitch;
	while ( (victim != NULL) &&
		( (victim->handler != handler) ||
		  (victim->userdata != userdata) )) {
	    snitch = &( (*snitch)->next );
	    victim = victim->next;
	}

	// Make sure we found one
	if (victim == NULL) {
		fprintf(stderr,
		   "vrpn_Callback_List::unregister_handler: No such handler\n");
		return -1;
	}

	// Remove the entry from the list
	*snitch = victim->next;
	delete victim;

	return 0;
  };

  /// This will pass the referenced parameter as a const to all the callbacks.
  void call_handlers(const CALLBACK_STRUCT &info) {
    CHANGELIST_ENTRY *handler = d_change_list;
    while (handler != NULL) {
      handler->handler(handler->userdata, info);
      handler = handler->next;
    }
  };

  /// The list starts out empty
  vrpn_Callback_List() : d_change_list(NULL) {};

  /// Clear the list upon destruction if it is not empty already
  ~vrpn_Callback_List() {
    while (d_change_list != NULL) {
      CHANGELIST_ENTRY *next = d_change_list->next;
      delete d_change_list;
      d_change_list = next;
    }
  };

protected:
  typedef struct vrpn_CBS {
	  void			*userdata;
	  HANDLER_TYPE		handler;
	  struct vrpn_CBS	*next;
  } CHANGELIST_ENTRY;
  CHANGELIST_ENTRY	*d_change_list;
};

#endif
