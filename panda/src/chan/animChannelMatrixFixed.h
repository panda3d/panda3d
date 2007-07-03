// Filename: animChannelMatrixFixed.h
// Created by:  drose (19Jan06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef ANIMCHANNELMATRIXFIXED_H
#define ANIMCHANNELMATRIXFIXED_H

#include "pandabase.h"

#include "animChannel.h"
#include "luse.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, AnimChannel<ACMatrixSwitchType>);

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelMatrixFixed
// Description : A specialization on AnimChannel to add all the
//               special matrix component operations.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimChannelMatrixFixed : public AnimChannel<ACMatrixSwitchType> {
protected:
  AnimChannelMatrixFixed(AnimGroup *parent, const AnimChannelMatrixFixed &copy);

public:
  AnimChannelMatrixFixed(const string &name, const TransformState *transform);

  virtual bool has_changed(double last_frame, double this_frame);
  virtual void get_value(int frame, LMatrix4f &value);
  virtual void get_value_no_scale_shear(int frame, LMatrix4f &value);
  virtual void get_scale(int frame, LVecBase3f &scale);
  virtual void get_hpr(int frame, LVecBase3f &hpr);
  virtual void get_quat(int frame, LQuaternionf &quat);
  virtual void get_pos(int frame, LVecBase3f &pos);
  virtual void get_shear(int frame, LVecBase3f &shear);

  virtual void output(ostream &out) const;

private:
  CPT(TransformState) _transform;

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



