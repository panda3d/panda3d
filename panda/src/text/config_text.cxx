// Filename: config_text.cxx
// Created by:  drose (02Mar00)
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

#include "config_text.h"
#include "staticTextFont.h"
#include "dynamicTextFont.h"
#include "dynamicTextPage.h"
#include "textFont.h"
#include "textNode.h"

#include <dconfig.h>

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  StaticTextFont::init_type();
  DynamicTextFont::init_type();
  DynamicTextPage::init_type();
  TextFont::init_type();
  TextNode::init_type();
}

const bool flatten_text = config_text.GetBool("flatten-text", true);
