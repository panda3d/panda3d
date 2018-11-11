/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaLogger.h
 * @author rdb
 * @date 2011-05-05
 */

#ifndef PANDALOGGER_H
#define PANDALOGGER_H

#include "config_assimp.h"

#include <assimp/Logger.hpp>

/**
 * Custom implementation of Assimp::Logger.  It simply wraps around the
 * assimp_cat methods.
 */
class PandaLogger : public Assimp::Logger {
public:
  static void set_default();

protected:
  INLINE bool attachStream(Assimp::LogStream*, unsigned int) {
    return false;
  };
  INLINE bool detatchStream(Assimp::LogStream*, unsigned int) {
    return false;
  };

  void OnDebug(const char *message);
  void OnError(const char *message);
  void OnInfo(const char *message);
  void OnWarn(const char *message);

private:
  static PandaLogger *_ptr;
};

#endif
