/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_net.cxx
 * @author drose
 * @date 2000-02-25
 */

#include "config_net.h"

#include "netDatagram.h"
#include "pandaSystem.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_NET)
  #error Buildsystem error: BUILDING_PANDA_NET not defined
#endif

Configure(config_net);
NotifyCategoryDef(net, "");

ConfigureFn(config_net) {
  init_libnet();
}




// The following two maximum queue sizes are totally arbitrary and serve only
// to provide sanity caps on the various queues in the net package.  You can
// set them to any sane values you like.  Also see the set_max_queue_size()
// methods in the various classes, which you can change at runtime on a
// particular instance.

int
get_net_max_write_queue() {
  static ConfigVariableInt *net_max_write_queue = nullptr;

  if (net_max_write_queue == nullptr) {
    net_max_write_queue = new ConfigVariableInt
      ("net-max-write-queue", 10000,
       PRC_DESC("This limits the number of datagrams in a ConnectionWriter's "
                "output queue."));
  }

  return *net_max_write_queue;
}

int
get_net_max_response_queue() {
  static ConfigVariableInt *net_max_response_queue = nullptr;

  if (net_max_response_queue == nullptr) {
    net_max_response_queue = new ConfigVariableInt
      ("net-max-response-queue", 50000,
       PRC_DESC("This limits the number of datagrams, messages, what have you, "
                "in the various QueuedConnectionReader, QueuedConnectionListener, "
                "and QueuedConnectionManager classes."));
  }

  return *net_max_response_queue;
}

bool
get_net_error_abort() {
  static ConfigVariableBool *net_error_abort = nullptr;

  if (net_error_abort == nullptr) {
    net_error_abort = new ConfigVariableBool
      ("net-error-abort", false);
  }

  return *net_error_abort;
}

double
get_net_max_poll_cycle() {
  static ConfigVariableDouble *net_max_poll_cycle = nullptr;

  if (net_max_poll_cycle == nullptr) {
    net_max_poll_cycle = new ConfigVariableDouble
      ("net-max-poll-cycle", 0.2,
       PRC_DESC("Specifies the maximum amount of time, in seconds, to "
                "continue to read data within one cycle of the poll() "
                "call.  If this is negative, the program will wait as "
                "long as data is available to be read.  Setting this to "
                "a reasonable value prevents poll() from completely "
                "starving the rest of the application when data is coming "
                "in faster than it can be processed."));
  }

  return *net_max_poll_cycle;
}

double
get_net_max_block() {
  static ConfigVariableDouble *net_max_block = nullptr;

  if (net_max_block == nullptr) {
    net_max_block = new ConfigVariableDouble
      ("net-max-block", 0.01,
       PRC_DESC("Specifies the maximum amount of time, in seconds, to "
                "completely block the process during any blocking wait "
                "in the net subsystem.  This is an internal timeout only, "
                "and gives the net subsystem a chance to detect things "
                "like explicitly-closed connections in another thread; it "
                "does not affect the blocking behavior at the high "
                "level."));
  }

  return *net_max_block;
}

// This function is used in the ReaderThread and WriterThread constructors to
// make a simple name for each thread.
std::string
make_thread_name(const std::string &thread_name, int thread_index) {
  std::ostringstream stream;
  stream << thread_name << "_" << thread_index;
  return stream.str();
}


ConfigVariableInt net_max_read_per_epoch
("net-max-read-per-epoch", 1024,
 PRC_DESC("The maximum number of bytes to read from the net in a single "
          "thread epoch, when SIMPLE_THREADS is defined.  This is designed "
          "to minimize the impact of the networking layer on the other "
          "threads."));

ConfigVariableInt net_max_write_per_epoch
("net-max-write-per-epoch", 1024,
 PRC_DESC("The maximum number of bytes to write to the net in a single "
          "thread epoch, when SIMPLE_THREADS is defined.  This is designed "
          "to minimize the impact of the networking layer on the other "
          "threads."));

ConfigVariableEnum<ThreadPriority> net_thread_priority
("net-thread-priority", TP_low,
 PRC_DESC("The default thread priority when creating threaded readers "
          "or writers."));


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libnet() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  NetDatagram::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("net");
}
