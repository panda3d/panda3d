/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketSystemInterface.cxx
 * @author rdb
 * @date 2011-11-03
 */

#include "rocketSystemInterface.h"
#include "clockObject.h"

/**
 * Get the number of seconds elapsed since the start of the application.
 */
float RocketSystemInterface::
GetElapsedTime() {
  ClockObject *clock = ClockObject::get_global_clock();
  //XXX not sure exactly how Rocket uses uses it, maybe get_frame_time is better?
  return clock->get_real_time();
}

/**
 * Log the specified message.  Returns true to continue execution, false to
 * break into the debugger.
 */
bool RocketSystemInterface::
LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message) {
  switch(type) {
  case Rocket::Core::Log::LT_ALWAYS:
  case Rocket::Core::Log::LT_ERROR:
  case Rocket::Core::Log::LT_ASSERT:
    rocket_cat->error() << message.CString() << "\n";
    return true;
  case Rocket::Core::Log::LT_WARNING:
    rocket_cat->warning() << message.CString() << "\n";
    return true;
  case Rocket::Core::Log::LT_INFO:
    rocket_cat->info() << message.CString() << "\n";
    return true;
  case Rocket::Core::Log::LT_DEBUG:
    rocket_cat->debug() << message.CString() << "\n";
    return true;
  case Rocket::Core::Log::LT_MAX:
    // Not really sent; just to keep compiler happy
    break;
  }
  return true;
}
