// Filename: config_ribgsg.cxx
// Created by:  cary (08Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_ribgsg.h"
#include "ribGraphicsStateGuardian.h"

#include <dconfig.h>

Configure(config_ribgsg);
NotifyCategoryDef(ribgsg, ":display:gsg");

ConfigureFn(config_ribgsg) {
  RIBGraphicsStateGuardian::init_type();
  GraphicsStateGuardian::_factory.
    register_factory(RIBGraphicsStateGuardian::get_class_type(),
		     RIBGraphicsStateGuardian::make_RIBGraphicsStateGuardian);
}
