// Filename: config_wgldisplay.cxx
// Created by:  mike (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_wgldisplay.h"
#include "wglGraphicsPipe.h"
#include "wglGraphicsWindow.h"

#include <dconfig.h>

Configure(config_wgldisplay);
NotifyCategoryDef(wgldisplay, "display");

ConfigureFn(config_wgldisplay) {
  wglGraphicsPipe::init_type();
  GraphicsPipe::_factory.register_factory(
			wglGraphicsPipe::get_class_type(),
		  	wglGraphicsPipe::make_wglGraphicsPipe);
  wglGraphicsWindow::init_type();
  GraphicsWindow::_factory.register_factory(
			wglGraphicsWindow::get_class_type(),
		    	wglGraphicsWindow::make_wglGraphicsWindow);
}
