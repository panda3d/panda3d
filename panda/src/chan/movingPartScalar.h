// Filename: movingPartScalar.h
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

#ifndef MOVINGPARTSCALAR_H
#define MOVINGPARTSCALAR_H

#include "pandabase.h"

#include "movingPart.h"
#include "animChannel.h"
#include "animChannelFixed.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MovingPart<ACScalarSwitchType>);

////////////////////////////////////////////////////////////////////
//       Class : MovingPartScalar
// Description : This is a particular kind of MovingPart that accepts
//               a scalar each frame.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MovingPartScalar : public MovingPart<ACScalarSwitchType> {
protected:
  INLINE MovingPartScalar(const MovingPartScalar &copy);

public:
  INLINE MovingPartScalar(PartGroup *parent, const string &name,
                          const float &initial_value = 0);

  virtual void get_blend_value(const PartBundle *root);

protected:
  INLINE MovingPartScalar(void);

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_MovingPartScalar(const FactoryParams &params);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovingPart<ACScalarSwitchType>::init_type();
    AnimChannelFixed<ACScalarSwitchType>::init_type();
    register_type(_type_handle, "MovingPartScalar",
                  MovingPart<ACScalarSwitchType>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "movingPartScalar.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif



