/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file save_egg_file.cxx
 * @author drose
 * @date 2002-02-26
 */

#include "save_egg_file.h"
#include "eggSaver.h"
#include "config_egg2pg.h"
#include "modelRoot.h"
#include "sceneGraphReducer.h"
#include "virtualFileSystem.h"
#include "config_putil.h"

/**
 * A convenience function; converts the indicated scene graph to an egg file
 * and writes it to disk.
 */
bool
save_egg_file(const Filename &filename, PandaNode *node, CoordinateSystem cs) {
  PT(EggData) data = new EggData;
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  data->set_coordinate_system(cs);

  EggSaver saver(data);
  if (node->is_of_type(ModelRoot::get_class_type())) {
    // If this is a ModelRoot, only write the nodes below it.  Otherwise we
    // end up inserting a node when we do a bam2egg.
    saver.add_subgraph(node);
  } else {
    saver.add_node(node);
  }

  return data->write_egg(filename);
}

/**
 * Another convenience function; works like save_egg_file() but populates an
 * EggData instead of writing the results to disk.
 */
bool
save_egg_data(EggData *data, PandaNode *node) {
  EggSaver saver(data);
  if (node->is_of_type(ModelRoot::get_class_type())) {
    // If this is a ModelRoot, only write the nodes below it.  Otherwise we
    // end up inserting a node when we do a bam2egg.
    saver.add_subgraph(node);
  } else {
    saver.add_node(node);
  }
  return true;
}
