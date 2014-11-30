#ifndef VRPN_SHARED_OBJECT
#define VRPN_SHARED_OBJECT

#include "vrpn_Shared.h"  // for types
  // This *must* be here to take care of winsock.h and sys/time.h and other
  // assorted system-dependent details.

#include "vrpn_Connection.h"  // for vrpn_HANDLERPARAM

class VRPN_API vrpn_LamportClock;  // from "vrpn_LamportClock.h"
class VRPN_API vrpn_LamportTimestamp;

// It's increasingly clear that we could handle all this with
// a template, except for the fact that vrpn_Shared_String is
// based on char *.  All we need is a String base class.
// We could try to adopt BCString from nano's libnmb...

// I'd like to implement shouldAcceptUpdate/shouldSendUpdate
// with the Strategy pattern (Gamma/Helm/Johnson/Vlissides 1995, pg 315).
// That would make it far, far easier to extend, but the implementation
// looks too unweildy.

class VRPN_API vrpn_Shared_int32;
class VRPN_API vrpn_Shared_float64;
class VRPN_API vrpn_Shared_String;

typedef int (VRPN_CALLBACK * vrpnDeferredUpdateCallback) (void * userdata);

typedef int (VRPN_CALLBACK * vrpnSharedIntCallback) (void * userdata, vrpn_int32 newValue,
                                       vrpn_bool isLocal);
typedef int (VRPN_CALLBACK * vrpnSharedFloatCallback) (void * userdata,
                                         vrpn_float64 newValue,
                                       vrpn_bool isLocal);
typedef int (VRPN_CALLBACK * vrpnSharedStringCallback)
                    (void * userdata, const char * newValue,
                                       vrpn_bool isLocal);

typedef int (VRPN_CALLBACK * vrpnTimedSharedIntCallback)
                    (void * userdata, vrpn_int32 newValue,
                     timeval when, vrpn_bool isLocal);
typedef int (VRPN_CALLBACK * vrpnTimedSharedFloatCallback)
                    (void * userdata, vrpn_float64 newValue,
                     timeval when, vrpn_bool isLocal);
typedef int (VRPN_CALLBACK * vrpnTimedSharedStringCallback)
                    (void * userdata, const char * newValue,
                     timeval when, vrpn_bool isLocal);

// Update callbacks should return 0 on successful completion,
// nonzero on error (which will prevent further update callbacks
// from being invoked).

typedef int (VRPN_CALLBACK * vrpnSharedIntSerializerPolicy)
                    (void * userdata, vrpn_int32 newValue,
                     timeval when, vrpn_Shared_int32 * object);
typedef int (VRPN_CALLBACK * vrpnSharedFloatSerializerPolicy)
                    (void * userdata, vrpn_float64 newValue,
                     timeval when, vrpn_Shared_float64 * object);
typedef int (VRPN_CALLBACK * vrpnSharedStringSerializerPolicy)
                    (void * userdata, const char * newValue,
                     timeval when, vrpn_Shared_String * object);

// Policy callbacks should return 0 if the update should be accepted,
// nonzero if it should be denied.

#define VRPN_SO_DEFAULT 0x00
#define VRPN_SO_IGNORE_IDEMPOTENT 0x01
#define VRPN_SO_DEFER_UPDATES 0x10
#define VRPN_SO_IGNORE_OLD 0x100

// Each of these flags can be passed to all vrpn_Shared_* constructors.
// If VRPN_SO_IGNORE_IDEMPOTENT is used, calls of operator = (v) or set(v)
// are *ignored* if v == d_value.  No callbacks are called, no network
// traffic takes place.
// If VRPN_SO_DEFER_UPDATES is used, calls of operator = (v) or set(v)
// on vrpn_Shared_*_Remote are sent to the server but not reflected
// locally until an update message is received from the server.
// If VRPN_SO_IGNORE_OLD is set, calls of set(v, t) are ignored if
// t < d_lastUpdate.  This includes messages propagated over the network.

// A vrpn_Shared_*_Server/Remote pair using VRPN_SO_IGNORE_OLD are
// guaranteed to reach the same final state - after quiescence (all messages
// sent on the network are delivered) they will yield the same value(),
// but they are *not* guaranteed to go through the same sequence of
// callbacks.

// Using VRPN_SO_DEFER_UPDATES serializes all changes to d_value and
// all callbacks, so it guarantees that all instances of the shared
// variable see the same sequence of callbacks.

