// Filename: config_net.cxx
// Created by:  drose (25Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "config_net.h"

#include "netDatagram.h"

#include <dconfig.h>

Configure(config_net);
NotifyCategoryDef(net, "");

ConfigureFn(config_net) {
  NetDatagram::init_type();
}

// The following two maximum queue sizes are totally arbitrary and
// serve only to provide sanity caps on the various queues in the net
// package.  You can set them to any sane values you like.  Also see
// the set_max_queue_size() methods in the various classes, which you
// can change at runtime on a particular instance.

// This one limits the number of datagrams in a ConnectionWriter's
// output queue.
int get_net_max_write_queue() {
  return config_net.GetInt("net-max-write-queue", 10000);
}

// This one limits the number of datagrams, messages, what have you,
// in the various QueuedConnectionReader, QueuedConnectionListener,
// and QueuedConnectionManager classes.
int get_net_max_response_queue() {
  return config_net.GetInt("net-max-response-queue", 10000);
}

bool get_net_error_abort() {
  return config_net.GetBool("net-error-abort", false);
}
