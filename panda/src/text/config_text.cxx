// Filename: config_text.cxx
// Created by:  drose (02Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_text.h"
#include "textNode.h"

#include <dconfig.h>

Configure(config_text);

NotifyCategory &text_cat = *Notify::ptr()->get_category(":text");

ConfigureFn(config_text) {
  TextNode::init_type();
}

