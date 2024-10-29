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

#include "datagramInputFile.h"
#include "datagramOutputFile.h"
#include "pandaVersion.h"
#include "pStatCollectorDef.h"
#include "pStatTimeline.h"
#include "pStatStripChart.h"
#include "pStatFlameGraph.h"
#include "pStatPianoRoll.h"

using std::string;

static const string session_file_header("pstat\0\n\r", 8);
static const string layout_file_header("pslyt\0\n\r", 8);

static const Filename layout_filename = Filename::binary_filename(
#ifdef _WIN32
  Filename::expand_from("$USER_APPDATA/Panda3D-" PANDA_ABI_VERSION_STR "/pstats-layout")
#elif defined(__APPLE__)
  Filename::expand_from("$HOME/Library/Caches/Panda3D-" PANDA_ABI_VERSION_STR "/pstats-layout")
#else
  Filename::expand_from("$XDG_STATE_HOME/panda3d/pstats-layout")
#endif
);

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
  close();
}

/**
 * Called shortly after startup time with the greeting from the client.  This
 * indicates the client's reported hostname and program name.
 */
void PStatMonitor::
hello_from(const string &hostname, const string &progname, int pid) {
  _client_known = true;
  _client_hostname = hostname;
  _client_progname = progname;
  _client_pid = pid;
  got_hello();
}

/**
 * Called shortly after startup time with the greeting from the client.  In
 * this case, the client seems to have an incompatible version and will be
 * automatically disconnected; the server should issue a message to that
 * effect.
 */
