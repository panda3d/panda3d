// Filename: deadrec_rec.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"

#include <dconfig.h>

NotifyCategoryDecl(deadrec, EXPCL_MISC, EXPTP_MISC);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

typedef set<PT(Connection)> Clients;

static PT_Node smiley;
static RenderRelation* my_arc;
static int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);
static thread* monitor;
static bool stop_monitoring;
QueuedConnectionListener* listener;
QueuedConnectionManager cm;
Clients clients;
QueuedConnectionReader* reader;

static void* internal_monitor(void*) {
  if (deadrec->is_debug())
    deadrec->debug() << "internal monitoring thread started" << endl;
  while (!stop_monitoring) {
    // check for new clients
    while (listener->new_connection_available()) {
      PT(Connection) rv;
      NetAddress address;
      PT(Connection) new_connection;
      if (listener->get_new_connection(rv, address, new_connection)) {
	if (deadrec_cat->is_debug())
	  deadrec_cat->debug() << "Got connection from " << address << endl;
	reader->add_connection(new_connection);
	clients.insert(new_connection);
      }
    }
    // check for reset clients
    while (cm.reset_connection_available()) {
      PT(Connection) connection;
      if (cm.get_reset_connection(connection)) {
	if (deadrec_cat->is_debug())
	  deadrec_cat->debug() << "Lost connection from "
			       << connection->get_address() << endl;
	clients.erase(connection);
	cm.close_connection(connection);
      }
    }
    // process all available datagrams
    while (reader->data_available()) {
      NetDatagram datagram;
      if (reader->get_data(datagram)) {
	if (deadrec_cat->is_debug())
	  deadrec_cat->debug() << "Got datagram " << datagram << " from "
			       << datagram.get_address() << endl;
	// unpack and deal with the datagram now
	// DO THIS
	// part of this includes logic on when to shutdown, I hope
      }
    }
    // sleep for about 100 milliseconds
    ipc_traits::sleep(0, 100000);
  }
  if (deadrec->is_debug())
    deadrec->debug() << "internal monitoring thread exiting" << endl;
  return (void*)0L;
}

static void deadrec_setup(void) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
  // prepair to get a connection
  PT(Connection) rendezvous = cm.open_TCP_server_rendezvous(hostport, 5);
  if (rendezvous.is_null()) {
    deadrec_cat->fatal() << "cannot get port " << hostport << endl;
    exit();
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "Listening for connections on port " << port
			 << endl;
  listener = new QueuedConnectionListener(&cm, 0);
  listener->add_connection(rendezvous);
  reader = new QueuedConnectionReader(&cm, 1);
  stop_monitoring = false;
  monitor = thread::create(internal_monitor, (void*)0L,
			   thread::PRIORITY_NORMAL);
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup();
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
