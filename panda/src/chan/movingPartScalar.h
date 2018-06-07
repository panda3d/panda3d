/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file movingPartScalar.h
 * @author drose
 * @date 1999-02-23
 */

#ifndef MOVINGPARTSCALAR_H
#define MOVINGPARTSCALAR_H

#include "pandabase.h"

#include "movingPart.h"
#include "animChannel.h"
#include "animChannelFixed.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, MovingPart<ACScalarSwitchType>);

/**
 * This is a particular kind of MovingPart that accepts a scalar each frame.
 */
class EXPCL_PANDA_CHAN MovingPartScalar : public MovingPart<ACScalarSwitchType> {
protected:
  INLINE MovingPartScalar(const MovingPartScalar &copy);

public:
  INLINE MovingPartScalar(PartGroup *parent, const std::string &name,
                          const PN_stdfloat &default_value = 0);
  virtual ~MovingPartScalar();

  virtual void get_blend_value(const PartBundle *root);

  virtual bool apply_freeze_scalar(PN_stdfloat value);
  virtual bool apply_control(PandaNode *node);

protected:
  INLINE MovingPartScalar();

public:
  static void register_with_read_factory();

  static TypedWritable *make_MovingPartScalar(const FactoryParams &params);

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
    MovingPart<ACScalarSwitchType>::init_type();
    AnimChannelFixed<ACScalarSwitchType>::init_type();
    register_type(_type_handle, "MovingPartScalar",
                  MovingPart<ACScalarSwitchType>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "movingPartScalar.I"

#endif
