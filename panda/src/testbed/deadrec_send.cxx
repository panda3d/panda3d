// Filename: deadrec_send.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <renderRelation.h>
#include <queuedConnectionManager.h>
#include <modelPool.h>

#include <dconfig.h>

NotifyCategoryDecl(deadrec, EXPCL_MISC, EXPTP_MISC);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

static PT_Node smiley;
static RenderRelation* my_arc;
string hostname = deadrec.GetString("deadrec-rec", "localhost");
int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);

static QueuedConnectionManager cm;
PT(Connection) conn;
ConnectionWriter* writer;

static void deadrec_setup(void) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
  // open a connection to the receiver
  NetAddress host;
  if (!host.set_host(hostname, hostport)) {
    deadrec_cat->fatal() << "Unknown host: " << hostname << endl;
    exit(0);
  }
  conn = cm.open_TCP_client_connection(host, 5000);
  if (conn.is_null()) {
    deadrec_cat->fatal() << "no connection." << endl;
    exit(0);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "opened TCP connection to " << hostname
			 << " on port " << conn->get_address().get_port()
			 << " and IP " << conn->get_address() << endl;
  writer = new ConnectionWriter(&cm, 0);
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup();
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
