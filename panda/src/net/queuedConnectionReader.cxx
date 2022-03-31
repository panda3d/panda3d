/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queuedConnectionReader.cxx
 * @author drose
 * @date 2000-02-08
 */

#include "queuedConnectionReader.h"
#include "config_net.h"
#include "trueClock.h"
#include "lightMutexHolder.h"

template class QueuedReturn<NetDatagram>;

/**
 *
 */
QueuedConnectionReader::
QueuedConnectionReader(ConnectionManager *manager, int num_threads) :
  ConnectionReader(manager, num_threads)
{
#ifdef SIMULATE_NETWORK_DELAY
  _delay_active = false;
  _min_delay = 0.0;
  _delay_variance = 0.0;
#endif  // SIMULATE_NETWORK_DELAY
}

/**
 *
 */
QueuedConnectionReader::
~QueuedConnectionReader() {
  // We call shutdown() here to guarantee that all threads are gone before the
  // QueuedReturn destructs.
  shutdown();
}

/**
 * Returns true if a datagram is available on the queue; call get_data() to
 * extract the datagram.
 */
bool QueuedConnectionReader::
data_available() {
  poll();
#ifdef SIMULATE_NETWORK_DELAY
  get_delayed();
#endif  // SIMULATE_NETWORK_DELAY
  return thing_available();
}

/**
 * If a previous call to data_available() returned true, this function will
 * return the datagram that has become available.
 *
 * The return value is true if a datagram was successfully returned, or false
 * if there was, in fact, no datagram available.  (This may happen if there
 * are multiple threads accessing the QueuedConnectionReader).
 */
bool QueuedConnectionReader::
get_data(NetDatagram &result) {
  return get_thing(result);
}

/**
 * This flavor of QueuedConnectionReader::get_data(), works like the other,
 * except that it only fills a Datagram object, not a NetDatagram object.
 * This means that the Datagram cannot be queried for its source Connection
 * and/or NetAddress, but it is useful in all other respects.
 */
bool QueuedConnectionReader::
get_data(Datagram &result) {
  NetDatagram nd;
  if (!get_thing(nd)) {
    return false;
  }
  result = nd;
  return true;
}

/**
 * An internal function called by ConnectionReader() when a new datagram has
 * become available.  The QueuedConnectionReader simply queues it up for later
 * retrieval by get_data().
 */
void QueuedConnectionReader::
receive_datagram(const NetDatagram &datagram) {
  /*
  if (net_cat.is_spam()) {
    net_cat.spam()
      << "Received datagram of " << datagram.get_length()
      << " bytes\n";
  }
  */

#ifdef SIMULATE_NETWORK_DELAY
  delay_datagram(datagram);

#else  // SIMULATE_NETWORK_DELAY
  if (!enqueue_thing(datagram)) {
    net_cat.error()
      << "QueuedConnectionReader queue full!\n";
  }
#endif  // SIMULATE_NETWORK_DELAY
}


#ifdef SIMULATE_NETWORK_DELAY
/**
 * Enables a simulated network latency.  All packets received from this point
 * on will be held for a random interval of least min_delay seconds, and no
 * more than max_delay seconds, before being visible to the
 * data_available()/get_data() interface.  It is as if packets suddenly took
 * much longer to arrive.
 */
void QueuedConnectionReader::
start_delay(double min_delay, double max_delay) {
  LightMutexHolder holder(_dd_mutex);
  _min_delay = min_delay;
  _delay_variance = std::max(max_delay - min_delay, 0.0);
  _delay_active = true;
}

/**
 * Disables the simulated network latency started by a previous call to
 * start_delay().  Packets will once again be visible as soon as they are
 * received.
 */
void QueuedConnectionReader::
stop_delay() {
  LightMutexHolder holder(_dd_mutex);
  _delay_active = false;

  // Copy the entire contents of the delay queue to the normal queue.
  while (!_delayed.empty()) {
    const DelayedDatagram &dd = _delayed.front();
    if (!enqueue_thing(dd._datagram)) {
      net_cat.error()
        << "QueuedConnectionReader queue full!\n";
    }
    _delayed.pop_front();
  }
}

/**
 * Checks the delayed queue for any now available datagrams, and adds them to
 * the normal queue if they are available.
 */
void QueuedConnectionReader::
get_delayed() {
  if (_delay_active) {
    LightMutexHolder holder(_dd_mutex);
    double now = TrueClock::get_global_ptr()->get_short_time();
    while (!_delayed.empty()) {
      const DelayedDatagram &dd = _delayed.front();
      if (dd._reveal_time > now) {
        // Not yet.
        break;
      }
      if (!enqueue_thing(dd._datagram)) {
        net_cat.error()
          << "QueuedConnectionReader queue full!\n";
      }
      _delayed.pop_front();
    }
  }
}

/**
 * Adds the datagram to the delay queue for a random time interval.
 */
void QueuedConnectionReader::
delay_datagram(const NetDatagram &datagram) {
  if (!_delay_active) {
    if (!enqueue_thing(datagram)) {
      net_cat.error()
        << "QueuedConnectionReader queue full!\n";
    }
  } else {
    LightMutexHolder holder(_dd_mutex);
    // Check the delay_active flag again, now that we have grabbed the mutex.
    if (!_delay_active) {
      if (!enqueue_thing(datagram)) {
        net_cat.error()
          << "QueuedConnectionReader queue full!\n";
      }

    } else {
      double now = TrueClock::get_global_ptr()->get_short_time();
      double reveal_time = now + _min_delay;

      if (_delay_variance > 0.0) {
        reveal_time += _delay_variance * ((double)rand() / (double)RAND_MAX);
      }
      _delayed.push_back(DelayedDatagram());
      DelayedDatagram &dd = _delayed.back();
      dd._reveal_time = reveal_time;
      dd._datagram = datagram;
    }
  }
}

#endif  // SIMULATE_NETWORK_DELAY
