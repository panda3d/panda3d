// Filename: config_sgiglxdisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgiglxdisplay.h"
#include "sgiglxGraphicsPipe.h"

#include <dconfig.h>

Configure(config_sgiglxdisplay);
NotifyCategoryDef(sgiglxdisplay, "display");

ConfigureFn(config_sgiglxdisplay) {
  SgiGlxGraphicsPipe::init_type();
  GraphicsPipe::get_factory().register_factory(SgiGlxGraphicsPipe::get_class_type(),
                                          SgiGlxGraphicsPipe::make_sgiglxgraphicspipe);
}
