// Filename: config_lerp.cxx
// Created by:  frang (30May00)
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


#include "lerp.h"
#include "lerpfunctor.h"

#include "config_lerp.h"

Configure(config_lerp);
NotifyCategoryDef(lerp, "");

ConfigureFn(config_lerp) {
  Lerp::init_type();
  AutonomousLerp::init_type();

  LerpFunctor::init_type();
  SimpleLerpFunctor<int>::init_type();
  SimpleLerpFunctor<float>::init_type();
  SimpleLerpFunctor<LPoint2f>::init_type();
  SimpleLerpFunctor<LPoint3f>::init_type();
  SimpleLerpFunctor<LPoint4f>::init_type();
  SimpleLerpFunctor<LVecBase2f>::init_type();
  SimpleLerpFunctor<LVecBase3f>::init_type();
  SimpleLerpFunctor<LVecBase4f>::init_type();
  SimpleLerpFunctor<LVector2f>::init_type();
  SimpleLerpFunctor<LVector3f>::init_type();
  SimpleLerpFunctor<LVector4f>::init_type();
  SimpleQueryLerpFunctor<int>::init_type();
  SimpleQueryLerpFunctor<float>::init_type();
  SimpleQueryLerpFunctor<LPoint2f>::init_type();
  SimpleQueryLerpFunctor<LPoint3f>::init_type();
  SimpleQueryLerpFunctor<LPoint4f>::init_type();
  SimpleQueryLerpFunctor<LVecBase2f>::init_type();
  SimpleQueryLerpFunctor<LVecBase3f>::init_type();
  SimpleQueryLerpFunctor<LVecBase4f>::init_type();
  SimpleQueryLerpFunctor<LVector2f>::init_type();
  SimpleQueryLerpFunctor<LVector3f>::init_type();
  SimpleQueryLerpFunctor<LVector4f>::init_type();
  MultiLerpFunctor::init_type();

  LerpBlendType::init_type();
  EaseInBlendType::init_type();
  EaseOutBlendType::init_type();
  EaseInOutBlendType::init_type();
  NoBlendType::init_type();
}
