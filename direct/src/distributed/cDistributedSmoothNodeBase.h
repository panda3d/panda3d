// Filename: cDistributedSmoothNodeBase.h
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

#ifndef CDISTRIBUTEDSMOOTHNODEBASE_H
#define CDISTRIBUTEDSMOOTHNODEBASE_H

#include "directbase.h"
#include "nodePath.h"
#include "dcbase.h"
#include "dcPacker.h"
#include "dcPython.h"  // to pick up Python.h
#include "clockObject.h"

class DCClass;
class CConnectionRepository;

////////////////////////////////////////////////////////////////////
//       Class : CDistributedSmoothNodeBase
// Description : This class defines some basic methods of
//               DistributedSmoothNodeBase which have been moved into
//               C++ as a performance optimization.
////////////////////////////////////////////////////////////////////
class EXPCL_DIRECT CDistributedSmoothNodeBase {
PUBLISHED:
  CDistributedSmoothNodeBase();
  ~CDistributedSmoothNodeBase();
  
  INLINE static void
  set_repository(CConnectionRepository *repository,
                 bool is_ai, CHANNEL_TYPE ai_id);

#ifdef HAVE_PYTHON
  INLINE static void
  set_clock_delta(PyObject *clock_delta);
#endif

  void initialize(const NodePath &node_path, DCClass *dclass,
                  CHANNEL_TYPE do_id);

  void send_everything();

  void broadcast_pos_hpr_full();
  void broadcast_pos_hpr_xyh();
  void broadcast_pos_hpr_xy();

  void set_curr_l(PN_uint64 l);
  void print_curr_l();

private:
  INLINE static bool only_changed(int flags, int compare);

  INLINE void d_setSmStop();
  INLINE void d_setSmH(float h);
  INLINE void d_setSmZ(float z);
  INLINE void d_setSmXY(float x, float y);
  INLINE void d_setSmXZ(float x, float z);
  INLINE void d_setSmPos(float x, float y, float z);
  INLINE void d_setSmHpr(float h, float p, float r);
  INLINE void d_setSmXYH(float x, float y, float h);
  INLINE void d_setSmXYZH(float x, float y, float z, float h);
  INLINE void d_setSmPosHpr(float x, float y, float z, float h, float p, float r);
  INLINE void d_setSmPosHprL(float x, float y, float z, float h, float p, float r, PN_uint64 l);

  void begin_send_update(DCPacker &packer, const string &field_name);
  void finish_send_update(DCPacker &packer);

  enum Flags {
    F_new_x     = 0x01,
    F_new_y     = 0x02,
    F_new_z     = 0x04,
    F_new_h     = 0x08,
    F_new_p     = 0x10,
    F_new_r     = 0x20,
  };

  NodePath _node_path;
  DCClass *_dclass;
  CHANNEL_TYPE _do_id;

  static CConnectionRepository *_repository;
  static bool _is_ai;
  static CHANNEL_TYPE _ai_id;
#ifdef HAVE_PYTHON
  static PyObject *_clock_delta;
#endif

  LPoint3f _store_xyz;
  LVecBase3f _store_hpr;
  bool _store_stop;
  // contains most recently sent location info as
  // index 0, index 1 contains most recently set location info
  PN_uint64 _currL[2];
};

#include "cDistributedSmoothNodeBase.I"

#endif  // CDISTRIBUTEDSMOOTHNODEBASE_H
