// Filename: animChannelFixed.h
// Created by:  drose (24Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCHANNELFIXED_H
#define ANIMCHANNELFIXED_H

#include <pandabase.h>

#include "animChannel.h"


////////////////////////////////////////////////////////////////////
//       Class : AnimChannelFixed
// Description : This template class is a special kind of AnimChannel
//               that always returns just one fixed value.  It is a
//               special channel, in that it need not be assigned
//               within a hierarchy.  It may stand alone, so that it
//               may be created on-the-fly for parts that need default
//               anims to bind against.
////////////////////////////////////////////////////////////////////
template<class SwitchType>
class EXPCL_PANDA AnimChannelFixed : public AnimChannel<SwitchType> {
public:
  INLINE AnimChannelFixed(AnimGroup *parent, const string &name, const ValueType &value);
  INLINE AnimChannelFixed(const string &name, const ValueType &value);

  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, ValueType &value);

  virtual void output(ostream &out) const;

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


#include "animChannelFixed.I"

#endif
