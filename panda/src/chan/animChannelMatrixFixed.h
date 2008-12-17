// Filename: animChannelMatrixFixed.h
// Created by:  drose (19Jan06)
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

#ifndef ANIMCHANNELMATRIXFIXED_H
#define ANIMCHANNELMATRIXFIXED_H

#include "pandabase.h"

#include "animChannel.h"
#include "luse.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA_CHAN, EXPTP_PANDA_CHAN, AnimChannel<ACMatrixSwitchType>);

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelMatrixFixed
// Description : A specialization on AnimChannel to add all the
//               special matrix component operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_CHAN AnimChannelMatrixFixed : public AnimChannel<ACMatrixSwitchType> {
protected:
  AnimChannelMatrixFixed(AnimGroup *parent, const AnimChannelMatrixFixed &copy);

public:
  AnimChannelMatrixFixed(const string &name, const LVecBase3f &pos, const LVecBase3f &hpr, const LVecBase3f &scale);

  virtual bool has_changed(int last_frame, double last_frac, 
                           int this_frame, double this_frac);
  virtual void get_value(int frame, LMatrix4f &value);
  virtual void get_value_no_scale_shear(int frame, LMatrix4f &value);
  virtual void get_scale(int frame, LVecBase3f &scale);
  virtual void get_hpr(int frame, LVecBase3f &hpr);
  virtual void get_quat(int frame, LQuaternionf &quat);
  virtual void get_pos(int frame, LVecBase3f &pos);
  virtual void get_shear(int frame, LVecBase3f &shear);

  virtual void output(ostream &out) const;

private:
  LVecBase3f _pos, _hpr, _scale;

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



