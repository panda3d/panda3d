// Filename: config_egg_palettize.cxx
// Created by:  drose (02Nov00)
// 
////////////////////////////////////////////////////////////////////

#include <dconfig.h>

#include "sourceEgg.h"

Configure(config_egg_palettize);

ConfigureFn(config_egg_palettize) {
  SourceEgg::init_type();
}
