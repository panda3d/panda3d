/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_distributed.cxx
 * @author drose
 * @date 2004-05-19
 */

#include "config_distributed.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DIRECT_DISTRIBUTED)
  #error Buildsystem error: BUILDING_DIRECT_DISTRIBUTED not defined
#endif

Configure(config_distributed);
NotifyCategoryDef(distributed, "");

ConfigureFn(config_distributed) {
  init_libdistributed();
}

ConfigVariableInt game_server_timeout_ms
("game-server-timeout-ms", 20000,
 PRC_DESC("This represents the amount of time to block waiting for the TCP "
          "connection to the game server.  It is only used when the connection "
          "method is NSPR."));

ConfigVariableDouble min_lag
("min-lag", 0.0,
 PRC_DESC("This represents the time in seconds by which to artificially lag "
          "inbound messages.  It is useful to test a game's tolerance of "
          "network latency."));

ConfigVariableDouble max_lag
("max-lag", 0.0,
 PRC_DESC("This represents the time in seconds by which to artificially lag "
          "inbound messages.  It is useful to test a game's tolerance of "
          "network latency."));

ConfigVariableBool handle_datagrams_internally
("handle-datagrams-internally", true,
 PRC_DESC("When this is true, certain datagram types can be handled "
          "directly by the C++ cConnectionRepository implementation, "
          "for performance reasons.  When it is false, all datagrams "
          "are handled by the Python implementation."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdistributed() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

}
