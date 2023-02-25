/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClientData.cxx
 * @author drose
 * @date 2000-07-11
 */

#include "pStatClientData.h"
#include "pStatFrameData.h"
#include "pStatReader.h"

#include "pStatCollectorDef.h"

using std::string;

PStatCollectorDef PStatClientData::_null_collector(-1, "Unknown");

/**
 *
 */
PStatClientData::
PStatClientData(PStatReader *reader) :
  _is_alive(true),
  _is_dirty(false),
  _reader(reader)
{
}

/**
 *
 */
PStatClientData::
~PStatClientData() {
  for (Collector &collector : _collectors) {
    delete collector._def;
  }
}

/**
 * Clears the is_dirty() flag.
 */
void PStatClientData::
clear_dirty() const {
  _is_dirty = false;
}

/**
 * Returns true if the data was modified since the last time clear_dirty() was
 * called.
 */
bool PStatClientData::
is_dirty() const {
  return _is_dirty;
}

/**
 * Returns true if the data is actively getting filled by a connected client,
 * or false if the client has terminated.
 */
bool PStatClientData::
is_alive() const {
  return _is_alive;
}

/**
 * Closes the client connection if it is open.
 */
void PStatClientData::
close() {
  if (_is_alive && _reader != nullptr) {
    _reader->close();
    _reader = nullptr;
    _is_alive = false;
  }
}

/**
 * Returns the timestamp (in seconds elapsed since connection) of the latest
 * available frame.
 */
double PStatClientData::
get_latest_time() const {
  double time = 0.0;
  for (const Thread &thread : _threads) {
    if (thread._data != nullptr && !thread._data->is_empty()) {
      time = std::max(time, thread._data->get_latest_time());
    }
  }

  return time;
}

/**
 * Returns the total number of collectors the Data knows about.
 */
int PStatClientData::
get_num_collectors() const {
  return _collectors.size();
}

/**
 * Returns true if the indicated collector has been defined by the client
 * already, false otherwise.  It is possible for the client to start streaming
 * data before all of the collectors have been defined.
 */
bool PStatClientData::
has_collector(int index) const {
  return (index >= 0 && index < (int)_collectors.size() &&
          _collectors[index]._def != nullptr);
}

/**
 * Returns the index of the collector with the given full name, or -1 if no
 * such collector has been defined by the client.
 */
int PStatClientData::
find_collector(const std::string &fullname) const {
  // Take the last bit, we can compare it more cheaply, only check the full
  // name if the basename matches.
  const char *colon = strrchr(fullname.c_str(), ':');
  std::string name(colon != nullptr ? colon + 1 : fullname.c_str());

  for (int index = 0; index < get_num_collectors(); ++index) {
    const PStatCollectorDef *def = _collectors[index]._def;
    if (def != nullptr && def->_name == name &&
        get_collector_fullname(index) == fullname) {
      return index;
    }
  }

  return -1;
}

/**
 * Returns the nth collector definition.
 */
const PStatCollectorDef &PStatClientData::
get_collector_def(int index) const {
  if (!has_collector(index)) {
    return _null_collector;
  }
  return *_collectors[index]._def;
}

/**
 * Returns the name of the indicated collector.
 */
string PStatClientData::
get_collector_name(int index) const {
  if (!has_collector(index)) {
    return "Unknown";
  }
  const PStatCollectorDef *def = _collectors[index]._def;
  return def->_name;
}

/**
 * Returns the "full name" of the indicated collector.  This will be the
 * concatenation of all of the collector's parents' names (except Frame) and
 * the collector's own name.
 */
string PStatClientData::
get_collector_fullname(int index) const {
  if (!has_collector(index)) {
    return "Unknown";
  }

  const PStatCollectorDef *def = _collectors[index]._def;
  if (def->_parent_index == 0) {
    return def->_name;
  } else {
    return get_collector_fullname(def->_parent_index) + ":" + def->_name;
  }
}

/**
 * Indicates whether the given collector has level data (and consequently,
 * whether it should appear on the Levels menu).
 *
 * The return value is true if anything changed, false otherwise.
 */
bool PStatClientData::
set_collector_has_level(int index, int thread_index, bool flag) {
  bool any_changed = false;
  slot_collector(index);
  nassertr(index >= 0 && index < (int)_collectors.size(), false);

  if (_collectors[index]._is_level.get_bit(thread_index) != flag) {
    any_changed = true;
    _collectors[index]._is_level.set_bit_to(thread_index, flag);
  }

  // Turning this on for a given collector also implicitly turns all of its
  // ancestors.
  if (flag) {
    PStatCollectorDef *def = _collectors[index]._def;
    if (def != nullptr && def->_parent_index != 0) {
      if (set_collector_has_level(def->_parent_index, thread_index, flag)) {
        any_changed = true;
      }
    }
  }

  if (any_changed) {
    _is_dirty = true;
  }

  return any_changed;
}


