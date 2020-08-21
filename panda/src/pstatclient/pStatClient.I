/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatClient.I
 * @author drose
 * @date 2000-07-16
 */

/**
 * Returns the total number of collectors the Client knows about.
 */
INLINE int PStatClient::
get_num_collectors() const {
  ReMutexHolder holder(_lock);
  return (int)_num_collectors;
}

/**
 * Returns the definition body of the nth collector.
 */
INLINE PStatCollectorDef *PStatClient::
get_collector_def(int index) const {
  nassertr(index >= 0 && index < _num_collectors, nullptr);

  return get_collector_ptr(index)->get_def(this, index);
}

/**
 * Returns the total number of threads the Client knows about.
 */
INLINE int PStatClient::
get_num_threads() const {
  ReMutexHolder holder(_lock);
  return (int)_num_threads;
}

/**
 * Returns the name of the indicated thread.
 */
INLINE std::string PStatClient::
get_thread_name(int index) const {
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_threads), std::string());
  return get_thread_ptr(index)->_name;
}

/**
 * Returns the sync_name of the indicated thread.
 */
INLINE std::string PStatClient::
get_thread_sync_name(int index) const {
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_threads), std::string());
  return get_thread_ptr(index)->_sync_name;
}

/**
 * Returns the Panda Thread object associated with the indicated PStatThread.
 */
INLINE PT(Thread) PStatClient::
get_thread_object(int index) const {
  nassertr(index >= 0 && index < AtomicAdjust::get(_num_threads), nullptr);
  InternalThread *thread = get_thread_ptr(index);
  return thread->_thread.lock();
}

/**
 * Attempts to establish a connection to the indicated PStatServer.  Returns
 * true if successful, false on failure.
 */
INLINE bool PStatClient::
connect(const std::string &hostname, int port) {
  return get_global_pstats()->client_connect(hostname, port);
}

/**
 * Closes the connection previously established.
 */
INLINE void PStatClient::
disconnect() {
  get_global_pstats()->client_disconnect();
}

/**
 * Returns true if the client believes it is connected to a working
 * PStatServer, false otherwise.
 */
INLINE bool PStatClient::
is_connected() {
  return get_global_pstats()->client_is_connected();
}

/**
 * Resumes the PStatClient after the simulation has been paused for a while.
 * This allows the stats to continue exactly where it left off, instead of
 * leaving a big gap that would represent a chug.
 */
INLINE void PStatClient::
resume_after_pause() {
  get_global_pstats()->client_resume_after_pause();
}

/**
 * Returns true if the PStatClientImpl object has been created for this object
 * yet, false otherwise.
 */
INLINE bool PStatClient::
has_impl() const {
  return (_impl != nullptr);
}

/**
 * Returns the PStatClientImpl object for this object.  If the PStatClientImpl
 * object has not yet been created, implicitly creates it.
 */
INLINE PStatClientImpl *PStatClient::
get_impl() {
  ReMutexHolder holder(_lock);
  if (_impl == nullptr) {
    make_impl();
  }
  return _impl;
}

/**
 * Returns the PStatClientImpl object for this object.  If the PStatClientImpl
 * object has not yet been created, implicitly creates it.
 */
INLINE const PStatClientImpl *PStatClient::
get_impl() const {
  ReMutexHolder holder(_lock);
  if (_impl == nullptr) {
    make_impl();
  }
  return _impl;
}

/**
 * Returns the nth collector in a thread-safe manner, even if _lock is not
 * held.
 */
INLINE PStatClient::Collector *PStatClient::
get_collector_ptr(int collector_index) const {
  CollectorPointer *collectors = (CollectorPointer *)AtomicAdjust::get_ptr(_collectors);
  return collectors[collector_index];
}

/**
 * Returns the nth thread in a thread-safe manner, even if _lock is not held.
 */
INLINE PStatClient::InternalThread *PStatClient::
get_thread_ptr(int thread_index) const {
  ThreadPointer *threads = (ThreadPointer *)AtomicAdjust::get_ptr(_threads);
  return threads[thread_index];
}

/**
 *
 */
INLINE PStatClient::Collector::
Collector(int parent_index, const std::string &name) :
  _def(nullptr),
  _parent_index(parent_index),
  _name(name)
{
}

/**
 *
 */
INLINE int PStatClient::Collector::
get_parent_index() const {
  return _parent_index;
}

/**
 *
 */
INLINE const std::string &PStatClient::Collector::
get_name() const {
  return _name;
}

/**
 * Returns true if the indicated collector has been designated as active,
 * false otherwise.  This might return initially false until the collector def
 * has actually been created.
 */
INLINE bool PStatClient::Collector::
is_active() const {
  return _def != nullptr && _def->_is_active;
}

/**
 * Returns the PStatCollectorDef that contains all of the information about
 * the collector.  If this object has not yet been created, creates it.
 */
INLINE PStatCollectorDef *PStatClient::Collector::
get_def(const PStatClient *client, int this_index) const {
  if (_def == nullptr) {
    ((Collector *)this)->make_def(client, this_index);
  }

  return _def;
}
