// Filename: config_gui.cxx
// Created by:  cary (26Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "config_gui.h"
#include "guiLabel.h"
#include "guiRegion.h"
#include "guiItem.h"
#include "guiSign.h"
#include "guiRollover.h"
#include "guiButton.h"
#include "guiFrame.h"

#include <dconfig.h>

Configure(config_gui);
NotifyCategoryDef(gui, "");

ConfigureFn(config_gui) {
  GuiLabel::init_type();
  GuiRegion::init_type();
  GuiItem::init_type();
  GuiSign::init_type();
  GuiRollover::init_type();
  GuiButton::init_type();
  GuiFrame::init_type();
}

float simple_text_margin_top =
  config_gui.GetFloat("simple-text-margin-top", 0.02);
float simple_text_margin_bottom =
  config_gui.GetFloat("simple-text-margin-bottom", 0.05);
float simple_text_margin_left =
  config_gui.GetFloat("simple-text-margin-left", 0.02);
float simple_text_margin_right =
  config_gui.GetFloat("simple-text-margin-right", 0.02);