/**
 * Returns whether the given collector has level data (and consequently,
 * whether it should appear on the Levels menu).
 */
bool PStatClientData::
get_collector_has_level(int index, int thread_index) const {
  return (index >= 0 && index < (int)_collectors.size() &&
          _collectors[index]._is_level.get_bit(thread_index));
}

/**
 * Returns the total number of collectors that are toplevel collectors.  These
 * are the collectors that are the children of "Frame", which is collector 0.
 */
int PStatClientData::
get_num_toplevel_collectors() const {
  return _toplevel_collectors.size();
}

/**
 * Returns the collector index of the nth toplevel collector.  Use this
 * function to iterate through the n toplevel collectors indicated by
 * get_num_toplevel_collectors().
 */
int PStatClientData::
get_toplevel_collector(int n) const {
  nassertr(n >= 0 && n < (int)_toplevel_collectors.size(), 0);
  return _toplevel_collectors[n];
}

/**
 * Returns the total number of threads the Data knows about.
 */
int PStatClientData::
get_num_threads() const {
  return _threads.size();
}

/**
 * Returns true if the indicated thread has been defined by the client
 * already, false otherwise.  It is possible for the client to start streaming
 * data before all of the threads have been defined.
 */
bool PStatClientData::
has_thread(int index) const {
  return (index >= 0 && (size_t)index < _threads.size() &&
          !_threads[index]._name.empty());
}

/**
 * Returns the index of the thread with the given name, or -1 if no such thread
 * has yet been defined.
 */
int PStatClientData::
find_thread(const std::string &name) const {
  for (int index = 0; index < get_num_threads(); ++index) {
    if (_threads[index]._name == name) {
      return index;
    }
  }

  return -1;
}

/**
 * Returns the name of the indicated thread.
 */
string PStatClientData::
get_thread_name(int index) const {
  if (!has_thread(index)) {
    return "Unknown";
  }
  return _threads[index]._name;
}

/**
 * Returns the data associated with the indicated thread.  This will create a
 * thread definition if it does not already exist.
 */
const PStatThreadData *PStatClientData::
get_thread_data(int index) const {
  ((PStatClientData *)this)->define_thread(index);
  nassertr(index >= 0 && (size_t)index < _threads.size(), nullptr);
  return _threads[index]._data;
}

/**
 * Returns true if the given thread is still alive.
 */
bool PStatClientData::
is_thread_alive(int index) const {
  return (index >= 0 && (size_t)index < _threads.size() && _threads[index]._is_alive);
}

/**
 * Returns the number of Collectors between the indicated parent and the child
 * Collector in the relationship graph.  If child is the same as parent,
 * returns zero.  If child is an immediate child of parent, returns 1.  If
 * child is a grandchild of parent, returns 2, and so on.  If child is not a
 * descendant of parent at all, returns -1.
 */
int PStatClientData::
get_child_distance(int parent, int child) const {
  if (parent == child) {
    return 0;
  }
  if (!has_collector(child) || child == 0) {
    return -1;
  }
  int dist = get_child_distance(parent, get_collector_def(child)._parent_index);
  if (dist == -1) {
    return -1;
  } else {
    return dist + 1;
  }
}

/**
 * Adds a new collector definition to the dataset.  Presumably this is
 * information just arrived from the client.
 *
 * The pointer will become owned by the PStatClientData object and will be
 * freed on destruction.
 */
void PStatClientData::
add_collector(PStatCollectorDef *def) {
  slot_collector(def->_index);
  nassertv(def->_index >= 0 && def->_index < (int)_collectors.size());

  if (_collectors[def->_index]._def != nullptr) {
    // Free the old definition, if any.
    delete _collectors[def->_index]._def;
  }

  _collectors[def->_index]._def = def;
  update_toplevel_collectors();

  // If we already had the _is_level flag set, it should be immediately
  // applied to all ancestors.
  const BitArray &is_level = _collectors[def->_index]._is_level;
  int max_threads = is_level.get_num_bits();
  for (int thread_index = 0; thread_index < max_threads; ++thread_index) {
    if (is_level.get_bit(thread_index)) {
      set_collector_has_level(def->_parent_index, thread_index, true);
    }
  }

  _is_dirty = true;
}

/**
 * Adds a new thread definition to the dataset.  Presumably this is
 * information just arrived from the client.
 */
void PStatClientData::
define_thread(int thread_index, const string &name, bool mark_alive) {
  // A sanity check on the index number.
  nassertv(thread_index < 1000);

  // Make sure we have enough slots allocated.
  while ((int)_threads.size() <= thread_index) {
    _threads.push_back(Thread());
  }

  if (mark_alive) {
    _threads[thread_index]._is_alive = true;
  }

  if (!name.empty()) {
    _threads[thread_index]._name = name;
  }

  if (_threads[thread_index]._data.is_null()) {
    _threads[thread_index]._data = new PStatThreadData(this);
  }

  _is_dirty = true;
}

