// Filename: pStatCollector.I
// Created by:  drose (10Jul00)
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
//       Access: Public
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
PStatCollector(const string &name, const RGBColorf &suggested_color,
	       int sort, PStatClient *client) {
  if (client == (PStatClient *)NULL) {
    client = PStatClient::get_global_pstats();
  }
  PStatCollector collector(client->make_collector(0, name, suggested_color, sort));
  (*this) = collector;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Constructor
//       Access: Public
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
PStatCollector(const PStatCollector &parent, const string &name, 
	       const RGBColorf &suggested_color, int sort) {
  nassertv(parent._client != (PStatClient *)NULL);
  PStatCollector collector(parent._client->make_collector
			   (parent._index, name, suggested_color, sort));
  (*this) = collector;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::Copy Constructor
//       Access: Public
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
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
operator = (const PStatCollector &copy) {
  _client = copy._client;
  _index = copy._index;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::start
//       Access: Public
//  Description: Starts this particular timer ticking.  This should be
//               called before the code you want to measure.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
start() {
  _client->start(_index, 0, _client->_clock.get_real_time());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::start
//       Access: Public
//  Description: Starts this timer ticking within a particular thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
start(const PStatThread &thread) {
  _client->start(_index, thread._index, _client->_clock.get_real_time());
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
start(const PStatThread &thread, double as_of) {
  _client->start(_index, thread._index, as_of);
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::stop
//       Access: Public
//  Description: Stops this timer.  This should be called after the
//               code you want to measure.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
stop() {
  _client->stop(_index, 0, _client->_clock.get_real_time());
}

////////////////////////////////////////////////////////////////////
//     Function: PStatCollector::stop
//       Access: Public
//  Description: Stops this timer within a particular thread.
////////////////////////////////////////////////////////////////////
INLINE void PStatCollector::
stop(const PStatThread &thread) {
  _client->stop(_index, thread._index, _client->_clock.get_real_time());
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
stop(const PStatThread &thread, double as_of) {
  _client->stop(_index, thread._index, as_of);
}

#endif  // DO_PSTATS
