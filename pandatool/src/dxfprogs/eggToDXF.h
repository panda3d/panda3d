/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToDXF.h
 * @author drose
 * @date 2004-05-04
 */

#ifndef EGGTODXF_H
#define EGGTODXF_H

#include "pandatoolbase.h"

#include "eggToSomething.h"
#include "eggToDXFLayer.h"

class EggGroupNode;

/**
 * A program to read an egg file and write a DXF file.
 */
class EggToDXF : public EggToSomething {
public:
  EggToDXF();

  void run();

  bool _use_polyline;

private:
  void get_layers(EggGroupNode *group);
  void write_tables(std::ostream &out);
  void write_entities(std::ostream &out);

  EggToDXFLayers _layers;
};

#endif
