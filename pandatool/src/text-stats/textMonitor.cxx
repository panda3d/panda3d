/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textMonitor.cxx
 * @author drose
 * @date 2000-07-12
 */

#include "textMonitor.h"
#include "textStats.h"
#include "pStatCollectorDef.h"
#include "pStatFrameData.h"
#include "indent.h"
#include <stdio.h>  // sprintf

/**
 *
 */
TextMonitor::
TextMonitor(TextStats *server, std::ostream *outStream, bool show_raw_data ) : PStatMonitor(server) {
    _outStream = outStream;    //[PECI]
    _show_raw_data = show_raw_data;
}

/**
 * Returns the server that owns this monitor.
 */
TextStats *TextMonitor::
get_server() {
  return (TextStats *)PStatMonitor::get_server();
}

/**
 * Should be redefined to return a descriptive name for the type of
 * PStatsMonitor this is.
 */
std::string TextMonitor::
get_monitor_name() {
  return "Text Stats";
}

/**
 * Called when the "hello" message has been received from the client.  At this
 * time, the client's hostname and program name will be known.
 */
void TextMonitor::
got_hello() {
  nout << "Now connected to " << get_client_progname() << " on host "
       << get_client_hostname() << "\n";
}

/**
 * Like got_hello(), this is called when the "hello" message has been received
 * from the client.  At this time, the client's hostname and program name will
 * be known.  However, the client appears to be an incompatible version and
 * the connection will be terminated; the monitor should issue a message to
 * that effect.
 */
void TextMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  nout
    << "Rejected connection by " << get_client_progname()
    << " from " << get_client_hostname()
    << ".  Client uses PStats version "
    << client_major << "." << client_minor
    << ", while server expects PStats version "
    << server_major << "." << server_minor << ".\n";
}

/**
 * Called as each frame's data is made available.  There is no gurantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void TextMonitor::
new_data(int thread_index, int frame_number) {
  PStatView &view = get_view(thread_index);
  const PStatThreadData *thread_data = view.get_thread_data();

  if (frame_number == thread_data->get_latest_frame_number()) {
    view.set_to_frame(frame_number);

    if (view.all_collectors_known()) {
      const PStatClientData *client_data = get_client_data();

      (*_outStream) << "\rThread "
           << client_data->get_thread_name(thread_index)
           << " frame " << frame_number << ", "
           << view.get_net_value() * 1000.0 << " ms ("
           << thread_data->get_frame_rate() << " Hz):\n";

      if (_show_raw_data) {
        const PStatFrameData &frame_data = thread_data->get_frame(frame_number);
        (*_outStream) << "raw data:\n";
        int num_events = frame_data.get_num_events();
        for (int i = 0; i < num_events; ++i) {
          // The iomanipulators are much too clumsy.
          char formatted[32];
          sprintf(formatted, "%15.06lf", frame_data.get_time(i));
          (*_outStream) << formatted;

          if (frame_data.is_start(i)) {
            (*_outStream) << " start ";
          } else {
            (*_outStream) << " stop  ";
          }

          int collector_index = frame_data.get_time_collector(i);
          (*_outStream) << client_data->get_collector_fullname(collector_index) << "\n";
        }
      }

      const PStatViewLevel *level = view.get_top_level();
      int num_children = level->get_num_children();
      for (int i = 0; i < num_children; i++) {
        show_ms(level->get_child(i), 2);
      }

      int num_toplevel_collectors = client_data->get_num_toplevel_collectors();
      for (int tc = 0; tc < num_toplevel_collectors; tc++) {
        int collector = client_data->get_toplevel_collector(tc);
        if (client_data->has_collector(collector) &&
            client_data->get_collector_has_level(collector, thread_index)) {

          PStatView &level_view = get_level_view(collector, thread_index);
          level_view.set_to_frame(frame_number);
          const PStatViewLevel *level = level_view.get_top_level();
          show_level(level, 2);
        }
      }
    }
  }
  _outStream->flush();
}


/**
 * Called whenever the connection to the client has been lost.  This is a
 * permanent state change.  The monitor should update its display to represent
 * this, and may choose to close down automatically.
 */
void TextMonitor::
lost_connection() {
  nout << "Lost connection.\n";
}

/**
 * Should be redefined to return true if this monitor class can handle running
 * in a sub-thread.
 *
 * This is not related to the question of whether it can handle multiple
 * different PStatThreadDatas; this is strictly a question of whether or not
 * the monitor itself wants to run in a sub-thread.
 */
bool TextMonitor::
is_thread_safe() {
  return false;
}

/**
 *
 */
void TextMonitor::
show_ms(const PStatViewLevel *level, int indent_level) {
  int collector_index = level->get_collector();

  const PStatClientData *client_data = get_client_data();
  const PStatCollectorDef &def = client_data->get_collector_def(collector_index);

  indent((*_outStream), indent_level)
    << def._name << " = " << level->get_net_value() * 1000.0 << " ms\n" ;

  int num_children = level->get_num_children();
  for (int i = 0; i < num_children; i++) {
    show_ms(level->get_child(i), indent_level + 2);
  }
}

/**
 *
 */
void TextMonitor::
show_level(const PStatViewLevel *level, int indent_level) {
  int collector_index = level->get_collector();

  const PStatClientData *client_data = get_client_data();
  const PStatCollectorDef &def = client_data->get_collector_def(collector_index);

  indent((*_outStream), indent_level)
    << def._name << " = " << level->get_net_value() << " "
    << def._level_units << "\n";

  int num_children = level->get_num_children();
  for (int i = 0; i < num_children; i++) {
    show_level(level->get_child(i), indent_level + 2);
  }
}
