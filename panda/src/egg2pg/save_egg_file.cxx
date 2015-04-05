// Filename: save_egg_file.cxx
// Created by:  drose (26Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "save_egg_file.h"
#include "eggSaver.h"
#include "config_egg2pg.h"
#include "sceneGraphReducer.h"
#include "virtualFileSystem.h"
#include "config_util.h"

////////////////////////////////////////////////////////////////////
//     Function: save_egg_file
//  Description: A convenience function; converts the indicated scene
//               graph to an egg file and writes it to disk.
////////////////////////////////////////////////////////////////////
bool
save_egg_file(const Filename &filename, PandaNode *node, CoordinateSystem cs) {
  PT(EggData) data = new EggData;
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  data->set_coordinate_system(cs);

  EggSaver saver(data);
  saver.add_node(node);

  return data->write_egg(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: save_egg_data
//  Description: Another convenience function; works like
//               save_egg_file() but populates an EggData instead of
//               writing the results to disk.
////////////////////////////////////////////////////////////////////
bool
save_egg_data(EggData *data, PandaNode *node) {
  EggSaver saver(data);
  saver.add_node(node);
  return true;
}
