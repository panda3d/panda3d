// Filename: config_gsgbase.cxx
// Created by:  drose (06Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_gsgbase.h"
#include "graphicsStateGuardianBase.h"

#include <dconfig.h>

Configure(config_gsgbase);

ConfigureFn(config_gsgbase) {
  GraphicsStateGuardianBase::init_type();
}
