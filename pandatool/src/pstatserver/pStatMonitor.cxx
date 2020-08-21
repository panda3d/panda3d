/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatMonitor.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "pStatMonitor.h"

#include "pStatCollectorDef.h"

using std::string;


/**
 *
 */
PStatMonitor::
PStatMonitor(PStatServer *server) : _server(server) {
  _client_known = false;
}

/**
 *
 */
PStatMonitor::
~PStatMonitor() {
}

/**
 * Called shortly after startup time with the greeting from the client.  This
 * indicates the client's reported hostname and program name.
 */
void PStatMonitor::
hello_from(const string &hostname, const string &progname) {
  _client_known = true;
  _client_hostname = hostname;
  _client_progname = progname;
  got_hello();
}

/**
 * Called shortly after startup time with the greeting from the client.  In
 * this case, the client seems to have an incompatible version and will be
 * automatically disconnected; the server should issue a message to that
 * effect.
 */
void PStatMonitor::
bad_version(const string &hostname, const string &progname,
            int client_major, int client_minor,
            int server_major, int server_minor) {
  _client_known = true;
  _client_hostname = hostname;
  _client_progname = progname;
  got_bad_version(client_major, client_minor,
                  server_major, server_minor);
}

/**
 * Called by the PStatServer at setup time to set the new data pointer for the
 * first time.
 */
void PStatMonitor::
set_client_data(PStatClientData *client_data) {
  _client_data = client_data;
  initialized();
}

/**
 * Returns true if the client is alive and connected, false otherwise.
 */
bool PStatMonitor::
is_alive() const {
  if (_client_data.is_null()) {
    // Not yet, but in a second probably.
    return false;
  }
  return _client_data->is_alive();
}

/**
 * Closes the client connection if it is active.
 */
void PStatMonitor::
close() {
  if (!_client_data.is_null()) {
    _client_data->close();
  }
}

/**
 * Returns the color associated with the indicated collector.  If the
 * collector has no associated color, or is unknown, a new color will be made
 * up on the spot and associated with this collector for the rest of the
 * session.
 */
const LRGBColor &PStatMonitor::
get_collector_color(int collector_index) {
  Colors::iterator ci;
  ci = _colors.find(collector_index);
  if (ci != _colors.end()) {
    return (*ci).second;
  }

  // Ask the client data about the color.
  if (!_client_data.is_null() &&
      _client_data->has_collector(collector_index)) {
    const PStatCollectorDef &def =
      _client_data->get_collector_def(collector_index);

    LRGBColor sc(def._suggested_color.r,
                 def._suggested_color.g,
                 def._suggested_color.b);
    if (sc != LRGBColor::zero()) {
      ci = _colors.insert(Colors::value_type(collector_index, sc)).first;
      return (*ci).second;
    }

    // Use the fullname of the collector as a hash to seed the random number
    // generator (consulted below), so we get the same color for a given name
    // across sessions.
    string fullname = _client_data->get_collector_fullname(collector_index);
    unsigned int hash = 0;
    for (string::const_iterator ci = fullname.begin(); ci != fullname.end(); ++ci) {
      hash = hash * 37 + (unsigned int)(*ci);
    }
    srand(hash);
  }

  // We didn't have a color for the collector; make one up.
  LRGBColor random_color;
  random_color[0] = (double)rand() / (double)RAND_MAX;
  random_color[1] = (double)rand() / (double)RAND_MAX;
  random_color[2] = (double)rand() / (double)RAND_MAX;

  ci = _colors.insert(Colors::value_type(collector_index, random_color)).first;
  return (*ci).second;
}

/**
 * Returns a view on the given thread index.  If there is no such view already
 * for the indicated thread, this will create one.  This view can be used to
 * examine the accumulated data for the given thread.
 */
PStatView &PStatMonitor::
get_view(int thread_index) {
  Views::iterator vi;
  vi = _views.find(thread_index);
  if (vi == _views.end()) {
    vi = _views.insert(Views::value_type(thread_index, PStatView())).first;
    (*vi).second.set_thread_data(_client_data->get_thread_data(thread_index));
  }
  return (*vi).second;
}

/**
 * Returns a view on the level value (as opposed to elapsed time) for the
 * given collector over the given thread.  If there is no such view already
 * for the indicated thread, this will create one.
 */
PStatView &PStatMonitor::
get_level_view(int collector_index, int thread_index) {
  LevelViews::iterator lvi;
  lvi = _level_views.find(collector_index);
  if (lvi == _level_views.end()) {
    lvi = _level_views.insert(LevelViews::value_type(collector_index, Views())).first;
  }
  Views &views = (*lvi).second;

  Views::iterator vi;
  vi = views.find(thread_index);
  if (vi == views.end()) {
    vi = views.insert(Views::value_type(thread_index, PStatView())).first;
    (*vi).second.set_thread_data(_client_data->get_thread_data(thread_index));
    (*vi).second.constrain(collector_index, true);
  }
  return (*vi).second;
}

/**
 * Called after the monitor has been fully set up.  At this time, it will have
 * a valid _client_data pointer, and things like is_alive() and close() will
 * be meaningful.  However, we may not yet know who we're connected to
 * (is_client_known() may return false), and we may not know anything about
 * the threads or collectors we're about to get data on.
 */
void PStatMonitor::
initialized() {
}

/**
 * Called when the "hello" message has been received from the client.  At this
 * time, the client's hostname and program name will be known.
 */
void PStatMonitor::
got_hello() {
}

/**
 * Like got_hello(), this is called when the "hello" message has been received
 * from the client.  At this time, the client's hostname and program name will
 * be known.  However, the client appears to be an incompatible version and
 * the connection will be terminated; the monitor should issue a message to
 * that effect.
 */
void PStatMonitor::
got_bad_version(int, int, int, int) {
}

/**
 * Called whenever a new Collector definition is received from the client.
 * Generally, the client will send all of its collectors over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Collector definitions midstream.
 */
void PStatMonitor::
new_collector(int) {
}

/**
 * Called whenever a new Thread definition is received from the client.
 * Generally, the client will send all of its threads over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Thread definitions midstream.
 */
void PStatMonitor::
new_thread(int) {
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.  The use of the PStatFrameData / PStatView objects to report the
 * data will facilitate this.
 */
void PStatMonitor::
new_data(int, int) {
}

/**
 * Called whenever the connection to the client has been lost.  This is a
 * permanent state change.  The monitor should update its display to represent
 * this, and may choose to close down automatically.
 */
void PStatMonitor::
lost_connection() {
}

/**
 * If has_idle() returns true, this will be called periodically to allow the
 * monitor to update its display or whatever it needs to do.
 */
void PStatMonitor::
idle() {
}

/**
 * Should be redefined to return true if you want to redefine idle() and
 * expect it to be called.
 */
bool PStatMonitor::
has_idle() {
  return false;
}

/**
 * Should be redefined to return true if this monitor class can handle running
 * in a sub-thread.
 *
 * This is not related to the question of whether it can handle multiple
 * different PStatThreadDatas; this is strictly a question of whether or not
 * the monitor itself wants to run in a sub-thread.
 */
bool PStatMonitor::
is_thread_safe() {
  return false;
}

/**
 * Called when the user guide bars have been changed.
 */
void PStatMonitor::
user_guide_bars_changed() {
}
