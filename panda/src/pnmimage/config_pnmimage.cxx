// Filename: config_pnmimage.cxx
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pnmimage.h"
#include "pnmFileType.h"

#include <dconfig.h>

Configure(config_pnmimage);
NotifyCategoryDef(pnmimage, "");

ConfigureFn(config_pnmimage) {
  PNMFileType::init_type();
}