/**
 * Indicates that the given thread has expired.  Presumably this is information
 * just arrived from the client.
 */
void PStatClientData::
expire_thread(int thread_index) {
  if (thread_index >= 0 && (size_t)thread_index < _threads.size()) {
    _threads[thread_index]._is_alive = false;
  }
}

/**
 * Removes the given thread data entirely.
 */
void PStatClientData::
remove_thread(int thread_index) {
  if (thread_index >= 0 && (size_t)thread_index < _threads.size()) {
    _threads[thread_index]._name.clear();
    _threads[thread_index]._data.clear();
    _threads[thread_index]._is_alive = false;
  }
}

/**
 * Makes room for and stores a new frame's worth of data associated with some
 * particular thread (which may or may not have already been defined).
 *
 * The pointer will become owned by the PStatThreadData object and will be
 * freed on destruction.
 */
void PStatClientData::
record_new_frame(int thread_index, int frame_number,
                 PStatFrameData *frame_data) {
  define_thread(thread_index);
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
  _threads[thread_index]._data->record_new_frame(frame_number, frame_data);
  _is_dirty = true;
}

/**
 * Writes the client data in the form of a JSON output that can be loaded into
 * Chrome's event tracer.
 */
void PStatClientData::
write_json(std::ostream &out, int pid) const {
  out << "[\n";

  for (int thread_index = 0; thread_index < get_num_threads(); ++thread_index) {
    const Thread &thread = _threads[thread_index];

    if (thread_index == 0) {
      out << "{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":" << pid
          << ",\"tid\":0,\"args\":{\"name\":\"Main\"}}";
    }
    else if (!thread._name.empty()) {
      out
        << ",\n{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":" << pid
        << ",\"tid\":" << thread_index << ",\"args\":{\"name\":\""
        << thread._name << "\"}}";
    }
    if (thread._data == nullptr || thread._data->is_empty()) {
      continue;
    }

    int first_frame = thread._data->get_oldest_frame_number();
    int last_frame = thread._data->get_latest_frame_number();
    for (int frame_number = first_frame; frame_number <= last_frame; ++frame_number) {
      const PStatFrameData &frame_data = thread._data->get_frame(frame_number);
      size_t num_events = frame_data.get_num_events();
      for (size_t i = 0; i < num_events; ++i) {
        int collector_index = frame_data.get_time_collector(i);
        out
          << ",\n{\"name\":\"" << get_collector_fullname(collector_index)
          << "\",\"ts\":" << (uint64_t)(frame_data.get_time(i) * 1000000)
          << ",\"ph\":\"" << (frame_data.is_start(i) ? 'B' : 'E') << "\""
          << ",\"pid\":" << pid << ",\"tid\":" << thread_index << "}";
      }
    }
  }

  out << "\n]\n";
  out.flush();
}

/**
 * Writes the client data to a datagram.
 */
void PStatClientData::
write_datagram(Datagram &dg) const {
  for (const Collector &collector : _collectors) {
    PStatCollectorDef *def = collector._def;
    if (def != nullptr && def->_index != -1) {
      def->write_datagram(dg);
      collector._is_level.write_datagram(nullptr, dg);
    }
  }
  dg.add_int16(-1);

  int thread_index = 0;
  for (const Thread &thread : _threads) {
    if (thread._data != nullptr) {
      dg.add_int16(thread_index);
      dg.add_string(thread._name);
      thread._data->write_datagram(dg);
    }
    ++thread_index;
  }
  dg.add_int16(-1);
}

/**
 * Restores the client data from a datagram.
 */
void PStatClientData::
read_datagram(DatagramIterator &scan) {
  while (scan.peek_int16() != -1) {
    PStatCollectorDef *def = new PStatCollectorDef;
    def->read_datagram(scan);
    add_collector(def);
    _collectors[def->_index]._is_level.read_datagram(scan, nullptr);
  }
  scan.skip_bytes(2);

  int thread_index;
  while ((thread_index = scan.get_int16()) != -1) {
    std::string name = scan.get_string();
    define_thread(thread_index, name, true);

    _threads[thread_index]._data->read_datagram(scan, this);
  }

  update_toplevel_collectors();
}

/**
 * Makes sure there is an entry in the array for a collector with the given
 * index number.
 */
void PStatClientData::
slot_collector(int collector_index) {
  // A sanity check on the index number.
  nassertv(collector_index < 100000);

  while ((int)_collectors.size() <= collector_index) {
    Collector collector;
    collector._def = nullptr;
    _collectors.push_back(collector);
  }
}

/**
 * Rebuilds the list of toplevel collectors.
 */
void PStatClientData::
update_toplevel_collectors() {
  _toplevel_collectors.clear();

  for (Collector &collector : _collectors) {
    PStatCollectorDef *def = collector._def;
    if (def != nullptr && def->_parent_index == 0) {
      _toplevel_collectors.push_back(def->_index);
    }
  }
}
