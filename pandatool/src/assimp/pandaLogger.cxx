// Filename: pandaLogger.cxx
// Created by:  rdb (05May11)
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

#include "pandaLogger.h"

#include "assimp/DefaultLogger.hpp"

PandaLogger *PandaLogger::_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: PandaLogger::set_default
//       Access: Public
//  Description: Makes sure there's a global PandaLogger object and
//               makes sure that it is Assimp's default logger.
////////////////////////////////////////////////////////////////////
void PandaLogger::
set_default() {
  if (_ptr == NULL) {
    _ptr = new PandaLogger;
  }
  if (_ptr != Assimp::DefaultLogger::get()) {
    Assimp::DefaultLogger::set(_ptr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaLogger::OnDebug
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PandaLogger::OnDebug(const char *message) {
  assimp_cat.debug() << message << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaLogger::OnError
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PandaLogger::OnError(const char *message) {
  assimp_cat.error() << message << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaLogger::OnInfo
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PandaLogger::OnInfo(const char *message) {
  assimp_cat.info() << message << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: PandaLogger::OnWarn
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PandaLogger::OnWarn(const char *message) {
  assimp_cat.warning() << message << "\n";
}
