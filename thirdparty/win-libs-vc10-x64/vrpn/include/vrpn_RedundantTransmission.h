#ifndef VRPN_REDUNDANT_TRANSMISSION_H
#define VRPN_REDUNDANT_TRANSMISSION_H

/// @class vrpn_RedundantTransmission
/// Helper class for vrpn_Connection that automates redundant transmission
/// for unreliable (low-latency) messages.  Call pack_messages() here instead
/// of on your connection, and call mainloop() here before calling mainloop()
/// on your connection.

#include "vrpn_Shared.h"  // for timeval, types
#include "vrpn_BaseClass.h"
#include "vrpn_Connection.h"  // for vrpn_HANDLERPARAM, vrpn_Connection

class VRPN_API vrpn_RedundantTransmission {

  public:

    vrpn_RedundantTransmission (vrpn_Connection * c);
    ~vrpn_RedundantTransmission (void);


    // ACCESSORS


    vrpn_uint32 defaultRetransmissions (void) const;
    timeval defaultInterval (void) const;
    vrpn_bool isEnabled (void) const;


    // MANIPULATORS


    virtual void mainloop (void);
      ///< Determines which messages need to be resent and queues
      ///< them up on the connection for transmission.

    void enable (vrpn_bool);

    virtual void setDefaults (vrpn_uint32 numRetransmissions,
                              timeval transmissionInterval);
      ///< Set default values for future calls to pack_message().

    virtual int pack_message
      (vrpn_uint32 len, timeval time, vrpn_uint32 type,
       vrpn_uint32 sender, const char * buffer,
       vrpn_uint32 class_of_service,
       vrpn_int32 numRetransmissions = -1,
       timeval * transmissionInterval = NULL);
      ///< If !isEnabled(), does a normal pack_message(), but if isEnabled()
      ///< ignores class_of_service and sends it vrpn_CONNECTION_LOW_LATENCY,
      ///< sending it an additional number of times equal to numRetransmissions
      ///< at minimum intervals of transmissionInterval.
      ///< Specify -1 and NULL to use default values.

  protected:

    vrpn_Connection * d_connection;

    struct queuedMessage {
      vrpn_HANDLERPARAM p;
      vrpn_uint32 remainingTransmissions;
      timeval transmissionInterval;
      timeval nextValidTime;
      queuedMessage * next;
    };

    queuedMessage * d_messageList;
    vrpn_uint32 d_numMessagesQueued;
      ///< For debugging, mostly.

    // Default values.

    vrpn_uint32 d_numTransmissions;
    timeval d_transmissionInterval;

    vrpn_bool d_isEnabled;


};


struct vrpn_RedundantController_Protocol {

  char * encode_set (int * len, vrpn_uint32 num, timeval interval);
  void decode_set (const char ** buf, vrpn_uint32 * num, timeval * interval);

  char * encode_enable (int * len, vrpn_bool);
  void decode_enable (const char ** buf, vrpn_bool *);

  void register_types (vrpn_Connection *);

  vrpn_int32 d_set_type;
  vrpn_int32 d_enable_type;
};


/// @class vrpn_RedundantController
/// Accepts commands over a connection to control a local
/// vrpn_RedundantTransmission's default parameters.
  
class VRPN_API vrpn_RedundantController : public vrpn_BaseClass {

  public:

    vrpn_RedundantController (vrpn_RedundantTransmission *, vrpn_Connection *);
    ~vrpn_RedundantController (void);

    void mainloop (void);
      // Do nothing;  vrpn_BaseClass requires this.

  protected:

    virtual int register_types (void);

    vrpn_RedundantController_Protocol d_protocol;

    static int VRPN_CALLBACK handle_set (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_enable (void *, vrpn_HANDLERPARAM);

    vrpn_RedundantTransmission * d_object;
};



/// @class vrpn_RedundantRemote
/// Sends messages to a vrpn_RedundantController so that a
/// vrpn_RedundantTransmission on a server can be controlled from a client.

class VRPN_API vrpn_RedundantRemote : public vrpn_BaseClass {

  public:

    vrpn_RedundantRemote (vrpn_Connection *);
    ~vrpn_RedundantRemote (void);

    void mainloop (void);
      // Do nothing;  vrpn_BaseClass requires this.

    void set (int numRetransmissions, timeval transmissionInterval);
    void enable (vrpn_bool);


  protected:

    int register_types (void);

    vrpn_RedundantController_Protocol d_protocol;
};




/// @class vrpn_RedundantReceiver
/// Helper class that eliminates duplicates;  only the first instance of
/// a message is delivered.  Registers a callback on connection for any
/// type it's told to monitor;  when it gets a message back, checks its
/// list of recently-seen-timestamps for that type;  if it isn't on the
/// list, it's dispatched and replaces the oldest item on the list.
/// List length is limited, so
/// if too many messages of the same type (more than VRPN_RR_LENGTH) are
/// interleaved - if transmissionInterval * numRetransmissions >
/// VRPN_RR_LENGTH * the normal rate of message generation - it will not
/// detect the redundant messages.

// A TypeDispatcher insists on too much control of its table for
// us to use one here - we want to use the same indices as the
// vrpn_Connection we're attached to, but if we had our own TypeDispatcher
// we'd have an independent, inconsistent set of type & sender ids.

#define VRPN_RR_LENGTH 8

class VRPN_API vrpn_RedundantReceiver {

  public:

    vrpn_RedundantReceiver (vrpn_Connection *);
    ~vrpn_RedundantReceiver (void);

    virtual int register_handler (vrpn_int32 type,
                vrpn_MESSAGEHANDLER handler, void * userdata,
                vrpn_int32 sender = vrpn_ANY_SENDER);
    virtual int unregister_handler (vrpn_int32 type,
                vrpn_MESSAGEHANDLER handler, void * userdata,
                vrpn_int32 sender = vrpn_ANY_SENDER);

    void record (vrpn_bool);
      ///< Turns "memory" (tracking statistics of redundant reception)
      ///< on and off.

    void writeMemory (const char * filename);
      ///< Writes statistics to the named file:  timestamp of every message
      ///< received and number of copies of that message.  Detects partial
      ///< losses, but not when all copies are lost, since vrpn_RR doesn't
      ///< expect messages.

    void clearMemory (void);
      ///< Throws away / resets statistics.

  protected:

    vrpn_Connection * d_connection;

    struct VRPN_API RRRecord {
      RRRecord (void);

      timeval timestampSeen [VRPN_RR_LENGTH];
      int numSeen [VRPN_RR_LENGTH];
      int nextTimestampToReplace;

      vrpnMsgCallbackEntry * cb;
      vrpn_bool handlerIsRegistered;
    };

    RRRecord d_records [vrpn_CONNECTION_MAX_TYPES];
    RRRecord d_generic;

    struct RRMemory {
      timeval timestamp;
      int numSeen;
      RRMemory * next;
    };

    RRMemory * d_memory;
    RRMemory * d_lastMemory;
    vrpn_bool d_record;

    static int VRPN_CALLBACK handle_possiblyRedundantMessage (void *, vrpn_HANDLERPARAM); 
};


#endif  // VRPN_REDUNDANT_TRANSMISSION_H


