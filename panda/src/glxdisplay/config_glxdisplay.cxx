// Filename: config_glxdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"
#include "glxGraphicsWindow.h"
#include "glxDisplay.h"

#include <dconfig.h>

Configure(config_glxdisplay);
NotifyCategoryDef(glxdisplay, "display");

ConfigureFn(config_glxdisplay) {
  glxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(glxGraphicsPipe::get_class_type(),
					  glxGraphicsPipe::make_glxGraphicsPipe);
  glxGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(glxGraphicsWindow::get_class_type(),
					    glxGraphicsWindow::make_GlxGraphicsWindow);
  glxDisplay::init_type();
}

bool gl_show_fps_meter = config_glxdisplay.GetBool("show-fps-meter", false);
float gl_fps_meter_update_interval = max((float)0.5,config_glxdisplay.GetFloat("fps-meter-update-interval", 1.7));
