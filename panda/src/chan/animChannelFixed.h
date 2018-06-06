/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelFixed.h
 * @author drose
 * @date 1999-02-24
 */

#ifndef ANIMCHANNELFIXED_H
#define ANIMCHANNELFIXED_H

#include "pandabase.h"

#include "animChannel.h"


/**
 * This template class is a special kind of AnimChannel that always returns
 * just one fixed value.  It is a special channel, in that it need not be
 * assigned within a hierarchy.  It may stand alone, so that it may be created
 * on-the-fly for parts that need default anims to bind against.
 */
template<class SwitchType>
class AnimChannelFixed : public AnimChannel<SwitchType> {
public:
  typedef typename AnimChannel<SwitchType>::ValueType ValueType;

protected:
  INLINE AnimChannelFixed(AnimGroup *parent, const AnimChannelFixed<SwitchType> &copy);

public:
  INLINE AnimChannelFixed(const std::string &name, const ValueType &value);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, ValueType &value);

  virtual void output(std::ostream &out) const;

  ValueType _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannel<SwitchType>::init_type();
    register_type(_type_handle, SwitchType::get_fixed_channel_type_name(),
                  AnimChannel<SwitchType>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, AnimChannelFixed<ACMatrixSwitchType>);
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, AnimChannelFixed<ACScalarSwitchType>);

#include "animChannelFixed.I"

#endif
