// Filename: config_net.cxx
// Created by:  drose (25Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "config_net.h"

#include "netDatagram.h"

#include "dconfig.h"

Configure(config_net);
NotifyCategoryDef(net, "");

ConfigureFn(config_net) {
  init_libnet();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libnet
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libnet() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

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
  return config_net.GetInt("net-max-response-queue", 50000);
}

bool get_net_error_abort() {
  return config_net.GetBool("net-error-abort", false);
}
