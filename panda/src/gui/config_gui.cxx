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
