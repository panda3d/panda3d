// Filename: config_wdxdisplay.cxx
// Created by:  mike (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_wdxdisplay.h"
#include "wdxGraphicsPipe.h"
#include "wdxGraphicsWindow.h"

#include <dconfig.h>

Configure(config_wdxdisplay);
NotifyCategoryDef(wdxdisplay, "display");

ConfigureFn(config_wdxdisplay) {
  wdxGraphicsPipe::init_type();
  GraphicsPipe::_factory.register_factory(
			wdxGraphicsPipe::get_class_type(),
		  	wdxGraphicsPipe::make_wdxGraphicsPipe);
  wdxGraphicsWindow::init_type();
  GraphicsWindow::_factory.register_factory(
			wdxGraphicsWindow::get_class_type(),
		    	wdxGraphicsWindow::make_wdxGraphicsWindow);
}
