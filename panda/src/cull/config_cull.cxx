/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_cull.cxx
 * @author drose
 * @date 2006-03-23
 */

#include "config_cull.h"

#include "cullBinBackToFront.h"
#include "cullBinFixed.h"
#include "cullBinFrontToBack.h"
#include "cullBinStateSorted.h"
#include "cullBinUnsorted.h"

#include "cullBinManager.h"
#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_CULL)
  #error Buildsystem error: BUILDING_PANDA_CULL not defined
#endif

ConfigureDef(config_cull);
NotifyCategoryDef(cull, "");

ConfigureFn(config_cull) {
  init_libcull();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libcull() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  CullBinBackToFront::init_type();
  CullBinFixed::init_type();
  CullBinFrontToBack::init_type();
  CullBinStateSorted::init_type();
  CullBinUnsorted::init_type();

  CullBinManager *bin_manager = CullBinManager::get_global_ptr();
  bin_manager->register_bin_type(CullBinManager::BT_unsorted,
                                 CullBinUnsorted::make_bin);
  bin_manager->register_bin_type(CullBinManager::BT_state_sorted,
                                 CullBinStateSorted::make_bin);
  bin_manager->register_bin_type(CullBinManager::BT_back_to_front,
                                 CullBinBackToFront::make_bin);
  bin_manager->register_bin_type(CullBinManager::BT_front_to_back,
                                 CullBinFrontToBack::make_bin);
  bin_manager->register_bin_type(CullBinManager::BT_fixed,
                                 CullBinFixed::make_bin);
}
