// Filename: deadrec_rec.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <renderRelation.h>
#include <queuedConnectionManager.h>
#include <queuedConnectionListener.h>
#include <modelPool.h>
#include <ipc_thread.h>
#include <transformTransition.h>

#include <dconfig.h>

NotifyCategoryDeclNoExport(deadrec, EXPCL_MISC, EXPTP_MISC);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

typedef set<PT(Connection)> Clients;

static PT_Node smiley;
static RenderRelation* my_arc;
static LPoint3f my_pos;
static int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);
static thread* monitor;
static bool stop_monitoring;
QueuedConnectionListener* listener;
QueuedConnectionManager cm;
Clients clients;
QueuedConnectionReader* reader;

enum TelemetryToken { T_End = 1, T_Pos, T_Vel, T_Num };

static inline unsigned char* get_uint8(unsigned char* b, unsigned char& v) {
  v = b[0];
  return ++b;
}

static inline unsigned char* get_float64(unsigned char* b, float& f) {
  unsigned char t[8]; // 64-bits
  memcpy(t, b, 8);
  if (sizeof(float)==8) {
    memcpy(&f, t, 8);
  } else if (sizeof(double)==8) {
    double d;
    memcpy(&d, t, 8);
    f = d;
  } else {
    deadrec_cat->error() << "neither float or double are 64-bit" << endl;
    f = 0.;
  }
  return b+8;
}

static void* internal_monitor(void*) {
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "internal monitoring thread started" << endl;
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
	unsigned char* buff = (unsigned char*)(datagram.get_data());
	unsigned char byte;
	TelemetryToken t;
	buff = get_uint8(buff, byte);
	t = (TelemetryToken)byte;
	while (t != T_End) {
	  switch (t) {
	  case T_Pos:
	    float x, y, z;
	    buff = get_float64(get_float64(get_float64(buff, x), y), z);
	    my_pos = LPoint3f(x, y, z);
	    break;
	  case T_Vel:
	    if (deadrec_cat->is_debug())
	      deadrec_cat->debug() << "got T_Num" << endl;
	    break;
	  case T_Num:
	    if (deadrec_cat->is_debug())
	      deadrec_cat->debug() << "got T_Num" << endl;
	    break;
	  default:
	    deadrec_cat->warning() << "got bad token in datagram (" << (int)t
				   << ")" << endl;
	  }
	  buff = get_uint8(buff, byte);
	  t = (TelemetryToken)byte;
	}
	// unpack and deal with the datagram now
	// DO THIS
	// part of this includes logic on when to shutdown, I hope
      }
    }
    // sleep for about 100 milliseconds
    ipc_traits::sleep(0, 100000);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "internal monitoring thread exiting" << endl;
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
    exit(0);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "Listening for connections on port " << hostport
			 << endl;
  listener = new QueuedConnectionListener(&cm, 0);
  listener->add_connection(rendezvous);
  reader = new QueuedConnectionReader(&cm, 1);
  stop_monitoring = false;
  monitor = thread::create(internal_monitor, (void*)0L,
			   thread::PRIORITY_NORMAL);
}

static void update_smiley(void) {
  LMatrix4f mat = LMatrix4f::translate_mat(my_pos);
  my_arc->set_transition(new TransformTransition(mat));
}

static void event_frame(CPT_Event) {
  update_smiley();
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup();

  eh.add_hook("NewFrame", event_frame);
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
