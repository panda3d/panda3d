// Filename: config_effects.cxx
// Created by:  jason (18Jul00)
// 

#include "config_effects.h"
#include "lensFlareNode.h"

#include <dconfig.h>

Configure(config_effects);
NotifyCategoryDef(effects, "");

ConfigureFn(config_effects) {
  LensFlareNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  LensFlareNode::register_with_read_factory();
}
