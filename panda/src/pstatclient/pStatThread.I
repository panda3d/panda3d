/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pStatThread.I
 * @author drose
 * @date 2000-07-11
 */

/**
 * Normally, this constructor is called only from PStatClient.  Use one of the
 * constructors below to create your own Thread.
 */
INLINE PStatThread::
PStatThread() {
}

/**
 * Normally, this constructor is called only from PStatClient.  Use one of the
 * constructors below to create your own Thread.
 */
INLINE PStatThread::
PStatThread(PStatClient *client, int index) :
  _client(client),
  _index(index)
{
}

/**
 * Creates a new named thread.  This will be used to unify tasks that share a
 * common thread, and differentiate tasks that occur in different threads.
 */
INLINE PStatThread::
PStatThread(Thread *thread, PStatClient *client) {
#ifdef DO_PSTATS
  if (client == nullptr) {
    client = PStatClient::get_global_pstats();
  }

  int thread_index = thread->get_pstats_index();
  if (thread_index != -1) {
    (*this) = PStatThread(client, thread_index);

  } else {
    // This is the first time we have encountered this current Thread.  Make a
    // new PStatThread object for it.
    (*this) = client->make_thread(thread);
  }
#else
  _client = nullptr;
  _index = 0;
#endif
}

/**
 *
 */
INLINE PStatThread::
PStatThread(const PStatThread &copy) :
  _client(copy._client),
  _index(copy._index)
{
}

/**
 *
 */
INLINE void PStatThread::
operator = (const PStatThread &copy) {
  _client = copy._client;
  _index = copy._index;
}

/**
 * Returns the index number of this particular thread within the PStatClient.
 */
INLINE int PStatThread::
get_index() const {
  return _index;
}
