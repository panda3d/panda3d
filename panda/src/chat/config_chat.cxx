// Filename: config_chat.cxx
// Created by:  drose (04May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifdef WIN32_VC
#include "chat_headers.h"
#endif

#pragma hdrstop

#ifndef WIN32_VC
#include "chatInput.h"
#endif

#include "config_chat.h"
#include <dconfig.h>

Configure(config_chat);
NotifyCategoryDef(chat, "");

ConfigureFn(config_chat) {
  ChatInput::init_type();
}