// setSerializerPolicy() can be used to change the way VRPN_SO_DEFER_UPDATES
// operates.  The default value described above is equivalent to calling
// setSerializerPolicy(vrpn_ACCEPT).  Also possible are vrpn_DENY_REMOTE,
// which causes the serializer to ignore all updates from its peers,
// vrpn_DENY_LOCAL, which accepts updates from peers but ignores local
// updates,
// and vrpn_CALLBACK, which passes the update to a callback which can
// return zero for vrpn_ACCEPT or nonzero for vrpn_DENY. 

enum vrpn_SerializerPolicy { vrpn_ACCEPT, vrpn_DENY_REMOTE,
                             vrpn_DENY_LOCAL, vrpn_CALLBACK };


// Separated out vrpn_SharedObject from common behavior of 3 classes
// on 14 Feb 2000.  Now all we need is permission to use templates to
// collapse them all together;  *all* the functions remaining on the
// other classes are type-dependent and should be templatable.
// (One exception:  the string that names the type.  This could probably
// be cut.)

class VRPN_API vrpn_SharedObject {

  public:

    vrpn_SharedObject (const char * name, const char * tname,
                       vrpn_int32 mode);
    virtual ~vrpn_SharedObject (void);

    // ACCESSORS

    const char * name (void) const;
    vrpn_bool isSerializer (void) const;

    // MANIPULATORS

    virtual void bindConnection (vrpn_Connection *);
      ///< Every derived class should call this, do what it needs to,
      ///< and ALSO call {server,remote}PostBindCleanup() to get
      ///< myId and peerId set up and to get standard handlers registered.


    void useLamportClock (vrpn_LamportClock *);
      ///< Lamport Clocks are NOT currently integrated.  They should
      ///< provide serialization (virtual timestamps) that work even
      ///< when the clocks of the computers communicating are not
      ///< roughly synchronized.

    void becomeSerializer (void);
      ///< Requests that this instance of the shared object becomes 
      ///< the serializer (i.e. lock-arbitrator), and we can then use
      ///< setSerializerPolicy to imitate a complete lock.  Does nothing
      ///< if we already are the serializer (isSerializer() returns true);
      ///< otherwise initiates a 3-phase request protocol with the
      ///< current serializer.  There currently isn't any provision for
      ///< notification of success (or failure).

    void registerDeferredUpdateCallback (vrpnDeferredUpdateCallback,
                                         void * userdata);
      ///< The specified function will be passed userdata when this
      ///< particular shared object defers an update (receives a local
      ///< update but is not the serializer and so sends the update off
      ///< to the serializer).  Intended to allow insertion of timing
      ///< code for those times when you really want to know how long
      ///< every little thing is taking.

  protected:

    char * d_name;
    vrpn_int32 d_mode;
    timeval d_lastUpdate;
    char * d_typename;  // currently int32, float64, or String

    vrpn_Connection * d_connection;
    //vrpn_int32 d_updateFromServer_type;
    //vrpn_int32 d_updateFromRemote_type;
    //vrpn_int32 d_myUpdate_type;  // fragile
    vrpn_int32 d_serverId;
    vrpn_int32 d_remoteId;
    vrpn_int32 d_myId;  // fragile
    vrpn_int32 d_peerId;  // fragile
    vrpn_int32 d_update_type;

    vrpn_int32 d_requestSerializer_type;
      ///< Sent to the serializer to assume its duties.
    vrpn_int32 d_grantSerializer_type;
      ///< Sent by the serializer to grant a request.
    vrpn_int32 d_assumeSerializer_type;
      ///< Sent by a new serializer once it has been notified that
      ///< its request has been granted.

    //vrpn_int32 d_updateFromServerLamport_type;
    //vrpn_int32 d_updateFromRemoteLamport_type;
    vrpn_int32 d_lamportUpdate_type;

    vrpn_bool d_isSerializer;
      ///< default to vrpn_TRUE for servers, FALSE for remotes
    vrpn_bool d_isNegotiatingSerializer;
      ///< As long as we have inorder delivery, this should be
      ///< sufficient to keep us from getting many at once.

    virtual vrpn_bool shouldSendUpdate (vrpn_bool isLocalSet,
                                        vrpn_bool acceptedUpdate);

    int yankCallbacks (vrpn_bool isLocal);
      ///< must set d_lastUpdate BEFORE calling yankCallbacks()
        
