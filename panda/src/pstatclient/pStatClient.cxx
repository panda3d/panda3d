// Filename: pStatClient.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "pStatClient.h"

#ifdef DO_PSTATS 
// This file only defines anything interesting if DO_PSTATS is
// defined.

#include "pStatClientControlMessage.h"
#include "pStatServerControlMessage.h"
#include "pStatCollector.h"
#include "pStatThread.h"
#include "config_pstats.h"

#include <algorithm>

#ifdef WIN32_VC
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#endif

PStatClient *PStatClient::_global_pstats = NULL;

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatClient::
PStatClient() :
  _reader(this, 0),
  _writer(this, 0)
{
  _is_connected = false;
  _got_udp_port = false;
  _collectors_reported = 0;
  _threads_reported = 0;

  // We always have a collector at index 0 named "Frame".  This tracks
  // the total frame time and is the root of all other collectors.  We
  // have to make this one by hand since it's the root.
  Collector collector;
  collector._def = new PStatCollectorDef(0, "Frame");
  collector._def->_parent_index = 0;
  collector._def->_suggested_color.set(0.5, 0.5, 0.5);
  _collectors.push_back(collector);

  // We also always have a thread at index 0 named "Main".
  make_thread("Main");

  _client_name = get_pstats_name();
  _max_rate = get_pstats_max_rate();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PStatClient::
~PStatClient() {
  disconnect();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_collectors
//       Access: Public
//  Description: Returns the total number of collectors the Client
//               knows about.
////////////////////////////////////////////////////////////////////
int PStatClient::
get_num_collectors() const {
  return _collectors.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector
//       Access: Public
//  Description: Returns the nth collector.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
get_collector(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), PStatCollector());
  return PStatCollector((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_def
//       Access: Public
//  Description: Returns the definition body of the nth collector.
////////////////////////////////////////////////////////////////////
const PStatCollectorDef &PStatClient::
get_collector_def(int index) const {
#ifndef NDEBUG
  static PStatCollectorDef bogus;
  nassertr(index >= 0 && index < (int)_collectors.size(), bogus);
#endif

  return *_collectors[index]._def;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_name
//       Access: Public
//  Description: Returns the name of the indicated collector.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_name(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  const PStatCollectorDef *def = _collectors[index]._def;
  return def->_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_fullname
//       Access: Public
//  Description: Returns the "full name" of the indicated collector.
//               This will be the concatenation of all of the
//               collector's parents' names (except Frame) and the
//               collector's own name.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_collector_fullname(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), string());

  const PStatCollectorDef *def = _collectors[index]._def;
  if (def->_parent_index == 0) {
    return def->_name;
  } else {
    return get_collector_fullname(def->_parent_index) + ":" + def->_name;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_threads
//       Access: Public
//  Description: Returns the total number of threads the Client
//               knows about.
////////////////////////////////////////////////////////////////////
int PStatClient::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread
//       Access: Public
//  Description: Returns the nth thread.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_thread(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), PStatThread());
  return PStatThread((PStatClient *)this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread_name
//       Access: Public
//  Description: Returns the name of the indicated thread.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_thread_name(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), string());
  return _threads[index]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_clock
//       Access: Public
//  Description: Returns a reference to the PStatClient's clock
//               object.  It keeps its own clock, instead of using the
//               global clock object, so the stats won't get mucked up
//               if you put the global clock in non-real-time mode or
//               something.
////////////////////////////////////////////////////////////////////
const ClockObject &PStatClient::
get_clock() const {
  return _clock;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_main_thread
//       Access: Public
//  Description: Returns a handle to the client's "Main", or default,
//               thread.  This is where collectors will be started and
//               stopped if they don't specify otherwise.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
get_main_thread() const {
  return PStatThread((PStatClient *)this, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_collector
//       Access: Private
//  Description: Returns a PStatCollector suitable for measuring
//               categories with the indicated name.  This is normally
//               called by a PStatCollector constructor.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
make_collector(int parent_index, string fullname) {
  if (fullname.empty()) {
    fullname = "Unnamed";
  }

  // Skip any colons at the beginning of the name.
  size_t start = 0;
  while (start < fullname.size() && fullname[start] == ':') {
    start++;
  }
  
  // If the name contains a colon (after the initial colon), it means
  // we are making a nested collector.
  size_t colon = fullname.find(':', start);
  if (colon != string::npos) {
    string parent_name = fullname.substr(start, colon - start);
    PStatCollector parent_collector = 
      make_collector(parent_index, parent_name);
    return make_collector(parent_collector._index, fullname.substr(colon + 1));
  }

  string name = fullname.substr(start);

  nassertr(parent_index >= 0 && parent_index < (int)_collectors.size(),
	   PStatCollector());

  Collector &parent = _collectors[parent_index];

  // A special case: if we asked for a child the same name as its
  // parent, we really meant the parent.  That is, "Frame:Frame" is
  // really the same collector as "Frame".
  if (parent._def->_name == name) {
    return PStatCollector(this, parent_index);
  }

  ThingsByName::const_iterator ni = parent._children.find(name);

  if (ni != parent._children.end()) {
    // We already had a collector by this name; return it.
    int index = (*ni).second;
    nassertr(index >= 0 && index < (int)_collectors.size(), PStatCollector());
    return PStatCollector(this, (*ni).second);
  }

  // Create a new collector for this name.
  int new_index = _collectors.size();
  parent._children.insert(ThingsByName::value_type(name, new_index));

  Collector collector;
  collector._def = new PStatCollectorDef(new_index, name);
  collector._def->_parent_index = parent_index;

  // We need one nested_count for each thread.
  while (collector._nested_count.size() < _threads.size()) {
    collector._nested_count.push_back(0);
  }

  _collectors.push_back(collector);

  return PStatCollector(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_collector
//       Access: Private
//  Description: This flavor of make_collector will make a new
//               collector and automatically set up some of its
//               properties.
////////////////////////////////////////////////////////////////////
PStatCollector PStatClient::
make_collector(int parent_index, const string &fullname,
	       const RGBColorf &suggested_color, int sort) {
  PStatCollector c = make_collector(parent_index, fullname);
  nassertr(c._client == this, PStatCollector());
  nassertr(c._index >= 0 && c._index < (int)_collectors.size(), PStatCollector());

  PStatCollectorDef *def = _collectors[c._index]._def;
  nassertr(def != (PStatCollectorDef *)NULL, PStatCollector());

  if (suggested_color != RGBColorf::zero() &&
      def->_suggested_color != suggested_color) {
    // We need to change the suggested color.
    def->_suggested_color = suggested_color;
    _collectors_reported = min(_collectors_reported, c._index);
  }

  if (sort != -1 && def->_sort != sort) {
    // We need to change the sort.
    def->_sort = sort;
    _collectors_reported = min(_collectors_reported, c._index);
  }

  return c;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::make_thread
//       Access: Private
//  Description: Returns a PStatThread with the indicated name
//               suitable for grouping collectors.  This is normally
//               called by a PStatThread constructor.
////////////////////////////////////////////////////////////////////
PStatThread PStatClient::
make_thread(const string &name) {
  ThingsByName::const_iterator ni = 
    _threads_by_name.find(name);

  if (ni != _threads_by_name.end()) {
    // We already had a thread by this name; return it.
    int index = (*ni).second;
    nassertr(index >= 0 && index < (int)_threads.size(), PStatThread());
    return PStatThread(this, (*ni).second);
  }

  // Create a new thread for this name.
  int new_index = _threads.size();
  _threads_by_name.insert(ThingsByName::value_type(name, new_index));

  Thread thread;
  thread._name = name;
  thread._is_active = false;
  thread._last_packet = 0.0;
  thread._frame_number = 0;

  _threads.push_back(thread);

  // We need an additional nested_count for this thread in all of the
  // collectors.
  Collectors::iterator ci;
  for (ci = _collectors.begin(); ci != _collectors.end(); ++ci) {
    (*ci)._nested_count.push_back(0);
    nassertr((*ci)._nested_count.size() == _threads.size(), PStatThread());
  }

  return PStatThread(this, new_index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::main_tick
//       Access: Public, Static
//  Description: A convenience function to call new_frame() on the
//               global PStatClient's main thread.
////////////////////////////////////////////////////////////////////
void PStatClient::
main_tick() {
  get_global_pstats()->get_main_thread().new_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_global_pstats
//       Access: Public, Static
//  Description: Returns a pointer to the global PStatClient object.
//               It's legal to declare your own PStatClient locally,
//               but it's also convenient to have a global one that
//               everyone can register with.  This is the global one.
////////////////////////////////////////////////////////////////////
PStatClient *PStatClient::
get_global_pstats() {
  if (_global_pstats == (PStatClient *)NULL) {
    _global_pstats = new PStatClient;
  }
  return _global_pstats;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::ns_connect
//       Access: Private
//  Description: The nonstatic implementation of connect().
////////////////////////////////////////////////////////////////////
bool PStatClient::
ns_connect(string hostname, int port) {
  ns_disconnect();

  if (hostname.empty()) {
    hostname = pstats_host;
  }
  if (port < 0) {
    port = pstats_port;
  }

  if (!_server.set_host(hostname, port)) {
    pstats_cat.error()
      << "Unknown host: " << hostname << "\n";
    return false;
  }

  _tcp_connection = open_TCP_client_connection(_server, 5000);

  if (_tcp_connection.is_null()) {
    pstats_cat.error()
      << "Couldn't connect to PStatServer at " << hostname << ":" 
      << port << "\n";
    return false;
  }

  _reader.add_connection(_tcp_connection);
  _is_connected = true;

  _udp_connection = open_UDP_connection();

  send_hello();

  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::ns_disconnect
//       Access: Private
//  Description: The nonstatic implementation of disconnect().
////////////////////////////////////////////////////////////////////
void PStatClient::
ns_disconnect() {
  if (_is_connected) {
    _reader.remove_connection(_tcp_connection);
    close_connection(_tcp_connection);
    close_connection(_udp_connection);
  }

  _tcp_connection.clear();
  _udp_connection.clear();

  _is_connected = false;
  _got_udp_port = false;

  _collectors_reported = 0;
  _threads_reported = 0;

  Threads::iterator ti;
  for (ti = _threads.begin(); ti != _threads.end(); ++ti) {
    (*ti)._frame_number = 0;
    (*ti)._is_active = false;
    (*ti)._last_packet = 0.0;
    (*ti)._frame_data.clear();
  }
  
  Collectors::iterator ci;
  for (ci = _collectors.begin(); ci != _collectors.end(); ++ci) {
    vector_int::iterator ii;
    for (ii = (*ci)._nested_count.begin(); 
	   ii != (*ci)._nested_count.end(); 
	 ++ii) {
      (*ii) = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::ns_is_connected
//       Access: Public
//  Description: The nonstatic implementation of is_connected().
////////////////////////////////////////////////////////////////////
bool PStatClient::
ns_is_connected() const {
  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start_collector
//       Access: Private
//  Description: Marks the indicated collector index as started.
//               Normally you would not use this interface directly;
//               instead, call pStatCollector::start().
////////////////////////////////////////////////////////////////////
void PStatClient::
start(int collector_index, int thread_index, double as_of) {
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());

  if (_threads[thread_index]._is_active) {
    if (_collectors[collector_index]._nested_count[thread_index] == 0) {
      // This collector wasn't already started in this thread; record
      // a new data point.
      _threads[thread_index]._frame_data.add_start(collector_index, as_of);
    }
    _collectors[collector_index]._nested_count[thread_index]++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::start_collector
//       Access: Private
//  Description: Marks the indicated collector index as stopped.
//               Normally you would not use this interface directly;
//               instead, call pStatCollector::stop().
////////////////////////////////////////////////////////////////////
void PStatClient::
stop(int collector_index, int thread_index, double as_of) {
  nassertv(collector_index >= 0 && collector_index < (int)_collectors.size());
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());

  if (_threads[thread_index]._is_active) {
    if (_collectors[collector_index]._nested_count[thread_index] == 0) {
      pstats_cat.warning()
	<< "Collector " << get_collector_fullname(collector_index)
	<< " was already stopped in thread " << get_thread_name(thread_index)
	<< "!\n";
      return;
    }
    
    _collectors[collector_index]._nested_count[thread_index]--;
    
    if (_collectors[collector_index]._nested_count[thread_index] == 0) {
      // This collector has now been completely stopped; record a new
      // data point.
      _threads[thread_index]._frame_data.add_stop(collector_index, as_of);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::new_frame
//       Access: Private
//  Description: Called by the PStatThread interface at the beginning
//               of every frame, for each thread.  This resets the
//               clocks for the new frame and transmits the data for
//               the previous frame.
////////////////////////////////////////////////////////////////////
void PStatClient::
new_frame(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());

  Thread &thread = _threads[thread_index];

  // If we're the main thread, we should exchange control packets with
  // the server.
  if (thread_index == 0) {
    transmit_control_data();
  }

  // If we've got the UDP port by the time the frame starts, it's
  // time to become active and start actually tracking data.
  if (_got_udp_port) {
    thread._is_active = true;
  }

  if (!thread._is_active) {
    return;
  }

  double frame_start = _clock.get_real_time();

  if (!thread._frame_data.is_empty()) {
    // Collector 0 is the whole frame.
    stop(0, thread_index, frame_start);

    /*
      We don't need to do this, since the stats server will do it.
      Why should we waste our time?

    // Make sure all of our Collectors have turned themselves off now
    // at the end of the frame.
    Collectors::iterator ci;
    for (ci = _collectors.begin(); ci != _collectors.end(); ++ci) {
      if ((*ci)._nested_count > 0) {
	pstats_cat.warning()
	  << "Collector " << get_collector_fullname((*ci)._def->_index)
	  << " wasn't stopped!\n";
	(*ci)._nested_count = 0;
      }
    } */

    transmit_frame_data(thread_index);
  }

  thread._frame_data.clear();
  thread._frame_number++;
  start(0, thread_index, frame_start);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::transmit_frame_data
//       Access: Private
//  Description: Should be called once per frame per thread to
//               transmit the latest data to the PStatServer.
////////////////////////////////////////////////////////////////////
void PStatClient::
transmit_frame_data(int thread_index) {
  nassertv(thread_index >= 0 && thread_index < (int)_threads.size());
  if (_is_connected && _threads[thread_index]._is_active) {
    
    // We don't want to send too many packets in a hurry and flood the
    // server.  In fact, we don't want to send more than
    // _max_rate packets per second, per thread.
    double min_packet_delay = 1.0 / _max_rate;
    double now = _clock.get_real_time();

    if (now - _threads[thread_index]._last_packet > min_packet_delay) {
      nassertv(_got_udp_port);
      
      // Send new data.
      NetDatagram datagram;
      datagram.add_uint16(thread_index);
      datagram.add_uint32(_threads[thread_index]._frame_number);
      _threads[thread_index]._frame_data.write_datagram(datagram);
      _writer.send(datagram, _udp_connection, _server);
      _threads[thread_index]._last_packet = now;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::transmit_control_data
//       Access: Private
//  Description: Should be called once a frame to exchange control
//               information with the server.
////////////////////////////////////////////////////////////////////
void PStatClient::
transmit_control_data() {
  // Check for new messages from the server.
  while (_is_connected && _reader.data_available()) {
    NetDatagram datagram;

    if (_reader.get_data(datagram)) {
      PStatServerControlMessage message;
      if (message.decode(datagram)) {
	handle_server_control_message(message);

      } else {
	pstats_cat.error()
	  << "Got unexpected message from server.\n";
      }
    }
  }
  
  if (_is_connected) {
    report_new_collectors();
    report_new_threads();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_hostname
//       Access: Private
//  Description: Returns the current machine's hostname.
////////////////////////////////////////////////////////////////////
string PStatClient::
get_hostname() {
  if (_hostname.empty()) {
    char temp_buff[1024];
    if (gethostname(temp_buff, 1024) == 0) {
      _hostname = temp_buff;
    } else {
      _hostname = "unknown";
    }
  }
  return _hostname;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::send_hello
//       Access: Private
//  Description: Sends the initial greeting message to the server.
////////////////////////////////////////////////////////////////////
void PStatClient::
send_hello() {
  nassertv(_is_connected);

  PStatClientControlMessage message;
  message._type = PStatClientControlMessage::T_hello;
  message._client_hostname = get_hostname();
  message._client_progname = _client_name;

  Datagram datagram;
  message.encode(datagram);
  _writer.send(datagram, _tcp_connection);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::report_new_collectors
//       Access: Private
//  Description: Sends over any information about new Collectors that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClient::
report_new_collectors() {
  nassertv(_is_connected);

  if (_collectors_reported < (int)_collectors.size()) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_collectors;
    while (_collectors_reported < (int)_collectors.size()) {
      message._collectors.push_back(_collectors[_collectors_reported]._def);
      _collectors_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::report_new_threads
//       Access: Private
//  Description: Sends over any information about new Threads that
//               the user code might have recently created.
////////////////////////////////////////////////////////////////////
void PStatClient::
report_new_threads() {
  nassertv(_is_connected);

  if (_threads_reported < (int)_threads.size()) {
    PStatClientControlMessage message;
    message._type = PStatClientControlMessage::T_define_threads;
    message._first_thread_index = _threads_reported;
    while (_threads_reported < (int)_threads.size()) {
      message._names.push_back(_threads[_threads_reported]._name);
      _threads_reported++;
    }

    Datagram datagram;
    message.encode(datagram);
    _writer.send(datagram, _tcp_connection);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::handle_server_control_message
//       Access: Private
//  Description: Called when a control message has been received by
//               the server over the TCP connection.
////////////////////////////////////////////////////////////////////
void PStatClient::
handle_server_control_message(const PStatServerControlMessage &message) {
  switch (message._type) {
  case PStatServerControlMessage::T_hello:
    pstats_cat.info()
      << "Connected to " << message._server_progname << " on " 
      << message._server_hostname << "\n";

    _server.set_port(message._udp_port);
    _got_udp_port = true;
    break;

  default:
    pstats_cat.error()
      << "Invalid control message received from server.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::connection_reset
//       Access: Private, Virtual
//  Description: Called by the internal net code when the connection
//               has been lost.
////////////////////////////////////////////////////////////////////
void PStatClient::
connection_reset(const PT(Connection) &connection) {
  if (connection == _tcp_connection) {
    disconnect();
  } else {
    pstats_cat.warning()
      << "Ignoring spurious connection_reset() message\n";
  }
}

#endif // DO_PSTATS
