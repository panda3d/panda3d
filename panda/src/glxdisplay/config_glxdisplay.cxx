// Filename: config_glxdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"

#include <dconfig.h>

Configure(config_glxdisplay);
NotifyCategoryDef(glxdisplay, "display");

ConfigureFn(config_glxdisplay) {
  glxGraphicsPipe::init_type();
  GraphicsPipe::_factory.register_factory(glxGraphicsPipe::get_class_type(),
					  glxGraphicsPipe::make_glxGraphicsPipe);
  glxGraphicsWindow::init_type();
  GraphicsWindow::_factory.register_factory(glxGraphicsWindow::get_class_type(),
					    glxGraphicsWindow::make_GlxGraphicsWindow);
}
