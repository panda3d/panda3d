// Filename: pStatClient.I
// Created by:  drose (16Jul00)
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


////////////////////////////////////////////////////////////////////
//     Function: PStatClient::set_client_name
//       Access: Published
//  Description: Sets the name of the client.  This is reported to the
//               PStatsServer, and will presumably be written in the
//               title bar or something.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
set_client_name(const string &name) {
  get_impl()->set_client_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_client_name
//       Access: Published
//  Description: Retrieves the name of the client as set.
////////////////////////////////////////////////////////////////////
INLINE string PStatClient::
get_client_name() const {
  return get_impl()->get_client_name();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::set_max_rate
//       Access: Published
//  Description: Controls the number of packets that will be sent to
//               the server.  Normally, one packet is sent per frame,
//               but this can flood the server with more packets than
//               it can handle if the frame rate is especially good
//               (e.g. if nothing is onscreen at the moment).  Set
//               this parameter to a reasonable number to prevent this
//               from happening.
//
//               This number specifies the maximum number of packets
//               that will be sent to the server per second, per
//               thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
set_max_rate(float rate) {
  get_impl()->set_max_rate(rate);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_max_rate
//       Access: Published
//  Description: Returns the maximum number of packets that will be
//               sent to the server per second, per thread.  See
//               set_max_rate().
////////////////////////////////////////////////////////////////////
INLINE float PStatClient::
get_max_rate() const {
  return get_impl()->get_max_rate();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_collectors
//       Access: Published
//  Description: Returns the total number of collectors the Client
//               knows about.
////////////////////////////////////////////////////////////////////
INLINE int PStatClient::
get_num_collectors() const {
  return _collectors.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_collector_def
//       Access: Published
//  Description: Returns the definition body of the nth collector.
////////////////////////////////////////////////////////////////////
INLINE PStatCollectorDef *PStatClient::
get_collector_def(int index) const {
  nassertr(index >= 0 && index < (int)_collectors.size(), NULL);

  return _collectors[index].get_def(this, index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_num_threads
//       Access: Published
//  Description: Returns the total number of threads the Client
//               knows about.
////////////////////////////////////////////////////////////////////
INLINE int PStatClient::
get_num_threads() const {
  return _threads.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_thread_name
//       Access: Published
//  Description: Returns the name of the indicated thread.
////////////////////////////////////////////////////////////////////
INLINE string PStatClient::
get_thread_name(int index) const {
  nassertr(index >= 0 && index < (int)_threads.size(), string());
  return _threads[index]._name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_clock
//       Access: Published
//  Description: Returns a reference to the PStatClient's clock
//               object.  It keeps its own clock, instead of using the
//               global clock object, so the stats won't get mucked up
//               if you put the global clock in non-real-time mode or
//               something.
////////////////////////////////////////////////////////////////////
INLINE const ClockObject &PStatClient::
get_clock() const {
  return get_impl()->get_clock();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::connect
//       Access: Published, Static
//  Description: Attempts to establish a connection to the indicated
//               PStatServer.  Returns true if successful, false on
//               failure.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
connect(const string &hostname, int port) {
  return get_global_pstats()->client_connect(hostname, port);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::disconnect
//       Access: Published, Static
//  Description: Closes the connection previously established.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
disconnect() {
  get_global_pstats()->client_disconnect();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::is_connected
//       Access: Published, Static
//  Description: Returns true if the client believes it is connected
//               to a working PStatServer, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
is_connected() {
  return get_global_pstats()->client_is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::resume_after_pause
//       Access: Published, Static
//  Description: Resumes the PStatClient after the simulation has been
//               paused for a while.  This allows the stats to
//               continue exactly where it left off, instead of
//               leaving a big gap that would represent a chug.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
resume_after_pause() {
  get_global_pstats()->client_resume_after_pause();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_connect
//       Access: Published
//  Description: The nonstatic implementation of connect().
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
client_connect(string hostname, int port) {
  client_disconnect();
  return get_impl()->client_connect(hostname, port);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_is_connected
//       Access: Published
//  Description: The nonstatic implementation of is_connected().
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
client_is_connected() const {
  return has_impl() && _impl->client_is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::client_resume_after_pause
//       Access: Published
//  Description: Resumes the PStatClient after the simulation has been
//               paused for a while.  This allows the stats to
//               continue exactly where it left off, instead of
//               leaving a big gap that would represent a chug.
////////////////////////////////////////////////////////////////////
INLINE void PStatClient::
client_resume_after_pause() {
  if (has_impl()) {
    _impl->client_resume_after_pause();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::has_impl
//       Access: Private
//  Description: Returns true if the PStatClientImpl object has been
//               created for this object yet, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::
has_impl() const {
  return (_impl != (PStatClientImpl *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_impl
//       Access: Private
//  Description: Returns the PStatClientImpl object for this object.
//               If the PStatClientImpl object has not yet been
//               created, implicitly creates it.
////////////////////////////////////////////////////////////////////
INLINE PStatClientImpl *PStatClient::
get_impl() {
  if (_impl == (PStatClientImpl *)NULL) {
    _impl = new PStatClientImpl(this);
  }

  return _impl;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::get_impl
//       Access: Private
//  Description: Returns the PStatClientImpl object for this object.
//               If the PStatClientImpl object has not yet been
//               created, implicitly creates it.
////////////////////////////////////////////////////////////////////
INLINE const PStatClientImpl *PStatClient::
get_impl() const {
  return ((PStatClient *)this)->get_impl();
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PStatClient::Collector::
Collector(int parent_index, const string &name) :
  _def(NULL),
  _parent_index(parent_index),
  _name(name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::get_parent_index
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PStatClient::Collector::
get_parent_index() const {
  return _parent_index;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::get_name
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const string &PStatClient::Collector::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::is_active
//       Access: Public
//  Description: Returns true if the indicated collector has been
//               designated as active, false otherwise.  This might
//               return initially false until the collector def has
//               actually been created.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClient::Collector::
is_active() const {
  return _def != (PStatCollectorDef *)NULL && _def->_is_active;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClient::Collector::get_def
//       Access: Public
//  Description: Returns the PStatCollectorDef that contains all of
//               the information about the collector.  If this object
//               has not yet been created, creates it.
////////////////////////////////////////////////////////////////////
INLINE PStatCollectorDef *PStatClient::Collector::
get_def(const PStatClient *client, int this_index) const {
  if (_def == (PStatCollectorDef *)NULL) {
    ((Collector *)this)->make_def(client, this_index);
  }

  return _def;
}
