// Filename: config_gui.cxx
// Created by:  cary (26Oct00)
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

#include "config_gui.h"
#include "guiLabel.h"
#include "guiItem.h"
#include "guiSign.h"
#include "guiRollover.h"
#include "guiButton.h"
#include "guiFrame.h"
#include "guiListBox.h"
#include "guiBehavior.h"
#include "guiBackground.h"
#include "guiChooser.h"
#include "guiCollection.h"

#include <dconfig.h>

Configure(config_gui);
NotifyCategoryDef(gui, "");

ConfigureFn(config_gui) {
  GuiLabel::init_type();
  GuiItem::init_type();
  GuiBehavior::init_type();
  GuiBehavior::BehaviorFunctor::init_type();
  GuiSign::init_type();
  GuiRollover::init_type();
  GuiButton::init_type();
  GuiFrame::init_type();
  GuiListBox::init_type();
  GuiBackground::init_type();
  GuiChooser::init_type();
  GuiCollection::init_type();
}

float simple_text_margin_top =
  config_gui.GetFloat("simple-text-margin-top", 0.02);
float simple_text_margin_bottom =
  config_gui.GetFloat("simple-text-margin-bottom", 0.05);
float simple_text_margin_left =
  config_gui.GetFloat("simple-text-margin-left", 0.02);
float simple_text_margin_right =
  config_gui.GetFloat("simple-text-margin-right", 0.02);
