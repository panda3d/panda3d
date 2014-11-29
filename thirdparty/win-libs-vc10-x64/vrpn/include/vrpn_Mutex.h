#ifndef VRPN_MUTEX_H
#define VRPN_MUTEX_H


#include "vrpn_Shared.h"  // for timeval and timeval manipulation functions
#include "vrpn_Connection.h"  // for vrpn_HANDLERPARAM


// Every time a Mutex_Remote connects to a Mutex_Server, the server assigns
// a unique ID to the remote.
// HACK - because vrpn doesn't let us unicast within a multicast (multiple-
// connection server) (in any clean way), or identify at a MC server which
// connection a message came in over, this code is fragile - it depends
// on the fact that vrpn_Connection only allows one connection to be made
// before triggering the got_connection callback.  If connections were somehow
// batched, or we multithreaded vrpn_Connection, this would break.

class VRPN_API vrpn_Mutex {

  public:

    vrpn_Mutex (const char * name, vrpn_Connection * = NULL);
    virtual ~vrpn_Mutex (void) = 0;

    void mainloop (void);

  protected:

    vrpn_Connection * d_connection;

    vrpn_int32 d_myId;
    vrpn_int32 d_requestIndex_type;
    vrpn_int32 d_requestMutex_type;
    vrpn_int32 d_release_type;
    vrpn_int32 d_releaseNotification_type;
    vrpn_int32 d_grantRequest_type;
    vrpn_int32 d_denyRequest_type;
    vrpn_int32 d_initialize_type;

    void sendRequest (vrpn_int32 index);
    void sendRelease (void);
    void sendReleaseNotification (void);
    void sendGrantRequest (vrpn_int32 index);
    void sendDenyRequest (vrpn_int32 index);

};

class VRPN_API vrpn_Mutex_Server : public vrpn_Mutex {

  public:

    vrpn_Mutex_Server (const char * name, vrpn_Connection * = NULL);
    virtual ~vrpn_Mutex_Server (void);

  protected:

    enum state { HELD, FREE };

    state d_state;

    vrpn_int32 d_remoteIndex;
      ///< Counts remotes who have had IDs issued to them.

