// Filename: config_text.cxx
// Created by:  drose (02Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_text.h"
#include "textFont.h"
#include "textNode.h"

#include <dconfig.h>

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  TextFont::init_type();
  TextNode::init_type();
}

const bool flatten_text = config_text.GetBool("flatten-text", true);