    static int VRPN_CALLBACK handle_requestSerializer (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_grantSerializer (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_assumeSerializer (void *, vrpn_HANDLERPARAM);

    vrpn_bool d_queueSets;
      ///< If this is true, no set()s are processed;  instead, they
      ///< are queued for later execution.
      ///< NOT IMPLEMENTED

    vrpn_LamportClock * d_lClock;
    vrpn_LamportTimestamp * d_lastLamportUpdate;


    struct deferredUpdateCallbackEntry {
      vrpnDeferredUpdateCallback handler;
      void * userdata;
      deferredUpdateCallbackEntry * next;
    };
    deferredUpdateCallbackEntry * d_deferredUpdateCallbacks;

    int yankDeferredUpdateCallbacks (void);
      ///< returns -1 on error (i.e. nonzero return by a callback)

    void serverPostBindCleanup (void);
    void remotePostBindCleanup (void);

    virtual void sendUpdate (void) = 0;
      ///< Should invoke default sendUpdate() for this derived type.
    virtual int handleUpdate (vrpn_HANDLERPARAM) = 0;

    static int VRPN_CALLBACK handle_gotConnection (void *, vrpn_HANDLERPARAM);
      ///< Register this handler in postBindCleanup();
      ///< it calls sendUpdate() to make sure the remote has the
      ///< correct value on first connection.
    static int VRPN_CALLBACK handle_update (void *, vrpn_HANDLERPARAM);
      ///< Passes arguments to handleUpdate() for this type;
      ///< registered in postBindCleanup();

  private:

    void postBindCleanup (void);
};



class VRPN_API vrpn_Shared_int32 : public vrpn_SharedObject {


  public:

    vrpn_Shared_int32 (const char * name,
                       vrpn_int32 defaultValue = 0,
                       vrpn_int32 mode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_int32 (void);

    // ACCESSORS

    vrpn_int32 value (void) const;
    operator vrpn_int32 () const;

    // MANIPULATORS

    vrpn_Shared_int32 & operator = (vrpn_int32 newValue);
      // calls set(newValue, now);

    vrpn_Shared_int32 & set (vrpn_int32 newValue, timeval when);
      // calls protected set (newValue, when, vrpn_TRUE);

    void register_handler (vrpnSharedIntCallback, void *);
    void unregister_handler (vrpnSharedIntCallback, void *);
    void register_handler (vrpnTimedSharedIntCallback, void *);
    void unregister_handler (vrpnTimedSharedIntCallback, void *);
      // Callbacks are (currently) called *AFTER* the assignment
      // has been made, so any check of the value of their shared int
      // will return newValue

    void setSerializerPolicy (vrpn_SerializerPolicy policy = vrpn_ACCEPT,
                              vrpnSharedIntSerializerPolicy f = NULL,
                              void * userdata = NULL);
  protected:

    vrpn_int32 d_value;

    // callback code
    // Could generalize this by making a class that gets passed
    // a vrpn_HANDLERPARAM and passes whatever is needed to its callback,
    // but it's not worth doing that unless we need a third or fourth
    // kind of callback.
    struct callbackEntry {
      vrpnSharedIntCallback handler;
      void * userdata;
      callbackEntry * next;
    };
    callbackEntry * d_callbacks;
    struct timedCallbackEntry {
      vrpnTimedSharedIntCallback handler;
      void * userdata;
      timedCallbackEntry * next;
    };
    timedCallbackEntry * d_timedCallbacks;

    vrpn_Shared_int32 & set (vrpn_int32, timeval,
                             vrpn_bool isLocalSet, 
                             vrpn_LamportTimestamp * = NULL);

    virtual vrpn_bool shouldAcceptUpdate (vrpn_int32 newValue, timeval when,
                                          vrpn_bool isLocalSet, 
                                          vrpn_LamportTimestamp *);

    virtual void sendUpdate (void);
    void sendUpdate (vrpn_int32 newValue, timeval when);

    void encode (char ** buffer, vrpn_int32 * len,
                 vrpn_int32 newValue, timeval when) const;
    void encodeLamport (char ** buffer, vrpn_int32 * len,
                 vrpn_int32 newValue, timeval when,
                 vrpn_LamportTimestamp * t) const;
      // We used to have sendUpdate() and encode() just read off of
      // d_value and d_lastUpdate, but that doesn't work when we're
      // serializing (VRPN_SO_DEFER_UPDATES), because we don't want
      // to change the local values but do want to send the new values
      // to the serializer.
    void decode (const char ** buffer, vrpn_int32 * len,
                 vrpn_int32 * newValue, timeval * when) const;
    void decodeLamport (const char ** buffer, vrpn_int32 * len,
                 vrpn_int32 * newValue, timeval * when,
                 vrpn_LamportTimestamp ** t) const;

    int yankCallbacks (vrpn_bool isLocal);
      // must set d_lastUpdate BEFORE calling yankCallbacks()

    // serializer policy code
    vrpn_SerializerPolicy d_policy;  // default to vrpn_ACCEPT
    vrpnSharedIntSerializerPolicy d_policyCallback;
    void * d_policyUserdata;
        
    int handleUpdate (vrpn_HANDLERPARAM);


    static int VRPN_CALLBACK handle_lamportUpdate (void *, vrpn_HANDLERPARAM);
};

// I don't think the derived classes should have to have operator = ()
// defined (they didn't in the last version??), but both SGI and HP
// compilers seem to insist on it.

class VRPN_API vrpn_Shared_int32_Server : public vrpn_Shared_int32 {

  public:

    vrpn_Shared_int32_Server (const char * name,
                       vrpn_int32 defaultValue = 0,
                       vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_int32_Server (void);

    vrpn_Shared_int32_Server & operator = (vrpn_int32 newValue);

    virtual void bindConnection (vrpn_Connection *);

  protected:


};

class VRPN_API vrpn_Shared_int32_Remote : public vrpn_Shared_int32 {

  public:

    vrpn_Shared_int32_Remote (const char * name,
                              vrpn_int32 defaultValue = 0,
                              vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_int32_Remote (void);

    vrpn_Shared_int32_Remote & operator = (vrpn_int32 newValue);

    virtual void bindConnection (vrpn_Connection *);

};



class VRPN_API vrpn_Shared_float64 : public vrpn_SharedObject {


  public:

    vrpn_Shared_float64 (const char * name,
                         vrpn_float64 defaultValue = 0.0,
                         vrpn_int32 mode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_float64 (void);

    // ACCESSORS

    vrpn_float64 value (void) const;
    operator vrpn_float64 () const;

    // MANIPULATORS

    vrpn_Shared_float64 & operator = (vrpn_float64 newValue);
      // calls set(newValue, now);

    virtual vrpn_Shared_float64 & set (vrpn_float64 newValue, timeval when);
      // calls protected set (newValue, when, vrpn_TRUE);

    void register_handler (vrpnSharedFloatCallback, void *);
    void unregister_handler (vrpnSharedFloatCallback, void *);
    void register_handler (vrpnTimedSharedFloatCallback, void *);
    void unregister_handler (vrpnTimedSharedFloatCallback, void *);
      // Callbacks are (currently) called *AFTER* the assignment
      // has been made, so any check of the value of their shared int
      // will return newValue

    void setSerializerPolicy (vrpn_SerializerPolicy policy = vrpn_ACCEPT,
                              vrpnSharedFloatSerializerPolicy f = NULL,
                              void * userdata = NULL);

  protected:

    vrpn_float64 d_value;

    // callback code
    // Could generalize this by making a class that gets passed
    // a vrpn_HANDLERPARAM and passes whatever is needed to its callback,
    // but it's not worth doing that unless we need a third or fourth
    // kind of callback.
    struct callbackEntry {
      vrpnSharedFloatCallback handler;
      void * userdata;
      callbackEntry * next;
    };
    callbackEntry * d_callbacks;
    struct timedCallbackEntry {
      vrpnTimedSharedFloatCallback handler;
      void * userdata;
      timedCallbackEntry * next;
    };
    timedCallbackEntry * d_timedCallbacks;

    vrpn_SerializerPolicy d_policy;  // default to vrpn_ACCEPT
    vrpnSharedFloatSerializerPolicy d_policyCallback;
    void * d_policyUserdata;

    vrpn_Shared_float64 & set (vrpn_float64, timeval, vrpn_bool isLocalSet);

    virtual vrpn_bool shouldAcceptUpdate (vrpn_float64 newValue, timeval when,
                                          vrpn_bool isLocalSet);

    virtual void sendUpdate (void);
    void sendUpdate (vrpn_float64 newValue,
                     timeval when);
    void encode (char ** buffer, vrpn_int32 * len, vrpn_float64 newValue,
                     timeval when) const;
    void decode (const char ** buffer, vrpn_int32 * len,
                 vrpn_float64 * newValue, timeval * when) const;

    int yankCallbacks (vrpn_bool isLocal);
      // must set d_lastUpdate BEFORE calling yankCallbacks()
        
    int handleUpdate (vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_lamportUpdate (void *, vrpn_HANDLERPARAM);
};

class VRPN_API vrpn_Shared_float64_Server : public vrpn_Shared_float64 {

  public:

    vrpn_Shared_float64_Server (const char * name,
                                vrpn_float64 defaultValue = 0,
                                vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_float64_Server (void);

    vrpn_Shared_float64_Server & operator = (vrpn_float64 newValue);

    virtual void bindConnection (vrpn_Connection *);

  protected:


};

class VRPN_API vrpn_Shared_float64_Remote : public vrpn_Shared_float64 {

  public:

    vrpn_Shared_float64_Remote (const char * name,
                                vrpn_float64 defaultValue = 0,
                                vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_float64_Remote (void);

    vrpn_Shared_float64_Remote & operator = (vrpn_float64 newValue);

    virtual void bindConnection (vrpn_Connection *);

};




class VRPN_API vrpn_Shared_String : public vrpn_SharedObject {


  public:

    vrpn_Shared_String (const char * name,
                         const char * defaultValue = NULL,
                         vrpn_int32 mode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_String (void);

    // ACCESSORS

    const char * value (void) const;
    operator const char * () const;

    // MANIPULATORS

    vrpn_Shared_String & operator = (const char * newValue);
      // calls set(newValue, now);

    virtual vrpn_Shared_String & set (const char * newValue, timeval when);
     // calls protected set (newValue, when, vrpn_TRUE);

    void register_handler (vrpnSharedStringCallback, void *);
    void unregister_handler (vrpnSharedStringCallback, void *);
    void register_handler (vrpnTimedSharedStringCallback, void *);
    void unregister_handler (vrpnTimedSharedStringCallback, void *);
      // Callbacks are (currently) called *AFTER* the assignment
      // has been made, so any check of the value of their shared int
      // will return newValue

    void setSerializerPolicy (vrpn_SerializerPolicy policy = vrpn_ACCEPT,
                              vrpnSharedStringSerializerPolicy f = NULL,
                              void * userdata = NULL);


  protected:

    char * d_value;

    // callback code
    // Could generalize this by making a class that gets passed
    // a vrpn_HANDLERPARAM and passes whatever is needed to its callback,
    // but it's not worth doing that unless we need a third or fourth
    // kind of callback.
    struct callbackEntry {
      vrpnSharedStringCallback handler;
      void * userdata;
      callbackEntry * next;
    };
    callbackEntry * d_callbacks;
    struct timedCallbackEntry {
      vrpnTimedSharedStringCallback handler;
      void * userdata;
      timedCallbackEntry * next;
    };
    timedCallbackEntry * d_timedCallbacks;

    vrpn_SerializerPolicy d_policy;  // default to vrpn_ACCEPT
    vrpnSharedStringSerializerPolicy d_policyCallback;
    void * d_policyUserdata;

    vrpn_Shared_String & set (const char *, timeval,
                             vrpn_bool isLocalSet);

    virtual vrpn_bool shouldAcceptUpdate (const char * newValue, timeval when,
                                    vrpn_bool isLocalSet);

    virtual void sendUpdate (void);
    void sendUpdate (const char * newValue,
                     timeval when);
    void encode (char ** buffer, vrpn_int32 * len, const char * newValue,
                     timeval when) const;
    void decode (const char ** buffer, vrpn_int32 * len,
                 char * newValue, timeval * when) const;

    int yankCallbacks (vrpn_bool isLocal);
      // must set d_lastUpdate BEFORE calling yankCallbacks()
        
    int handleUpdate (vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_lamportUpdate (void *, vrpn_HANDLERPARAM);

};

class VRPN_API vrpn_Shared_String_Server : public vrpn_Shared_String {

  public:

    vrpn_Shared_String_Server (const char * name,
                                const char * defaultValue = NULL,
                                vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_String_Server (void);

    vrpn_Shared_String_Server & operator = (const char *);

    virtual void bindConnection (vrpn_Connection *);

  protected:


};

class VRPN_API vrpn_Shared_String_Remote : public vrpn_Shared_String {

  public:

    vrpn_Shared_String_Remote (const char * name,
                                const char * defaultValue = NULL,
                                vrpn_int32 defaultMode = VRPN_SO_DEFAULT);
    virtual ~vrpn_Shared_String_Remote (void);

    vrpn_Shared_String_Remote & operator = (const char *);

    virtual void bindConnection (vrpn_Connection *);

};



#endif  // VRPN_SHARED_OBJECT

