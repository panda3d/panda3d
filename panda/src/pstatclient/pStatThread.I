// Filename: pStatThread.I
// Created by:  drose (11Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PStatThread::Default Constructor
//       Access: Private
//  Description: Normally, this constructor is called only from
//               PStatClient.  Use one of the constructors below to
//               create your own Thread.
////////////////////////////////////////////////////////////////////
INLINE PStatThread::
PStatThread() {
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::Constructor
//       Access: Published
//  Description: Normally, this constructor is called only from
//               PStatClient.  Use one of the constructors below to
//               create your own Thread.
////////////////////////////////////////////////////////////////////
INLINE PStatThread::
PStatThread(PStatClient *client, int index) :
  _client(client),
  _index(index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::Constructor
//       Access: Public
//  Description: Creates a new named thread.  This will be used to
//               unify tasks that share a common thread, and
//               differentiate tasks that occur in different threads.
////////////////////////////////////////////////////////////////////
INLINE PStatThread::
PStatThread(Thread *thread, PStatClient *client) {
#ifdef DO_PSTATS
  if (client == (PStatClient *)NULL) {
    client = PStatClient::get_global_pstats();
  }

  int thread_index = thread->get_pstats_index();
  if (thread_index != -1) {
    (*this) = PStatThread(client, thread_index);

  } else {
    // This is the first time we have encountered this current Thread.
    // Make a new PStatThread object for it.
    (*this) = client->make_thread(thread);
  }
#else
  _client = (PStatClient *)NULL;
  _index = 0;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PStatThread::
PStatThread(const PStatThread &copy) :
  _client(copy._client),
  _index(copy._index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PStatThread::
operator = (const PStatThread &copy) {
  _client = copy._client;
  _index = copy._index;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::new_frame
//       Access: Public
//  Description: This must be called at the start of every "frame",
//               whatever a frame may be deemed to be, to accumulate
//               all the stats that have collected so far for the
//               thread and ship them off to the server.
//
//               Calling PStatClient::thread_tick() will automatically
//               call this for any threads with the indicated sync
//               name.
////////////////////////////////////////////////////////////////////
INLINE void PStatThread::
new_frame() {
#ifdef DO_PSTATS
  _client->get_impl()->new_frame(_index);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::add_frame
//       Access: Public
//  Description: This is a slightly lower-level version of new_frame
//               that also specifies the data to send for this frame.
////////////////////////////////////////////////////////////////////
INLINE void PStatThread::
add_frame(const PStatFrameData &frame_data) {
#ifdef DO_PSTATS
  _client->get_impl()->add_frame(_index, frame_data);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: PStatThread::get_index
//       Access: Published
//  Description: Returns the index number of this particular thread
//               within the PStatClient.
////////////////////////////////////////////////////////////////////
INLINE int PStatThread::
get_index() const {
  return _index;
}
