// Filename: config_chat.cxx
// Created by:  drose (04May00)
// 
////////////////////////////////////////////////////////////////////

#include "config_chat.h"
#include "chatInput.h"

#include <dconfig.h>

Configure(config_chat);
NotifyCategoryDef(chat, "");

ConfigureFn(config_chat) {
  ChatInput::init_type();
}
