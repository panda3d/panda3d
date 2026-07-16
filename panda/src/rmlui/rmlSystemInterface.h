/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlSystemInterface.h
 * @author rdb
 * @date 2011-11-03
 */

#ifndef RML_SYSTEM_INTERFACE_H
#define RML_SYSTEM_INTERFACE_H

#include "config_rmlui.h"

#ifndef CPPPARSER
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Log.h>
#endif

/**
 * Implementation of SystemInterface that redirects to Panda's clock and notify.
 * SetMouseCursor fires a Panda Messenger event "rmlui-cursor" with the cursor
 * name so Python can react without polling.
 */
class RmlSystemInterface
#ifndef CPPPARSER
  : public Rml::SystemInterface
#endif
{
public:
  double GetElapsedTime() override;
  bool LogMessage(Rml::Log::Type type, const Rml::String &message) override;
#ifndef CPPPARSER
  void SetMouseCursor(const Rml::String &cursor_name) override;
#endif
};

#endif
