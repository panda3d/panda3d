// Filename: pStatCollector.I
// Created by:  drose (10Jul00)
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


#ifdef DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Default Constructor
//       Access: Private
//  Description: Normally, this constructor is called only from
//               PStatClient.  Use one of the constructors below to
//               create your own Collector.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector() {
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Private
//  Description: Normally, this constructor is called only from
//               PStatClient.  Use one of the constructors below to
//               create your own Collector.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(PStatClient *client, int index) :
  _client(client),
  _index(index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Published
//  Description: Creates a new PStatCollector, ready to start
//               accumulating data.  The name of the collector
//               uniquely identifies it among the other collectors; if
//               two collectors share the same name then they are
//               really the same collector.
//
//               The name may also be a compound name, something like
//               "Cull:Sort", which indicates that this is a collector
//               named "Sort", a child of the collector named "Cull".
//               The parent may also be named explicitly by reference
//               in the other flavor of the constructor; see further
//               comments on this for that constructor.
//
//               If the client pointer is non-null, it specifies a
//               particular client to register the collector with;
//               otherwise, the global client is used.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(const string &name, PStatClient *client) {
  if (client == (PStatClient *)NULL) {
    client = PStatClient::get_global_pstats();
  }
  (*this) = client->make_collector_with_relname(0, name);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Published
//  Description: Creates a new PStatCollector, ready to start
//               accumulating data.  The name of the collector
//               uniquely identifies it among the other collectors; if
//               two collectors share the same name then they are
//               really the same collector.
//
//               The parent is the collector that conceptually
//               includes all of the time measured for this collector.
//               For instance, a particular character's animation time
//               is owned by the "Animation" collector, which is in
//               turn owned by the "Frame" collector.  It is not
//               strictly necessary that all of the time spent in a
//               particular collector is completely nested within time
//               spent in its parent's collector.  If parent is the
//               empty string, the collector is owned by "Frame".
//
//               This constructor does not take a client pointer; it
//               always creates the new collector on the same client
//               as its parent.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(const PStatCollector &parent, const string &name) {
  nassertv(parent._client != (PStatClient *)NULL);
  (*this) =
    parent._client->make_collector_with_relname(parent._index, name);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(const PStatCollector &copy) :
  _client(copy._client),
  _index(copy._index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
operator = (const PStatCollector &copy) {
  _client = copy._client;
  _index = copy._index;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::is_active
//       Access: Published
//  Description: Returns true if this particular collector is active
//               on the default thread, and we are currently
//               transmitting PStats data.
////////////////////////////////////////////////////////////////////
INLINE bool PStatCollector::
is_active() {
  return _client->is_active(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::is_started
//       Access: Published
//  Description: Returns true if this particular collector has been
//               started on the default thread, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatCollector::
is_started() {
  return _client->is_started(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::start
//       Access: Published
//  Description: Starts this particular timer ticking.  This should be
//               called before the code you want to measure.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
start() {
  _client->start(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::stop
//       Access: Published
//  Description: Stops this timer.  This should be called after the
//               code you want to measure.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
stop() {
  _client->stop(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::clear_level
//       Access: Published
//  Description: Removes the level setting associated with this
//               collector for the main thread.  The collector
//               will no longer show up on any level graphs in the
//               main thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
clear_level() {
  _client->clear_level(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::set_level
//       Access: Published
//  Description: Sets the level setting associated with this
//               collector for the main thread to the indicated
//               value.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
set_level(float level) {
  _client->set_level(_index, 0, level);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::add_level
//       Access: Published
//  Description: Adds the indicated increment (which may be negative)
//               to the level setting associated with this collector
//               for the main thread.  If the collector did not
//               already have a level setting for the main thread, it
//               is initialized to 0.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
add_level(float increment) {
  _client->add_level(_index, 0, increment);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::sub_level
//       Access: Published
//  Description: Subtracts the indicated decrement (which may be
//               negative) to the level setting associated with this
//               collector for the main thread.  If the collector did
//               not already have a level setting for the main thread,
//               it is initialized to 0.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
sub_level(float decrement) {
  _client->add_level(_index, 0, -decrement);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::get_level
//       Access: Published
//  Description: Returns the current level value of the given collector.
////////////////////////////////////////////////////////////////////
INLINE float PStatCollector::
get_level() {
  return _client->get_level(_index, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::is_active
//       Access: Public
//  Description: Returns true if this particular collector is active
//               on the indicated thread, and we are currently
//               transmitting PStats data.
////////////////////////////////////////////////////////////////////
INLINE bool PStatCollector::
is_active(const PStatThread &thread) {
  return _client->is_active(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::is_started
//       Access: Public
//  Description: Returns true if this particular collector has been
//               started on the indicated thread, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatCollector::
is_started(const PStatThread &thread) {
  return _client->is_started(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::start
//       Access: Public
//  Description: Starts this timer ticking within a particular thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
start(const PStatThread &thread) {
  _client->start(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::start
//       Access: Public
//  Description: Marks that the timer should have been started as of
//               the indicated time.  This must be a time based on the
//               PStatClient's clock (see PStatClient::get_clock()),
//               and care should be taken that all such calls exhibit
//               a monotonically increasing series of time values.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
start(const PStatThread &thread, float as_of) {
  _client->start(_index, thread._index, as_of);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::stop
//       Access: Public
//  Description: Stops this timer within a particular thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
stop(const PStatThread &thread) {
  _client->stop(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::stop
//       Access: Public
//  Description: Marks that the timer should have been stopped as of
//               the indicated time.  This must be a time based on the
//               PStatClient's clock (see PStatClient::get_clock()),
//               and care should be taken that all such calls exhibit
//               a monotonically increasing series of time values.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
stop(const PStatThread &thread, float as_of) {
  _client->stop(_index, thread._index, as_of);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::clear_level
//       Access: Public
//  Description: Removes the level setting associated with this
//               collector for the indicated thread.  The collector
//               will no longer show up on any level graphs in this
//               thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
clear_level(const PStatThread &thread) {
  _client->clear_level(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::set_level
//       Access: Public
//  Description: Sets the level setting associated with this
//               collector for the indicated thread to the indicated
//               value.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
set_level(const PStatThread &thread, float level) {
  _client->set_level(_index, thread._index, level);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::add_level
//       Access: Public
//  Description: Adds the indicated increment (which may be negative)
//               to the level setting associated with this collector
//               for the indicated thread.  If the collector did not
//               already have a level setting for this thread, it is
//               initialized to 0.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
add_level(const PStatThread &thread, float increment) {
  _client->add_level(_index, thread._index, increment);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::sub_level
//       Access: Public
//  Description: Subtracts the indicated decrement (which may be
//               negative) to the level setting associated with this
//               collector for the indicated thread.  If the collector
//               did not already have a level setting for this thread,
//               it is initialized to 0.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
sub_level(const PStatThread &thread, float decrement) {
  _client->add_level(_index, thread._index, -decrement);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::get_level
//       Access: Public
//  Description: Returns the current level value of the given collector.
////////////////////////////////////////////////////////////////////
INLINE float PStatCollector::
get_level(const PStatThread &thread) {
  return _client->get_level(_index, thread._index);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::get_index
//       Access: Public
//  Description: Returns the index number of this particular collector
//               within the PStatClient.
////////////////////////////////////////////////////////////////////
INLINE int PStatCollector::
get_index() const {
  return _index;
}

#else  // DO_PSTATS

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Published
//  Description: This bogus version of the function is only defined if
//               DO_PSTATS is not defined, meaning all these functions
//               should compile to nothing.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(const string &, PStatClient *client) {
  // We need this bogus comparison just to prevent the SGI compiler
  // from dumping core.  It's perfectly meaningless.
#ifdef mips
  if (client == (PStatClient *)NULL) {
    return;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Published
//  Description: This bogus version of the function is only defined if
//               DO_PSTATS is not defined, meaning all these functions
//               should compile to nothing.
////////////////////////////////////////////////////////////////////
INLINE PStatCollector::
PStatCollector(const PStatCollector &parent, const string &) {
  // We need this bogus comparison just to prevent the SGI compiler
  // from dumping core.  It's perfectly meaningless.
#ifdef mips
  if (&parent == (const PStatCollector *)NULL) {
    return;
  }
#endif
}


#endif  // DO_PSTATS
