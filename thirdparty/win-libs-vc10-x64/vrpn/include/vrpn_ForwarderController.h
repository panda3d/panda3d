#ifndef VRPN_FORWARDER_CONTROLLER_H
#define VRPN_FORWARDER_CONTROLLER_H

// vrpn_Forwarder_Controller
//
// Tom Hudson, September 1998

// Written to allow remote a client to tell a server to open another port
// and forward some messages on it to a friend of the client's.

// Any server that wishes to implement this needs only construct a
// vrpn_Forwarder_Server for each server connection it has open and
// to call the vrpn_Forwarder_Server mainloop frequently.

// Clients can construct a vrpn_Forwarder_Controller on a connection
// and call start_remote_forwarding(port) to tell the server to open
// <port>, then call forward_message_type(port, name) to start forwarding
// messages of the given name.

// This isn't an ideal solution, because it means clients need access to
// the names of the message, which they are normally insulated from.

// Some of the fancier options of the Forwarder (renaming services or
// types, changing class of service) are hidden from the user;  this
// is meant to be a simple interface and simple first implementation.

// New Forwarder_Servers are NOT constructed on connections that a
// Forwarder_Server opens, so clients that are only listening to a
// forwarded stream cannot open new forwarders for still other clients to
// listen to.

#include "vrpn_Connection.h"  // for vrpn_HANDLERPARAM

class VRPN_API vrpn_ConnectionForwarder;

class VRPN_API vrpn_Forwarder_Brain {

  public:

    vrpn_Forwarder_Brain (vrpn_Connection *);
    virtual ~vrpn_Forwarder_Brain (void);

    // Tell a Forwarder_Server to open a vrpn_Connection on remote_port.

    virtual void start_remote_forwarding (vrpn_int32 remote_port) = 0;

    // Tell a Forwarder_Server to begin forwarding messages of type
    // message_type from the sender named service_name over remote_port.

    virtual void forward_message_type (vrpn_int32 remote_port,
                                       const char * service_name,
                                       const char * message_type) = 0;

  protected:

    vrpn_Connection * d_connection;

    vrpn_int32 d_myId;

    vrpn_int32 d_start_forwarding_type;
    vrpn_int32 d_forward_type;

    static char * encode_start_remote_forwarding (vrpn_int32 * length,
                       vrpn_int32 remote_port);
    static char * encode_forward_message_type (vrpn_int32 * length,
                       vrpn_int32 remote_port, const char * service_name,
                       const char * message_type);

    static void decode_start_remote_forwarding (const char * buffer,
                       vrpn_int32 * remote_port);
    static void decode_forward_message_type (const char * buffer,
                       vrpn_int32 * remote_port, char ** service_name,
                       char ** message_type);

};


// Server class

// VRPN server builders who want to enable remotely-controlled forwarding in
// their server need only create a Forwarder_Server on their server Connections
// and call its mainloop() regularly.

struct vrpn_Forwarder_List {
  vrpn_Forwarder_List * next;
  vrpn_int32 port;
  vrpn_Connection * connection;
  vrpn_ConnectionForwarder * forwarder;
};

class VRPN_API vrpn_Forwarder_Server : public vrpn_Forwarder_Brain {

  public:

    vrpn_Forwarder_Server (vrpn_Connection *);
    virtual ~vrpn_Forwarder_Server (void);

    virtual void mainloop (void);

    virtual void start_remote_forwarding
                 (vrpn_int32 remote_port);

    virtual void forward_message_type
                 (vrpn_int32 remote_port, const char * service_name,
                  const char * message_type);

  protected:

    vrpn_Forwarder_List * d_myForwarders;

  private:

    static int VRPN_CALLBACK handle_start (void *, vrpn_HANDLERPARAM);
    static int VRPN_CALLBACK handle_forward (void *, vrpn_HANDLERPARAM);

};

// Client class

// Construct a Forwarder_Controller on a connection to control a
// Forwarder_Server on its far end.

class VRPN_API vrpn_Forwarder_Controller : public vrpn_Forwarder_Brain {

  public:

    vrpn_Forwarder_Controller (vrpn_Connection *);
    ~vrpn_Forwarder_Controller (void);

    virtual void start_remote_forwarding (vrpn_int32 remote_port);

    virtual void forward_message_type
                 (vrpn_int32 remote_port, const char * service_name,
                  const char * message_type);

};

#endif  // VRPN_FORWARDER_CONTROLLER_H

