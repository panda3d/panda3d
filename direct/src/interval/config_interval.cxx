/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_interval.cxx
 * @author drose
 * @date 2002-08-27
 */

#include "config_interval.h"
#include "cInterval.h"
#include "cConstraintInterval.h"
#include "cConstrainTransformInterval.h"
#include "cConstrainPosInterval.h"
#include "cConstrainHprInterval.h"
#include "cConstrainPosHprInterval.h"
#include "cLerpInterval.h"
#include "cLerpNodePathInterval.h"
#include "cLerpAnimEffectInterval.h"
#include "cMetaInterval.h"
#include "showInterval.h"
#include "hideInterval.h"
#include "waitInterval.h"
#include "lerpblend.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_DIRECT_INTERVAL)
  #error Buildsystem error: BUILDING_DIRECT_INTERVAL not defined
#endif

Configure(config_interval);
NotifyCategoryDef(interval, "");

ConfigureFn(config_interval) {
  init_libinterval();
}

ConfigVariableDouble interval_precision
("interval-precision", 1000.0,
 PRC_DESC("Set this to the default value for set_precision() for each "
          "CMetaInterval created."));

ConfigVariableBool verify_intervals
("verify-intervals", false,
 PRC_DESC("Set this true to generate an assertion failure if interval "
          "functions are called out-of-order."));


/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libinterval() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CInterval::init_type();
  CConstraintInterval::init_type();
  CConstrainTransformInterval::init_type();
  CConstrainPosInterval::init_type();
  CConstrainHprInterval::init_type();
  CConstrainPosHprInterval::init_type();
  CLerpInterval::init_type();
  CLerpNodePathInterval::init_type();
  CLerpAnimEffectInterval::init_type();
  CMetaInterval::init_type();
  ShowInterval::init_type();
  HideInterval::init_type();
  WaitInterval::init_type();
  LerpBlendType::init_type();
  EaseInBlendType::init_type();
  EaseOutBlendType::init_type();
  EaseInOutBlendType::init_type();
  NoBlendType::init_type();
}
