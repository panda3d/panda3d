// Filename: config_ribdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_ribdisplay.h"
#include "ribGraphicsPipe.h"
#include "ribGraphicsWindow.h"

#include <dconfig.h>

Configure(config_ribdisplay);
NotifyCategoryDef(ribdisplay, "display");

ConfigureFn(config_ribdisplay) {
  RIBGraphicsPipe::init_type();
  GraphicsPipe::_factory.register_factory(RIBGraphicsPipe::get_class_type(),
					  RIBGraphicsPipe::make_RIBGraphicsPipe);
  RIBGraphicsWindow::init_type();
  GraphicsWindow::_factory.register_factory(RIBGraphicsWindow::get_class_type(),
					     RIBGraphicsWindow::make_RibGraphicsWindow);
}
