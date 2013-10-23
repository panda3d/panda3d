// Filename: rocketSystemInterface.h
// Created by:  rdb (03Nov11)
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

#ifndef ROCKET_SYSTEM_INTERFACE_H
#define ROCKET_SYSTEM_INTERFACE_H

#include "config_rocket.h"

#include <Rocket/Core/SystemInterface.h>
#include <Rocket/Core/Log.h>

////////////////////////////////////////////////////////////////////
//       Class : RocketSystemInterface
// Description : This is an implementation of SystemInterface
//               that redirects the log output to Panda's notify
//               system.
////////////////////////////////////////////////////////////////////
class RocketSystemInterface : public Rocket::Core::SystemInterface {
public:
  float GetElapsedTime();
  bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);
};

#endif
