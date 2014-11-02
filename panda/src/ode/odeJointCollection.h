// Filename: odeJointCollection.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODEJOINTCOLLECTION_H
#define ODEJOINTCOLLECTION_H

#include "odeJoint.h"

////////////////////////////////////////////////////////////////////
//       Class : OdeJointCollection
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeJointCollection {
PUBLISHED:
  OdeJointCollection();
  OdeJointCollection(const OdeJointCollection &copy);
  void operator = (const OdeJointCollection &copy);
  INLINE ~OdeJointCollection();

  void add_joint(const OdeJoint &joint);
  bool remove_joint(const OdeJoint &joint);
  void add_joints_from(const OdeJointCollection &other);
  void remove_joints_from(const OdeJointCollection &other);
  void remove_duplicate_joints();
  bool has_joint(const OdeJoint &joint) const;
  void clear();

  bool is_empty() const;
  int get_num_joints() const;
  OdeJoint get_joint(int index) const;
  MAKE_SEQ(get_joints, get_num_joints, get_joint);
  OdeJoint operator [] (int index) const;
  int size() const;
  INLINE void operator += (const OdeJointCollection &other);
  INLINE OdeJointCollection operator + (const OdeJointCollection &other) const;
  
private:  
  typedef PTA(OdeJoint) Joints;
  Joints _joints;
};

#include "odeJointCollection.I"

#endif
