// Filename: config_sgiglutdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgiglutdisplay.h"
#include "sgiglutGraphicsPipe.h"

#include <dconfig.h>

Configure(config_sgiglutdisplay);
NotifyCategoryDef(sgiglutdisplay, "display");

ConfigureFn(config_sgiglutdisplay) {
  sgiglutGraphicsPipe::init_type();
  GraphicsPipe::_factory.register_factory(sgiglutGraphicsPipe::get_class_type(),
					  sgiglutGraphicsPipe::make_sgiglutGraphicsPipe);
}
