#ifndef VRPN_FORWARDER_H
#define VRPN_FORWARDER_H

#include "vrpn_Connection.h"  // for vrpn_HANDLERPARAM


// vrpn_Forwarder
// Tom Hudson, August 1998
//
// Class to take messages from one VRPN connection and send them out
// on another.

// Design decisions:
//   Scale of forwarding:
//     Could write a forwarder per stream (serviceName per instantiation)
//     or per connection (serviceName per forward() call).  Latter is
//     more flexible, but takes up more memory if few distinct streams need
//     to be forwarded, has a clunkier syntax, ...
//   Flexibility of naming:
//     We allow users to take in a message of one name and send it out
//     with another name;  this is useful and dangerous.

// Faults:
//   There is currently no way to specify vrpn_SENDER_ANY as a source.
// If we do, it isn't clear what sender to specify to the destination.

class VRPN_API vrpn_ConnectionForwarder {

  public:

    // Set up to forward messages from <source> to <destination>
    vrpn_ConnectionForwarder (vrpn_Connection * source,
                              vrpn_Connection * destination);

    ~vrpn_ConnectionForwarder (void);



    // Begins forwarding of a message type.
    // Forwards messages of type <sourceName> and sender <sourceServiceName>,
    // sending them out as type <destinationName> from sender
    // <destinationServiceName>.
    // Return nonzero on failure.
    int forward (const char * sourceName,
                 const char * sourceServiceName,
                 const char * destinationName,
                 const char * destinationServiceName,
                 vrpn_uint32 classOfService = vrpn_CONNECTION_RELIABLE);

    // Stops forwarding of a message type.
    // Return nonzero on failure.
    int unforward (const char * sourceName,
                   const char * sourceServiceName,
                   const char * destinationName,
                   const char * destinationServiceName,
                   vrpn_uint32 classOfService = vrpn_CONNECTION_RELIABLE);

  private:

    static int VRPN_CALLBACK handle_message (void *, vrpn_HANDLERPARAM);

    // Translates (id, serviceId) from source to destination
    // and looks up intended class of service.
    // Returns nonzero if lookup fails.
    vrpn_int32 map (vrpn_int32 * id, vrpn_int32 * serviceId, vrpn_uint32 * serviceClass);

    vrpn_Connection * d_source;
    vrpn_Connection * d_destination;

    struct vrpn_CONNECTIONFORWARDERRECORD {

      vrpn_CONNECTIONFORWARDERRECORD (vrpn_Connection *, vrpn_Connection *,
           const char *, const char *, const char *, const char *,
           vrpn_uint32);

      vrpn_int32 sourceId;              // source's type id
      vrpn_int32 sourceServiceId;       // source's sender id
      vrpn_int32 destinationId;         // destination's type id
      vrpn_int32 destinationServiceId;  // destination's sender id
      vrpn_uint32 classOfService;  // class of service to send

      vrpn_CONNECTIONFORWARDERRECORD * next;
    };

    vrpn_CONNECTIONFORWARDERRECORD * d_list;

};

class VRPN_API vrpn_StreamForwarder {

  public:

    // Set up to forward messages from sender <sourceServiceName> on <source>
    // to <destination>, as if from sender <destinationServiceName>
    vrpn_StreamForwarder
        (vrpn_Connection * source,
         const char * sourceServiceName,
         vrpn_Connection * destination,
         const char * destinationServiceName);

    ~vrpn_StreamForwarder (void);



    // Begins forwarding of a message type.
    // Return nonzero on failure.
    int forward (const char * sourceName,
                 const char * destinationName,
                 vrpn_uint32 classOfService = vrpn_CONNECTION_RELIABLE);

    // Stops forwarding of a message type.
    // Return nonzero on failure.
    int unforward (const char * sourceName,
                   const char * destinationName,
                   vrpn_uint32 classOfService = vrpn_CONNECTION_RELIABLE);

  private:

    static int VRPN_CALLBACK handle_message (void *, vrpn_HANDLERPARAM);

    // Translates (id, serviceId) from source to destination
    // and looks up intended class of service.
    // Returns nonzero if lookup fails.
    vrpn_int32 map (vrpn_int32 * id, vrpn_uint32 * serviceClass);

    vrpn_Connection * d_source;
    vrpn_int32 d_sourceService;
    vrpn_Connection * d_destination;
    vrpn_int32 d_destinationService;

    struct vrpn_STREAMFORWARDERRECORD {

      vrpn_STREAMFORWARDERRECORD (vrpn_Connection *, vrpn_Connection *,
           const char *, const char *, vrpn_uint32);

      vrpn_int32 sourceId;       // source's type id
      vrpn_int32 destinationId;  // destination's type id
      vrpn_uint32 classOfService;  // class of service to send

      vrpn_STREAMFORWARDERRECORD * next;
    };

    vrpn_STREAMFORWARDERRECORD * d_list;

};


#endif  // VRPN_FORWARDER_H

