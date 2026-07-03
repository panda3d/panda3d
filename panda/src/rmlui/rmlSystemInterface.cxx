/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlSystemInterface.cxx
 * @author rdb
 * @date 2011-11-03
 */

#include "rmlSystemInterface.h"
#include "clockObject.h"
#include "throw_event.h"

/**
 * Returns the elapsed real time in seconds via Panda's global clock.
 */
double RmlSystemInterface::
GetElapsedTime() {
  return ClockObject::get_global_clock()->get_real_time();
}

/**
 * Fires "rmlui-cursor" on the Panda Messenger with the cursor name as a string
 * parameter, allowing Python to react without polling.  An empty name (reset)
 * is normalised to "default".
 */
void RmlSystemInterface::
SetMouseCursor(const Rml::String &cursor_name) {
  const std::string name = cursor_name.empty() ? "default" : cursor_name;
  throw_event("rmlui-cursor", EventParameter(name));
}

/**
 * Redirects an RmlUi log message to the appropriate Panda3D notify category.
 */
bool RmlSystemInterface::
LogMessage(Rml::Log::Type type, const Rml::String &message) {
  switch (type) {
  case Rml::Log::LT_ALWAYS:
  case Rml::Log::LT_ERROR:
  case Rml::Log::LT_ASSERT:
    rmlui_cat->error() << message << "\n";
    break;
  case Rml::Log::LT_WARNING:
    rmlui_cat->warning() << message << "\n";
    break;
  case Rml::Log::LT_INFO:
    rmlui_cat->info() << message << "\n";
    break;
  case Rml::Log::LT_DEBUG:
    rmlui_cat->debug() << message << "\n";
    break;
  default:
    break;
  }
  return true;
}
