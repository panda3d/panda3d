/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file queuedReturn.I
 * @author drose
 * @date 2000-02-25
 */

/**
 * Sets the maximum size the queue is allowed to grow to.  This is primarily
 * for a sanity check; this is a limit beyond which we can assume something
 * bad has happened.
 *
 * It's also a crude check against unfortunate seg faults due to the queue
 * filling up and quietly consuming all available memory.
 */
template<class Thing>
void QueuedReturn<Thing>::
set_max_queue_size(int max_size) {
  LightMutexHolder holder(_mutex);
  _max_queue_size = max_size;
}

/**
 * Returns the maximum size the queue is allowed to grow to.  See
 * set_max_queue_size().
 */
template<class Thing>
int QueuedReturn<Thing>::
get_max_queue_size() const {
  return _max_queue_size;
}

/**
 * Returns the current number of things in the queue.
 */
template<class Thing>
int QueuedReturn<Thing>::
get_current_queue_size() const {
  LightMutexHolder holder(_mutex);
  int size = _things.size();
  return size;
}

/**
 * Returns true if the queue has overflowed since the last call to
 * reset_overflow_flag() (implying that some elements have been dropped from
 * the queue), or false otherwise.
 */
template<class Thing>
bool QueuedReturn<Thing>::
get_overflow_flag() const {
  return _overflow_flag;
}

/**
 * Resets the overflow flag so that get_overflow_flag() will return false
 * until a new overflow occurs.
 */
template<class Thing>
void QueuedReturn<Thing>::
reset_overflow_flag() {
  _overflow_flag = false;
}

/**
 *
 */
template<class Thing>
QueuedReturn<Thing>::
QueuedReturn() {
  _available = false;
  _max_queue_size = get_net_max_response_queue();
  _overflow_flag = false;
}

/**
 *
 */
template<class Thing>
QueuedReturn<Thing>::
~QueuedReturn() {
}

/**
 * Returns true if a thing is available on the queue; call get_thing() to
 * extract the thing.
 */
template<class Thing>
INLINE bool QueuedReturn<Thing>::
thing_available() const {
  return _available;
}

/**
 * If a previous call to thing_available() returned true, this function will
 * return the thing that has become available.
 *
 * The return value is true if a thing was successfully returned, or false if
 * there was, in fact, no thing available.  (This may happen if there are
 * multiple threads accessing the QueuedReturn).
 */
template<class Thing>
bool QueuedReturn<Thing>::
get_thing(Thing &result) {
  LightMutexHolder holder(_mutex);
  if (_things.empty()) {
    // Huh.  Nothing after all.
    _available = false;
    return false;
  }

  result = _things.front();
  _things.pop_front();
  _available = !_things.empty();
  return true;
}

/**
 * Adds a new thing to the queue for later retrieval.  Returns true if
 * successful, false if the queue is full (i.e.  has reached _max_queue_size).
 */
template<class Thing>
bool QueuedReturn<Thing>::
enqueue_thing(const Thing &thing) {
  LightMutexHolder holder(_mutex);
  bool enqueue_ok = ((int)_things.size() < _max_queue_size);
  if (enqueue_ok) {
    _things.push_back(thing);
  } else {
    _overflow_flag = true;
  }
  _available = true;

  return enqueue_ok;
}

/**
 * The same as enqueue_thing(), except the queue is first checked that it
 * doesn't already have something like thing.  The return value is true if the
 * enqueue operation was successful, false if the queue was full or the thing
 * was already on the queue.
 */
template<class Thing>
bool QueuedReturn<Thing>::
enqueue_unique_thing(const Thing &thing) {
  LightMutexHolder holder(_mutex);
  bool enqueue_ok = ((int)_things.size() < _max_queue_size);
  if (enqueue_ok) {
    if (find(_things.begin(), _things.end(), thing) == _things.end()) {
      // It wasn't there already; add it now.
      _things.push_back(thing);
    } else {
      // It was already there; return false to indicate this.
      enqueue_ok = false;
    }

  } else {
    _overflow_flag = true;
  }
  _available = true;

  return enqueue_ok;
}
