// Filename: movingPartMatrix.h
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef MOVINGPARTMATRIX_H
#define MOVINGPARTMATRIX_H

#include "pandabase.h"

#include "movingPart.h"
#include "animChannel.h"
#include "animChannelFixed.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MovingPart<ACMatrixSwitchType>);

////////////////////////////////////////////////////////////////////
//       Class : MovingPartMatrix
// Description : This is a particular kind of MovingPart that accepts
//               a matrix each frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MovingPartMatrix : public MovingPart<ACMatrixSwitchType> {
protected:
  INLINE MovingPartMatrix(const MovingPartMatrix &copy);

public:
  INLINE MovingPartMatrix(PartGroup *parent, const string &name,
                          const LMatrix4f &initial_value =
                          LMatrix4f::ident_mat());

  virtual void get_blend_value(const PartBundle *root);

protected:
  INLINE MovingPartMatrix(void);

public:
  static void register_with_read_factory(void);

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



