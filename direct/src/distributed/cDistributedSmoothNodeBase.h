// Filename: cDistributedSmoothNodeBase.h
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

#ifndef CDISTRIBUTEDSMOOTHNODEBASE_H
#define CDISTRIBUTEDSMOOTHNODEBASE_H

#include "directbase.h"
#include "nodePath.h"
#include "dcbase.h"
#include "dcPacker.h"
#include "dcPython.h"  // to pick up Python.h

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

  INLINE static void
  set_clock_delta(PyObject *clock_delta);

  void initialize(const NodePath &node_path, DCClass *dclass,
                  CHANNEL_TYPE do_id);

  void send_everything();

  void broadcast_pos_hpr_full();
  void broadcast_pos_hpr_xyh();

private:
  INLINE static bool only_changed(int flags, int compare);

  INLINE void d_setSmStop();
  INLINE void d_setSmH(float h);
  INLINE void d_setSmXY(float x, float y);
  INLINE void d_setSmXZ(float x, float z);
  INLINE void d_setSmPos(float x, float y, float z);
  INLINE void d_setSmHpr(float h, float p, float r);
  INLINE void d_setSmXYH(float x, float y, float h);
  INLINE void d_setSmXYZH(float x, float y, float z, float h);
  INLINE void d_setSmPosHpr(float x, float y, float z, float h, float p, float r);

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
  static PyObject *_clock_delta;

  LPoint3f _store_xyz;
  LVecBase3f _store_hpr;
  bool _store_stop;
};

#include "cDistributedSmoothNodeBase.I"

#endif  // CDISTRIBUTEDSMOOTHNODEBASE_H
