// Filename: config_sgidisplay.cxx
// Created by:  cary (07Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgidisplay.h"
#include "sgiGraphicsPipe.h"
#include "sgiHardwareChannel.h"

#include <dconfig.h>

Configure(config_sgidisplay);

ConfigureFn(config_sgidisplay) {
  sgiGraphicsPipe::init_type();
  sgiHardwareChannel::init_type();
}