void PStatMonitor::
bad_version(const string &hostname, const string &progname, int pid,
            int client_major, int client_minor,
            int server_major, int server_minor) {
  _client_known = true;
  _client_hostname = hostname;
  _client_progname = progname;
  _client_pid = 0;
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
 * Writes the data and the UI state to the given file.
 */
bool PStatMonitor::
write(const Filename &fn) const {
  DatagramOutputFile dof;
  if (!dof.open(fn)) {
    return false;
  }
  if (!dof.write_header(session_file_header)) {
    return false;
  }
  Datagram dg;
  dg.set_stdfloat_double(false);
  dg.add_uint16(1);
  dg.add_uint16(1);
  write_datagram(dg);
  dof.put_datagram(dg);
  dof.close();
  return true;
}

/**
 * Reads the data and the UI state from the given file.
 */
bool PStatMonitor::
read(const Filename &fn) {
  close();

  DatagramInputFile dif;
  if (!dif.open(fn)) {
    nout << "Failed to open " << fn << " for reading.\n";
    return false;
  }
  string header;
  if (!dif.read_header(header, 8) || header != session_file_header) {
    nout << "Session file contains invalid header.\n";
    return false;
  }
  Datagram dg;
  dg.set_stdfloat_double(false);

  if (!dif.get_datagram(dg)) {
    nout << "Failed to read datagram from session file.\n";
    return false;
  }

  DatagramIterator scan(dg);
  int version = scan.get_uint16();
  if (version != 1) {
    nout << "Unsupported session file version " << version << ".\n";
    return false;
  }
  // Room for a minor version number
  scan.get_uint16();

  read_datagram(scan);
  dif.close();

  idle();

  _read_filename = fn;
  _client_data->clear_dirty();

  return true;
}

/**
 * Opens the default set of graphs.
 */
void PStatMonitor::
open_default_graphs() {
  DatagramInputFile dif;
  if (!dif.open(layout_filename)) {
    open_strip_chart(0, 0, false);
    return;
  }
  string header;
  if (!dif.read_header(header, 8) || header != layout_file_header) {
    nout << "Layout file contains invalid header.\n";
    open_strip_chart(0, 0, false);
    return;
  }
  Datagram dg;
  dg.set_stdfloat_double(false);

  if (!dif.get_datagram(dg)) {
    nout << "Failed to read datagram from layout file.\n";
    open_strip_chart(0, 0, false);
    return;
  }

  DatagramIterator scan(dg);
  int version = scan.get_uint16();
  if (version != 1) {
    nout << "Unsupported layout file version " << version << ".\n";
    open_strip_chart(0, 0, false);
    return;
  }
  // Room for a minor version number
  scan.get_uint16();

  size_t num_colors = scan.get_uint32();
  for (size_t i = 0; i < num_colors; ++i) {
    int key = scan.get_int32();
    LRGBColor &color = _colors[key];
    color[0] = scan.get_float32();
    color[1] = scan.get_float32();
    color[2] = scan.get_float32();
  }

  PStatGraph *graph;

  size_t num_timelines = scan.get_uint16();
  for (size_t i = 0; i < num_timelines; ++i) {
    int x = scan.get_int32();
    int y = scan.get_int32();
    int width = scan.get_int32();
    int height = scan.get_int32();
    bool minimized = scan.get_bool();
    bool maximized = scan.get_bool();
    graph = open_timeline();
    graph->set_window_state(x, y, width, height, minimized, maximized);
  }

  size_t num_strip_charts = scan.get_uint16();
  for (size_t i = 0; i < num_strip_charts; ++i) {
    std::string thread_name = scan.get_string();
    std::string collector_name = scan.get_string();
    bool show_level = scan.get_bool();
    int thread_index = _client_data->find_thread(thread_name);
    int collector_index = _client_data->find_collector(collector_name);
    int x = scan.get_int32();
    int y = scan.get_int32();
    int width = scan.get_int32();
    int height = scan.get_int32();
    bool minimized = scan.get_bool();
    bool maximized = scan.get_bool();
    if (thread_index != -1) {
      graph = open_strip_chart(thread_index, collector_index, show_level);
      graph->set_window_state(x, y, width, height, minimized, maximized);
    }
  }

  size_t num_flame_graphs = scan.get_uint16();
  for (size_t i = 0; i < num_flame_graphs; ++i) {
    std::string thread_name = scan.get_string();
    std::string collector_name = scan.get_string();
    int thread_index = _client_data->find_thread(thread_name);
    int collector_index = collector_name.empty() ? -1 : _client_data->find_collector(collector_name);
    int x = scan.get_int32();
    int y = scan.get_int32();
    int width = scan.get_int32();
    int height = scan.get_int32();
    bool minimized = scan.get_bool();
    bool maximized = scan.get_bool();
    if (thread_index != -1) {
      graph = open_flame_graph(thread_index, collector_index);
      graph->set_window_state(x, y, width, height, minimized, maximized);
    }
  }

  size_t num_piano_rolls = scan.get_uint16();
  for (size_t i = 0; i < num_piano_rolls; ++i) {
    std::string thread_name = scan.get_string();
    int thread_index = _client_data->find_thread(thread_name);
    int x = scan.get_int32();
    int y = scan.get_int32();
    int width = scan.get_int32();
    int height = scan.get_int32();
    bool minimized = scan.get_bool();
    bool maximized = scan.get_bool();
    if (thread_index != -1) {
      graph = open_piano_roll(thread_index);
      graph->set_window_state(x, y, width, height, minimized, maximized);
    }
  }

  dif.close();
  if (num_timelines + num_strip_charts + num_flame_graphs + num_piano_rolls == 0) {
    open_strip_chart(0, 0, false);
  }
}

/**
 * Saves the current graph layout as the default graph layout.
 */
bool PStatMonitor::
save_default_graphs() const {
  layout_filename.make_dir();

  DatagramOutputFile dof;
  if (!dof.open(layout_filename)) {
    return false;
  }
  if (!dof.write_header(layout_file_header)) {
    return false;
  }
  Datagram dg;
  dg.set_stdfloat_double(false);
  dg.add_uint16(1);
  dg.add_uint16(1);

  dg.add_uint32((uint32_t)_colors.size());
  for (const auto &item : _colors) {
    dg.add_int32(item.first);
    dg.add_float32(item.second[0]);
    dg.add_float32(item.second[1]);
    dg.add_float32(item.second[2]);
  }

  dg.add_uint16(_timelines.size());
  for (PStatGraph *graph : _timelines) {
    int x, y, width, height;
    bool minimized, maximized;
    if (graph->get_window_state(x, y, width, height, minimized, maximized)) {
      dg.add_int32(x);
      dg.add_int32(y);
      dg.add_int32(width);
      dg.add_int32(height);
      dg.add_bool(minimized);
      dg.add_bool(maximized);
    }
  }

  dg.add_uint16(_strip_charts.size());
  for (PStatGraph *graph : _strip_charts) {
    int x, y, width, height;
    bool minimized, maximized;
    if (graph->get_window_state(x, y, width, height, minimized, maximized)) {
      dg.add_string(_client_data->get_thread_name(((PStatStripChart *)graph)->get_thread_index()));
      dg.add_string(_client_data->get_collector_fullname(((PStatStripChart *)graph)->get_collector_index()));
      dg.add_bool(((PStatStripChart *)graph)->get_view().get_show_level());
      dg.add_int32(x);
      dg.add_int32(y);
      dg.add_int32(width);
      dg.add_int32(height);
      dg.add_bool(minimized);
      dg.add_bool(maximized);
    }
  }

  dg.add_uint16(_flame_graphs.size());
  for (PStatGraph *graph : _flame_graphs) {
    int x, y, width, height;
    bool minimized, maximized;
    if (graph->get_window_state(x, y, width, height, minimized, maximized)) {
      int collector_index = ((PStatFlameGraph *)graph)->get_collector_index();
      dg.add_string(_client_data->get_thread_name(((PStatFlameGraph *)graph)->get_thread_index()));
      dg.add_string(collector_index >= 0 ? _client_data->get_collector_fullname(collector_index) : "");
      dg.add_int32(x);
      dg.add_int32(y);
      dg.add_int32(width);
      dg.add_int32(height);
      dg.add_bool(minimized);
      dg.add_bool(maximized);
    }
  }

  dg.add_uint16(_piano_rolls.size());
  for (PStatGraph *graph : _piano_rolls) {
    int x, y, width, height;
    bool minimized, maximized;
    if (graph->get_window_state(x, y, width, height, minimized, maximized)) {
      dg.add_string(_client_data->get_thread_name(((PStatPianoRoll *)graph)->get_thread_index()));
      dg.add_int32(x);
      dg.add_int32(y);
      dg.add_int32(width);
      dg.add_int32(height);
      dg.add_bool(minimized);
      dg.add_bool(maximized);
    }
  }

  // Reserved for future graph type
  dg.add_uint16(0);

  dof.put_datagram(dg);
  dof.close();
  return true;
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
 * Sets a custom color associated with the given collector.
 */
void PStatMonitor::
set_collector_color(int collector_index, const LRGBColor &color) {
  _colors[collector_index] = color;
}

/**
 * Clears any custom custom color associated with the given collector.
 */
void PStatMonitor::
clear_collector_color(int collector_index) {
  _colors.erase(collector_index);
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
new_data(int thread_index, int frame_number) {
  const PStatClientData *client_data = get_client_data();

  // Don't bother to update the thread data until we know at least something
  // about the collectors and threads.
  if (client_data->get_num_collectors() != 0 &&
      client_data->get_num_threads() != 0) {
    PStatView &view = get_view(thread_index);
    const PStatThreadData *thread_data = view.get_thread_data();
    if (!thread_data->is_empty()) {
      int latest = thread_data->get_latest_frame_number();
      if (frame_number == latest) {
        view.set_to_frame(thread_data->get_frame(frame_number));
      }
    }
  }
}

/**
 * Called when a thread should be removed from the list of threads.
 */
void PStatMonitor::
remove_thread(int) {
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

/**
 * Opens a new timeline.
 */
PStatGraph *PStatMonitor::
open_timeline() {
  return nullptr;
}

/**
 * Opens a new strip chart showing the indicated data.
 */
PStatGraph *PStatMonitor::
open_strip_chart(int thread_index, int collector_index, bool show_level) {
  return nullptr;
}

/**
 * Opens a new flame graph showing the indicated data.
 */
PStatGraph *PStatMonitor::
open_flame_graph(int thread_index, int collector_index, int frame_number) {
  return nullptr;
}

/**
 * Opens a new piano roll showing the indicated data.
 */
PStatGraph *PStatMonitor::
open_piano_roll(int thread_index) {
  return nullptr;
}

/**
 * Writes the client data and open graphs to a datagram.
 */
void PStatMonitor::
write_datagram(Datagram &dg) const {
  dg.add_bool(_client_known);
  dg.add_string(_client_hostname);
  dg.add_string(_client_progname);
  dg.add_int32(_client_pid);

  get_client_data()->write_datagram(dg);

  dg.add_uint32((uint32_t)_colors.size());
  for (const auto &item : _colors) {
    dg.add_int32(item.first);
    dg.add_float32(item.second[0]);
    dg.add_float32(item.second[1]);
    dg.add_float32(item.second[2]);
  }

  dg.add_uint16(_timelines.size());
  for (PStatGraph *graph : _timelines) {
    graph->write_datagram(dg);
  }

  dg.add_uint16(_strip_charts.size());
  for (PStatGraph *graph : _strip_charts) {
    dg.add_int16(((PStatStripChart *)graph)->get_thread_index());
    dg.add_int16(((PStatStripChart *)graph)->get_collector_index());
    dg.add_bool(((PStatStripChart *)graph)->get_view().get_show_level());
    graph->write_datagram(dg);
  }

  dg.add_uint16(_flame_graphs.size());
  for (PStatGraph *graph : _flame_graphs) {
    dg.add_int16(((PStatFlameGraph *)graph)->get_thread_index());
    dg.add_int16(((PStatFlameGraph *)graph)->get_collector_index());
    graph->write_datagram(dg);
  }

  dg.add_uint16(_piano_rolls.size());
  for (PStatGraph *graph : _piano_rolls) {
    dg.add_int16(((PStatPianoRoll *)graph)->get_thread_index());
    graph->write_datagram(dg);
  }

  // Reserved for future graph type
  dg.add_uint16(0);
}

/**
 * Restores the client data and open graphs from a datagram.
 */
void PStatMonitor::
read_datagram(DatagramIterator &scan) {
  _client_known = scan.get_bool();
  _client_hostname = scan.get_string();
  _client_progname = scan.get_string();
  _client_pid = scan.get_int32();

  PStatClientData *client_data = new PStatClientData;
  client_data->read_datagram(scan);
  set_client_data(client_data);

  size_t num_colors = scan.get_uint32();
  for (size_t i = 0; i < num_colors; ++i) {
    int key = scan.get_int32();
    LRGBColor &color = _colors[key];
    color[0] = scan.get_float32();
    color[1] = scan.get_float32();
    color[2] = scan.get_float32();
  }

  int num_collectors = client_data->get_num_collectors();
  for (int collector_index = 0; collector_index < num_collectors; ++collector_index) {
    if (client_data->has_collector(collector_index)) {
      new_collector(collector_index);
    }
  }

  int num_threads = client_data->get_num_threads();
  for (int thread_index = 0; thread_index < num_threads; ++thread_index) {
    if (client_data->has_thread(thread_index)) {
      const PStatThreadData *thread_data = client_data->get_thread_data(thread_index);
      if (!thread_data->is_empty()) {
        const PStatFrameData &frame_data = thread_data->get_latest_frame();
        get_view(thread_index).set_to_frame(frame_data);

        int num_collectors = client_data->get_num_toplevel_collectors();
        for (int i = 0; i < num_collectors; ++i) {
          int collector_index = client_data->get_toplevel_collector(i);
          if (client_data->has_collector(collector_index)) {
            get_level_view(collector_index, thread_index).set_to_frame(thread_data->get_latest_frame());
          }
        }
      }
      new_thread(thread_index);
    }
  }

  PStatGraph *graph;

  size_t num_timelines = scan.get_uint16();
  for (size_t i = 0; i < num_timelines; ++i) {
    graph = open_timeline();
    graph->read_datagram(scan);
  }

  size_t num_strip_charts = scan.get_uint16();
  for (size_t i = 0; i < num_strip_charts; ++i) {
    int thread_index = scan.get_int16();
    int collector_index = scan.get_int16();
    graph = open_strip_chart(thread_index, collector_index, scan.get_bool());
    graph->read_datagram(scan);
  }

  size_t num_flame_graphs = scan.get_uint16();
  for (size_t i = 0; i < num_flame_graphs; ++i) {
    int thread_index = scan.get_int16();
    int collector_index = scan.get_int16();
    graph = open_flame_graph(thread_index, collector_index);
    graph->read_datagram(scan);
  }

  size_t num_piano_rolls = scan.get_uint16();
  for (size_t i = 0; i < num_piano_rolls; ++i) {
    int thread_index = scan.get_int16();
    graph = open_piano_roll(thread_index);
    graph->read_datagram(scan);
  }
}
