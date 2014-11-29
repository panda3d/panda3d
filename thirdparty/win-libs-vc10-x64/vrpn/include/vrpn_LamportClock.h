#ifndef VRPN_LAMPORT_CLOCK_H
#define VRPN_LAMPORT_CLOCK_H

#include "vrpn_Types.h"

/// @class vrpn_LamportTimestamp
/// Timestamp for a single event, produced by a vrpn_LamportClock and
/// hopefully generally usable in place of a struct timeval.

/// @class vrpn_LamportClock
/// Implements a distributed event clock as defined by Leslie Lamport in
/// some seminal papers I can't find my copies of, for use by people who
/// want to sequence events without relying on synchronization of wallclocks.

class VRPN_API vrpn_LamportTimestamp {

  public:

    vrpn_LamportTimestamp (int vectorLength, vrpn_uint32 * vector);
    vrpn_LamportTimestamp (const vrpn_LamportTimestamp &);
    ~vrpn_LamportTimestamp (void);

    vrpn_LamportTimestamp & operator = (const vrpn_LamportTimestamp &);


    // ACCESSORS


    vrpn_bool operator < (const vrpn_LamportTimestamp & r) const;
      ///< Returns vrpn_true if this timestamp precedes r.
      ///< It'd be nice if we could throw an exception here,
      ///< since some timestamps are incommesurate.


    // Utility functions.

    vrpn_uint32 operator [] (int i) const;
      ///< Returns the event count for the i'th host.

    int size (void) const;
      ///< Returns the number of hosts participating in the timestamp.


  private:

    void copy (const vrpn_uint32 *);
      ///< Used by constructors and operator = to copy values into
      ///< d_timestamp;  don't we wish we were using STL?

    int d_timestampSize;
    vrpn_uint32 * d_timestamp;

    vrpn_LamportTimestamp (void);
      ///< UNDEFINED - not legal.

};


class VRPN_API vrpn_LamportClock {

  public:

    vrpn_LamportClock (int numHosts, int ourIndex);
    ~vrpn_LamportClock (void);


    // MANIPULATORS


    void receive (const vrpn_LamportTimestamp &);
      ///< Updates this clock to reflect a timestamp received from
      ///< another clock/host.

    vrpn_LamportTimestamp * getTimestampAndAdvance (void);
      ///< Increments the current timestamp and returns it.


  private:

    int d_numHosts;
    int d_ourIndex;
    vrpn_uint32 * d_currentTimestamp;

};



#endif  // VRPN_LAMPORT_CLOCK_H


