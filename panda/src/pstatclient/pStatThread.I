// Filename: pStatThread.I
// Created by:  drose (11Jul00)
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
//       Access: Private
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
//               If any two different PStatThread objects share the
//               same name, then they are really the same thread.
////////////////////////////////////////////////////////////////////
INLINE PStatThread::
PStatThread(const string &name, PStatClient *client) {
#ifdef DO_PSTATS
  if (client == (PStatClient *)NULL) {
    client = PStatClient::get_global_pstats();
  }
  PStatThread thread(client->make_thread(name));
  (*this) = thread;
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
////////////////////////////////////////////////////////////////////
INLINE void PStatThread::
new_frame() {
#ifdef DO_PSTATS
  _client->get_impl()->new_frame(_index);
#endif
}
