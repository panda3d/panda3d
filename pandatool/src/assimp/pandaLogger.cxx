/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaLogger.cxx
 * @author rdb
 * @date 2011-05-05
 */

#include "pandaLogger.h"

#include <assimp/DefaultLogger.hpp>

PandaLogger *PandaLogger::_ptr = nullptr;

/**
 * Makes sure there's a global PandaLogger object and makes sure that it is
 * Assimp's default logger.
 */
void PandaLogger::
set_default() {
  if (_ptr == nullptr) {
    _ptr = new PandaLogger;
  }
  if (_ptr != Assimp::DefaultLogger::get()) {
    Assimp::DefaultLogger::set(_ptr);
  }
}

/**
 *
 */
void PandaLogger::OnDebug(const char *message) {
  if (assimp_cat.is_debug()) {
    assimp_cat.debug() << message << "\n";
  }
}

/**
 *
 */
void PandaLogger::OnError(const char *message) {
  assimp_cat.error() << message << "\n";
}

/**
 *
 */
void PandaLogger::OnInfo(const char *message) {
  assimp_cat.info() << message << "\n";
}

/**
 *
 */
void PandaLogger::OnWarn(const char *message) {
  assimp_cat.warning() << message << "\n";
}
