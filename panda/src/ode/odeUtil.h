// Filename: odeUtil.h
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

#ifndef ODEUTIL_H
#define ODEUTIL_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeJointCollection.h"

class OdeBody;
class OdeJoint;

////////////////////////////////////////////////////////////////////
//       Class : OdeUtil
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAODE OdeUtil {
PUBLISHED:
  static OdeJoint get_connecting_joint(const OdeBody &body1,
				       const OdeBody &body2);
  static OdeJointCollection get_connecting_joint_list(const OdeBody &body1,
						      const OdeBody &body2);
  static int are_connected(const OdeBody &body1,
                           const OdeBody &body2);
  static int are_connected_excluding(const OdeBody &body1,
	                             const OdeBody &body2,
	                             const int joint_type);

  static dReal OC_infinity;  

  // RAU we can't access OC_infinity as constants are not exposed in python
  static dReal get_infinity() {return OC_infinity;};
};

#endif
