/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatMonitor.I
 * @author drose
 * @date 2000-07-14
 */

/**
 * Returns the server that owns this monitor.
 */
INLINE PStatServer *PStatMonitor::
get_server() {
  return _server;
}

/**
 * Returns the client data associated with this monitor.
 */
INLINE const PStatClientData *PStatMonitor::
get_client_data() const {
  return _client_data;
}

/**
 * Returns the name of the indicated collector, if it is known.
 */
INLINE std::string PStatMonitor::
get_collector_name(int collector_index) {
  if (!_client_data.is_null() &&
      _client_data->has_collector(collector_index)) {
    return _client_data->get_collector_name(collector_index);
  }
  return "Unknown";
}

/**
 * Returns true if we've yet received the "hello" message from the client
 * indicating its name, etc.
 */
INLINE bool PStatMonitor::
is_client_known() const {
  return _client_known;
}

/**
 * Returns the hostname of the client we're connected to, if known.  This may
 * not be known immediately at creation time, but should be learned shortly
 * thereafter when we receive the client's "hello" message.  See
 * is_client_known().
 */
INLINE std::string PStatMonitor::
get_client_hostname() const {
  return _client_hostname;
}

/**
 * Returns the program name of the client we're connected to, if known.  This
 * may not be known immediately at creation time, but should be learned
 * shortly thereafter when we receive the client's "hello" message.  See
 * is_client_known().
 */
INLINE std::string PStatMonitor::
get_client_progname() const {
  return _client_progname;
}

/**
 * Returns the process id of the client, or -1 if it is not known.
 */
INLINE int PStatMonitor::
get_client_pid() const {
  return _client_pid;
}

/**
 * Returns true if the session data was loaded from a file.
 */
INLINE bool PStatMonitor::
has_read_filename() const {
  return !_read_filename.empty();
}

/**
 * Returns the filename this session data was loaded from.
 */
INLINE const Filename &PStatMonitor::
get_read_filename() const {
  return _read_filename;
}
