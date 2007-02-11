// Filename: odeJointCollection.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODEJOINTCOLLECTION_H
#define ODEJOINTCOLLECTION_H

class OdeJoint;

////////////////////////////////////////////////////////////////////
//       Class : OdeJointCollection
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeJointCollection {
PUBLISHED:
  OdeJointCollection();
  OdeJointCollection(const OdeJointCollection &copy);
  void operator = (const OdeJointCollection &copy);
  INLINE ~OdeJointCollection() {};

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
  OdeJoint operator [] (int index) const;
  
private:  
  typedef PTA(OdeJoint) Joints;
  Joints _joints;
};

#endif
