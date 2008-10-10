// Filename: cDistributedSmoothNodeBase.I
// Created by:  drose (03Sep04)
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

#ifdef HAVE_PYTHON
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
#endif  // HAVE_PYTHON

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
  //cout << "d_setSmStop" << endl;
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
  //cout << "d_setSmH: " << h << endl;
  DCPacker packer;
  begin_send_update(packer, "setSmH");
  packer.pack_double(h);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmZ
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmZ(float z) {
  //cout << "d_setSmZ: " << z << endl;
  DCPacker packer;
  begin_send_update(packer, "setSmZ");
  packer.pack_double(z);
  finish_send_update(packer);
}

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmXY
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmXY(float x, float y) {
  //cout << "d_setSmXY: " << x << ", " << y << endl;
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
  //cout << "d_setSmXZ: " << x << ", " << z << endl;
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
  //cout << "d_setSmXYZ: " << x << ", " << y << ", " << z << endl;
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
  //cout << "d_setSmHPR: " << h << ", " << p << ", " << r << endl;
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
  //cout << "d_setSmXYH: " << x << ", " << y << ", " << h << endl;
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
  //cout << "d_setSmXYZH: " << x << ", " << y << ", " << z << ", " << h << endl;
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
  //cout << "d_setSmPosHpr: " << x << ", " << y << ", " << z << ", " << h << ", " << p << ", " << r << endl;
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

////////////////////////////////////////////////////////////////////
//     Function: CDistributedSmoothNodeBase::d_setSmPosHprL
//                 send out pos, hpr, and location info (zoneId)
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CDistributedSmoothNodeBase::
d_setSmPosHprL(float x, float y, float z, float h, float p, float r, PN_uint64 l) {
  //cout << "d_setSmPosHprL: " << x << ", " << y << ", " << z << ", " << h << ", " << p << ", " << r << l << endl;
  DCPacker packer;
  begin_send_update(packer, "setSmPosHprL");
  packer.pack_uint64(_currL[0]);
  packer.pack_double(x);
  packer.pack_double(y);
  packer.pack_double(z);
  packer.pack_double(h);
  packer.pack_double(p);
  packer.pack_double(r);
  finish_send_update(packer);
}

