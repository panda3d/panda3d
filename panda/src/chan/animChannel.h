/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannel.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef ANIMCHANNEL_H
#define ANIMCHANNEL_H

#include "pandabase.h"

#include "animChannelBase.h"

#include "luse.h"

/**
 * This template class is the parent class for all kinds of AnimChannels that
 * return different values.
 */
template<class SwitchType>
class AnimChannel : public AnimChannelBase {
protected:
  // The default constructor is protected: don't try to create an AnimChannel
  // without a parent.  To create an AnimChannel hierarchy, you must first
  // create an AnimBundle, and use that to create any subsequent children.
  INLINE AnimChannel(const std::string &name = "");
  INLINE AnimChannel(AnimGroup *parent, const AnimChannel &copy);
public:
  typedef typename SwitchType::ValueType ValueType;

  INLINE AnimChannel(AnimGroup *parent, const std::string &name);
  INLINE ~AnimChannel();

PUBLISHED:
  virtual void get_value(int frame, ValueType &value)=0;

  // These transform-component methods only have meaning for matrix types.
  virtual void get_value_no_scale_shear(int frame, ValueType &value);
  virtual void get_scale(int frame, LVecBase3 &scale);
  virtual void get_hpr(int frame, LVecBase3 &hpr);
  virtual void get_quat(int frame, LQuaternion &quat);
  virtual void get_pos(int frame, LVecBase3 &pos);
  virtual void get_shear(int frame, LVecBase3 &shear);

  virtual TypeHandle get_value_type() const;

  // This class has no ReadWrite functions as it is abstract and defines no
  // new data

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannelBase::init_type();
    register_type(_type_handle, SwitchType::get_channel_type_name(),
                  AnimChannelBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};


// And now we instantiate a few useful types.

class EXPCL_PANDA_CHAN ACMatrixSwitchType {
public:
  typedef LMatrix4 ValueType;
  static const char *get_channel_type_name() { return "AnimChannelMatrix"; }
  static const char *get_fixed_channel_type_name() { return "AnimChannelFixed<LMatrix4>"; }
  static const char *get_part_type_name() { return "MovingPart<LMatrix4>"; }
  static void output_value(std::ostream &out, const ValueType &value);

  static void write_datagram(Datagram &dest, ValueType& me)
  {
    me.write_datagram(dest);
  }
  static void read_datagram(DatagramIterator &source, ValueType& me)
  {
    me.read_datagram(source);
  }
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, AnimChannel<ACMatrixSwitchType>);
typedef AnimChannel<ACMatrixSwitchType> AnimChannelMatrix;


class EXPCL_PANDA_CHAN ACScalarSwitchType {
public:
  typedef PN_stdfloat ValueType;
  static const char *get_channel_type_name() { return "AnimChannelScalar"; }
  static const char *get_fixed_channel_type_name() { return "AnimChannelScalarFixed"; }
  static const char *get_part_type_name() { return "MovingPart<PN_stdfloat>"; }
  static void output_value(std::ostream &out, ValueType value) {
    out << value;
  }
  static void write_datagram(Datagram &dest, ValueType& me)
  {
    dest.add_stdfloat(me);
  }
  static void read_datagram(DatagramIterator &source, ValueType& me)
  {
    me = source.get_stdfloat();
  }
};

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, AnimChannel<ACScalarSwitchType>);
typedef AnimChannel<ACScalarSwitchType> AnimChannelScalar;


#include "animChannel.I"

#endif
