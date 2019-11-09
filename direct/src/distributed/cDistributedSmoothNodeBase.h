/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cDistributedSmoothNodeBase.h
 * @author drose
 * @date 2004-09-03
 */

#ifndef CDISTRIBUTEDSMOOTHNODEBASE_H
#define CDISTRIBUTEDSMOOTHNODEBASE_H

#include "directbase.h"
#include "nodePath.h"
#include "dcbase.h"
#include "dcPacker.h"
#include "clockObject.h"

class DCClass;
class CConnectionRepository;

/**
 * This class defines some basic methods of DistributedSmoothNodeBase which
 * have been moved into C++ as a performance optimization.
 */
class CDistributedSmoothNodeBase {
PUBLISHED:
  CDistributedSmoothNodeBase();
  ~CDistributedSmoothNodeBase();

  INLINE void
  set_repository(CConnectionRepository *repository,
                 bool is_ai, CHANNEL_TYPE ai_id);

#ifdef HAVE_PYTHON
  INLINE void
  set_clock_delta(PyObject *clock_delta);
#endif

  void initialize(const NodePath &node_path, DCClass *dclass,
                  CHANNEL_TYPE do_id);

  void send_everything();

  void broadcast_pos_hpr_full();
  void broadcast_pos_hpr_xyh();
  void broadcast_pos_hpr_xy();

  void set_curr_l(uint64_t l);
  void print_curr_l();

private:
  INLINE static bool only_changed(int flags, int compare);

  INLINE void d_setSmStop();
  INLINE void d_setSmH(PN_stdfloat h);
  INLINE void d_setSmZ(PN_stdfloat z);
  INLINE void d_setSmXY(PN_stdfloat x, PN_stdfloat y);
  INLINE void d_setSmXZ(PN_stdfloat x, PN_stdfloat z);
  INLINE void d_setSmPos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE void d_setSmHpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  INLINE void d_setSmXYH(PN_stdfloat x, PN_stdfloat y, PN_stdfloat h);
  INLINE void d_setSmXYZH(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h);
  INLINE void d_setSmPosHpr(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);
  INLINE void d_setSmPosHprL(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat h, PN_stdfloat p, PN_stdfloat r, uint64_t l);

  void begin_send_update(DCPacker &packer, const std::string &field_name);
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

  CConnectionRepository *_repository;
  bool _is_ai;
  CHANNEL_TYPE _ai_id;
#ifdef HAVE_PYTHON
  PyObject *_clock_delta;
#endif

  LPoint3 _store_xyz;
  LVecBase3 _store_hpr;
  bool _store_stop;
  // contains most recently sent location info as index 0, index 1 contains
  // most recently set location info
  uint64_t _currL[2];
};

#include "cDistributedSmoothNodeBase.I"

#endif  // CDISTRIBUTEDSMOOTHNODEBASE_H
