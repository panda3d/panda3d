// Filename: config_stitch.cxx
// Created by:  drose (05Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "config_stitch.h"

#include <dconfig.h>

Configure(config_stitch);

ConfigureFn(config_stitch) {
}

string chan_cfg = config_stitch.GetString("chan-config", "single");