    static int VRPN_CALLBACK handle_requestIndex (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_requestMutex (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_release (void *, vrpn_HANDLERPARAM);

    static int VRPN_CALLBACK handle_gotConnection (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_dropLastConnection (void *, vrpn_HANDLERPARAM);
};

class VRPN_API vrpn_Mutex_Remote : public vrpn_Mutex {

  public:

    vrpn_Mutex_Remote (const char * name, vrpn_Connection * = NULL);
    virtual ~vrpn_Mutex_Remote (void);


    // ACCESSORS


    vrpn_bool isAvailable (void) const;
      ///< True from when release() is called or we receive a release
      ///< message from another process until request() is called or we
      ///< grant the lock to another process in response to its request
      ///< message.
    vrpn_bool isHeldLocally (void) const;
      ///< True from when RequestGranted callbacks are triggered until
      ///< release() is called.
    vrpn_bool isHeldRemotely (void) const;
      ///< True from when we grant the lock to another process in response
      ///< to its request message until we receive a release message from
      ///< another process.


    // MANIPULATORS


    void request (void);
      ///< Request the distributed lock.  Does not request the lock
      ///< if !isAvailable(), instead automatically triggering DeniedCallbacks.

    void release (void);
      ///< Release the distributed lock.  Does nothing if !isHeldLocally()
      ///< and there isn't a request pending.

    void addRequestGrantedCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when OUR request is granted.
    void addRequestDeniedCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when OUR request is denied.
    void addTakeCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when ANY peer gets the mutex.
    void addReleaseCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when ANY peer releases the
      ///< mutex.

  protected:

    void requestIndex (void);

    enum state { OURS, REQUESTING, AVAILABLE, HELD_REMOTELY};

    state d_state;
    vrpn_int32 d_myIndex;
    vrpn_bool d_requestBeforeInit;

    static int VRPN_CALLBACK handle_grantRequest (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_denyRequest (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_releaseNotification (void *, vrpn_HANDLERPARAM);

    static int VRPN_CALLBACK handle_initialize (void *, vrpn_HANDLERPARAM);

    static int VRPN_CALLBACK handle_gotConnection (void *, vrpn_HANDLERPARAM);

    void triggerGrantCallbacks (void);
    void triggerDenyCallbacks (void);
    void triggerTakeCallbacks (void);
    void triggerReleaseCallbacks (void);

    struct mutexCallback {
      int (* f) (void *);
      void * userdata;
      mutexCallback * next;
    };

    mutexCallback * d_reqGrantedCB;
    mutexCallback * d_reqDeniedCB;
    mutexCallback * d_takeCB;
    mutexCallback * d_releaseCB;
};






/// vrpn_PeerMutex
///
///   This class provides distributed mutual exclusion between every instance
/// with the same name for which addPeer() has been called.
///   If a process calls request() when isAvailable() returns true,
/// the mutex will attempt to secure a lock to whatever resource it is
/// governing;  either RequestGranted or RequestDenied callbacks will
/// be triggered.  If RequestGranted callbacks are triggered, the process
/// has the lock until it explicitly calls release() (and can verify this
/// by checking isHeldLocally()).  Once the lock-owner calls release(),
/// Release callbacks at every peer will be triggered.
///
///   Like most vrpn classes, the mainloop() must be called frequently.
///
///   Note that none of isAvailable(), isHeldLocally(), and isHeldRemotely()
/// are true between when request() is called and either RequestGranted or
/// RequestDenied callbacks are triggered.

// Known bugs -

//   The constructor that takes a Connection as an argument will incorrectly
// identify its IP address as the machine's default rather than the address
// used by the Connection.  This should not cause any errors in the protocol,
// but will bias the tiebreaking algorithm.  The same constructor will use
// the wrong port number;  without this information the tiebreaking algorithm
// fails.  Oops.  Use only one mutex per Connection for now.

// Possible bugs -

//   If on startup somebody else is holding the mutex we'll think it's
// available.  However, if we request it they'll deny it to us and
// we won't break.
//   If sites don't execute the same set of addPeer() commands, they may
// implicitly partition the network and not get true mutual exclusion.
// This could be fixed by sending an addPeer message.
//   If sites execute addPeer() while the lock is held, or being requested,
// we'll break.
//   - To fix:  send messages, but defer all executions of addPeer until the
// lock is released.  If we want to be really careful here, on getting an
// addPeer message when we think the lock is available we should request
// the lock and then (if we get it) release it immediately, without
// triggering any user callbacks.  Sounds tough to code?

// Handling more than 2 sites in a mutex requires multiconnection servers.
// It's been tested with 1-3 sites, and works fine.

// This is an O(n^2) network traffic implementation;
// for details (and how to fix if it ever becomes a problem),
// see the implementation notes in vrpn_Mutex.C.



class VRPN_API vrpn_PeerMutex {

  public:

    vrpn_PeerMutex (const char * name, int port,
                    const char * NICaddress = NULL);
      ///< This constructor opens a new connection/port for the mutex.

    ~vrpn_PeerMutex (void);
      ///< If isHeldLocally(), calls release().


    // ACCESSORS


    vrpn_bool isAvailable (void) const;
      ///< True from when release() is called or we receive a release
      ///< message from another process until request() is called or we
      ///< grant the lock to another process in response to its request
      ///< message.
    vrpn_bool isHeldLocally (void) const;
      ///< True from when RequestGranted callbacks are triggered until
      ///< release() is called.
    vrpn_bool isHeldRemotely (void) const;
      ///< True from when we grant the lock to another process in response
      ///< to its request message until we receive a release message from
      ///< another process.

    int numPeers (void) const;


    // MANIPULATORS


    void mainloop (void);

    void request (void);
      ///< Request the distributed lock.  Does not request the lock
      ///< if !isAvailable(), instead automatically triggering DeniedCallbacks.

    void release (void);
      ///< Release the distributed lock.  Does nothing if !isHeldLocally()
      ///< and there isn't a request pending.


    void addPeer (const char * stationName);
      ///< Takes a VRPN station name of the form "<host>:<port>".


    void addRequestGrantedCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when OUR request is granted.
    void addRequestDeniedCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when OUR request is denied.
    void addTakeCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when ANY peer gets the mutex.
      ///< (If several peers are competing for the mutex, and the
      ///<  implementation issues multiple "grants", these callbacks will
      ///<  only be triggered once between triggerings of ReleaseCallbacks.)
    void addReleaseCallback (void * userdata, int (*) (void *));
      ///< These callbacks are triggered when ANY peer releases the
      ///< mutex.



  protected:


    enum state { OURS, REQUESTING, AVAILABLE, HELD_REMOTELY};

    char * d_mutexName;

    state d_state;

    int d_numPeersGrantingLock;
      ///< Counts the number of "grants" we've received after issuing
      ///< a request;  when this reaches d_numPeers, the lock is ours.

    vrpn_Connection * d_server;
      ///< Receive on this connection.
    vrpn_Connection ** d_peer;
      ///< Send on these connections to other Mutex's well-known-ports.

    int d_numPeers;
    int d_numConnectionsAllocated;
      ///< Dynamic array size for d_peer and d_peerGrantedLock.

    vrpn_uint32 d_myIP;
    vrpn_uint32 d_myPort;
    vrpn_uint32 d_holderIP;
    vrpn_int32 d_holderPort;

    vrpn_int32 d_myId;
    vrpn_int32 d_request_type;
    vrpn_int32 d_release_type;
    vrpn_int32 d_grantRequest_type;
    vrpn_int32 d_denyRequest_type;
    //vrpn_int32 d_losePeer_type;

    static int VRPN_CALLBACK handle_request (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_release (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_grantRequest (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_denyRequest (void *, vrpn_HANDLERPARAM);

    static int VRPN_CALLBACK handle_losePeer (void *, vrpn_HANDLERPARAM);

    void sendRequest (vrpn_Connection *);
    void sendRelease (vrpn_Connection *);
    void sendGrantRequest (vrpn_Connection *, vrpn_uint32 IPnumber,
                           vrpn_uint32 PortNumber);
    void sendDenyRequest (vrpn_Connection *, vrpn_uint32 IPnumber,
                          vrpn_uint32 PortNumber);

    void triggerGrantCallbacks (void);
    void triggerDenyCallbacks (void);
    void triggerTakeCallbacks (void);
    void triggerReleaseCallbacks (void);

    void checkGrantMutex (void);

    void init (const char * name);

    struct mutexCallback {
      int (* f) (void *);
      void * userdata;
      mutexCallback * next;
    };

    mutexCallback * d_reqGrantedCB;
    mutexCallback * d_reqDeniedCB;
    mutexCallback * d_takeCB;
    mutexCallback * d_releaseCB;

    struct peerData {
      vrpn_uint32 IPaddress;
      vrpn_uint32 port;
      vrpn_bool grantedLock;
    };

    peerData * d_peerData;
      ///< Needed only to clean up when a peer shuts down (mid-request).
      ///< It isn't currently feasible to have all this data, so instead
      ///< we abort requests that were interrupted by a shutdown.

    vrpn_PeerMutex (const char * name, vrpn_Connection * c);
      ///< This constructor reuses a SERVER connection for the mutex.
      ///< BUG BUG BUG - do not use this constructor;  it does not reliably
      ///< resolve race conditions.
};


#endif  // VRPN_MUTEX_H


