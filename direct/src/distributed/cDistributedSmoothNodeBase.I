// Filename: cDistributedSmoothNodeBase.I
// Created by:  drose (03Sep04)
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
//     Function: CDistributedSmoothNodeBase::set_repository
//       Access: Published, Static
//  Description: Tells the C++ class definition about the AI or Client
//               repository, used for sending datagrams.
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
set_repository(CConnectionRepository *repository,
               bool is_ai, CHANNEL_TYPE ai_id) {
  _repository = repository;
  _is_ai = is_ai;
  _ai_id = ai_id;
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::set_clock_delta
//       Access: Published, Static
//  Description: Tells the C++ class definition about the global
//               ClockDelta object.
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
set_clock_delta(PyObject *clock_delta) {
  _clock_delta = clock_delta;
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::only_changed
//       Access: Private, Static
//  Description: Returns true if at least some of the bits of compare
//               are set in flags, but no bits outside of compare are
//               set.  That is to say, that the only things that are
//               changed are the bits indicated in compare.
////////////////////////////////////////////////////////////////////
INLINE bool CDistributedSmoothNodeBase::
only_changed(int flags, int compare) {
  return (flags & compare) != 0 && (flags & ~compare) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmStop
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmStop() {
  DCPacker packer;
  begin_send_update(packer, "setSmStop");
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmH
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmH(float h) {
  DCPacker packer;
  begin_send_update(packer, "setSmH");
  packer.pack_double(h);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmXY
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmXY(float x, float y) {
  DCPacker packer;
  begin_send_update(packer, "setSmXY");
  packer.pack_double(x);
  packer.pack_double(y);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmXZ
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmXZ(float x, float z) {
  DCPacker packer;
  begin_send_update(packer, "setSmXZ");
  packer.pack_double(x);
  packer.pack_double(z);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmPos
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmPos(float x, float y, float z) {
  DCPacker packer;
  begin_send_update(packer, "setSmPos");
  packer.pack_double(x);
  packer.pack_double(y);
  packer.pack_double(z);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmHpr
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmHpr(float h, float p, float r) {
  DCPacker packer;
  begin_send_update(packer, "setSmHpr");
  packer.pack_double(h);
  packer.pack_double(p);
  packer.pack_double(r);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmXYH
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmXYH(float x, float y, float h) {
  DCPacker packer;
  begin_send_update(packer, "setSmXYH");
  packer.pack_double(x);
  packer.pack_double(y);
  packer.pack_double(h);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmXYZH
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmXYZH(float x, float y, float z, float h) {
  DCPacker packer;
  begin_send_update(packer, "setSmXYZH");
  packer.pack_double(x);
  packer.pack_double(y);
  packer.pack_double(z);
  packer.pack_double(h);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmPosHpr
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmPosHpr(float x, float y, float z, float h, float p, float r) {
  DCPacker packer;
  begin_send_update(packer, "setSmPosHpr");
  packer.pack_double(x);
  packer.pack_double(y);
  packer.pack_double(z);
  packer.pack_double(h);
  packer.pack_double(p);
  packer.pack_double(r);
  finish_send_update(packer);
}
