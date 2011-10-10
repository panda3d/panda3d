// Filename: movingPartMatrix.h
// Created by:  drose (23Feb99)
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

#ifndef MOVINGPARTMATRIX_H
#define MOVINGPARTMATRIX_H

#include "pandabase.h"

#include "movingPart.h"
#include "animChannel.h"
#include "animChannelFixed.h"
#include "cmath.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, MovingPart<ACMatrixSwitchType>);

////////////////////////////////////////////////////////////////////
//       Class : MovingPartMatrix
// Description : This is a particular kind of MovingPart that accepts
//               a matrix each frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAN MovingPartMatrix : public MovingPart<ACMatrixSwitchType> {
protected:
  INLINE MovingPartMatrix(const MovingPartMatrix &copy);

public:
  INLINE MovingPartMatrix(PartGroup *parent, const string &name,
                          const LMatrix4 &default_value);
  virtual ~MovingPartMatrix();

  virtual AnimChannelBase *make_default_channel() const;
  virtual void get_blend_value(const PartBundle *root);

  virtual bool apply_freeze_matrix(const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale);
  virtual bool apply_control(PandaNode *node);

protected:
  INLINE MovingPartMatrix();

public:
  static void register_with_read_factory();

  static TypedWritable *make_MovingPartMatrix(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    MovingPart<ACMatrixSwitchType>::init_type();
    AnimChannelFixed<ACMatrixSwitchType>::init_type();
    register_type(_type_handle, "MovingPartMatrix",
                  MovingPart<ACMatrixSwitchType>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "movingPartMatrix.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif



