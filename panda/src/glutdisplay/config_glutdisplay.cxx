// Filename: config_glutdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_glutdisplay.h"
#include "glutGraphicsPipe.h"
#include "glutGraphicsWindow.h"

#include <dconfig.h>

Configure(config_glutdisplay);
NotifyCategoryDef(glutdisplay, "display");

ConfigureFn(config_glutdisplay) {
  glutGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(glutGraphicsPipe::get_class_type(),
                                          glutGraphicsPipe::make_glutGraphicsPipe);
  glutGraphicsWindow::init_type();
  GraphicsWindow::get_factory().register_factory(glutGraphicsWindow::get_class_type(),
                                            glutGraphicsWindow::make_GlutGraphicsWindow);
}
