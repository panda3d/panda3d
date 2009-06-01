// Filename: physxRay.h
// Created by:  pratt (Dec 12, 2007)
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

#ifndef PHYSXRAY_H
#define PHYSXRAY_H

#ifdef HAVE_PHYSX

#include "pandabase.h"

#include "physx_enumerations.h"
#include "physxManager.h"
#include "luse.h"

#include "NxPhysics.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxRay
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxRay {
PUBLISHED:
  PhysxRay();
  PhysxRay(const LVecBase3f & _orig, const LVecBase3f & _dir);
  ~PhysxRay();


  LVecBase3f get_dir() const;
  LVecBase3f get_orig() const;

  void set_dir(LVecBase3f value);
  void set_orig(LVecBase3f value);

public:
  NxRay *nRay;
};

#include "physxRay.I"

#endif // HAVE_PHYSX

#endif // PHYSXRAY_H
