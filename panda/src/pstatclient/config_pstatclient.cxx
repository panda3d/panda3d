/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_pstatclient.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "config_pstatclient.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_PSTATCLIENT)
  #error Buildsystem error: BUILDING_PANDA_PSTATCLIENT not defined
#endif

ConfigureDef(config_pstatclient);
NotifyCategoryDef(pstats, "");

ConfigureFn(config_pstatclient) {
  init_libpstatclient();
}

ConfigVariableString pstats_name
("pstats-name", "Panda Stats");

ConfigVariableDouble pstats_max_rate
("pstats-max-rate", 1000.0,
 PRC_DESC("The maximum number of packets per second, per thread, to send "
          "to the remote PStats server.  A packet is defined as a single "
          "UDP packet, or each 1024 bytes of a TCP message."));

ConfigVariableBool pstats_threaded_write
("pstats-threaded-write", true,
 PRC_DESC("Set this true to write to the PStats channel in a sub-thread, if "
          "threading is available.  Can't think of any reason why you "
          "wouldn't want this set true, unless you suspect something is "
          "broken with the threaded network interfaces."));

ConfigVariableInt pstats_max_queue_size
("pstats-max-queue-size", 1,
 PRC_DESC("If pstats-threaded-write is true, this specifies the maximum "
          "number of packets (generally, frames of data) that may be queued "
          "up for the thread to process.  If this is large, the writer "
          "thread may fall behind and the output of PStats will lag.  Keep "
          "this small to drop missed packets on the floor instead, and "
          "ensure that the frame data does not grow stale."));

ConfigVariableDouble pstats_tcp_ratio
("pstats-tcp-ratio", 0.01,
 PRC_DESC("This specifies the ratio of frame update messages that are eligible "
          "for UDP that are sent via TCP instead.  It does not count messages "
          "that are too large for UDP and must be sent via TCP anyway.  1.0 "
          "means all messages are sent TCP; 0.0 means all are sent UDP."));

ConfigVariableString pstats_host
("pstats-host", "localhost");

// The default port for PStats used to be 5180, but that's used by AIM.
ConfigVariableInt pstats_port
("pstats-port", 5185);

ConfigVariableDouble pstats_target_frame_rate
("pstats-target-frame-rate", 30.0,
 PRC_DESC("Specify the target frame rate to highlight on the PStats graph.  "
          "This frame rate is marked with a different-colored line; "
          "otherwise, this setting has no effect."));

ConfigVariableBool pstats_gpu_timing
("pstats-gpu-timing", false,
 PRC_DESC("Set this true to query the graphics library for the actual time "
          "that graphics operations take to execute on the video card.  "
          "Enabling this will harm performance, but this information can "
          "be more useful than the regular Draw information in tracking "
          "down bottlenecks, because the CPU-based Draw collectors only "
          "measure how long it takes for the API call to complete, which "
          "is not usually an accurate reflectino of how long the actual "
          "operation takes on the video card."));

// The rest are different in that they directly control the server, not the
// client.
ConfigVariableBool pstats_scroll_mode
("pstats-scroll-mode", true);
ConfigVariableDouble pstats_history
("pstats-history", 60.0);
ConfigVariableDouble pstats_average_time
("pstats-average-time", 3.0);

ConfigVariableBool pstats_mem_other
("pstats-mem-other", true,
 PRC_DESC("Set this true to collect memory categories smaller than 0.1% of "
          "the total into a single \"Other\" category, or false to show "
          "each nonzero memory category."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libpstatclient() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}
