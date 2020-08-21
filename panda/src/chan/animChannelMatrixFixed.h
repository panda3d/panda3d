/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixFixed.h
 * @author drose
 * @date 2006-01-19
 */

#ifndef ANIMCHANNELMATRIXFIXED_H
#define ANIMCHANNELMATRIXFIXED_H

#include "pandabase.h"

#include "animChannel.h"
#include "luse.h"

/**
 * A specialization on AnimChannel to add all the special matrix component
 * operations.
 */
class EXPCL_PANDA_CHAN AnimChannelMatrixFixed : public AnimChannel<ACMatrixSwitchType> {
protected:
  AnimChannelMatrixFixed(AnimGroup *parent, const AnimChannelMatrixFixed &copy);

public:
  AnimChannelMatrixFixed(const std::string &name, const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, LMatrix4 &value);
  virtual void get_value_no_scale_shear(int frame, LMatrix4 &value);
  virtual void get_scale(int frame, LVecBase3 &scale);
  virtual void get_hpr(int frame, LVecBase3 &hpr);
  virtual void get_quat(int frame, LQuaternion &quat);
  virtual void get_pos(int frame, LVecBase3 &pos);
  virtual void get_shear(int frame, LVecBase3 &shear);

  virtual void output(std::ostream &out) const;

private:
  LVecBase3 _pos, _hpr, _scale;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannel<ACMatrixSwitchType>::init_type();
    register_type(_type_handle, "AnimChannelMatrixFixed",
                  AnimChannel<ACMatrixSwitchType>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelMatrixFixed.I"

#endif
