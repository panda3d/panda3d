// Filename: eggToDXF.h
// Created by:  drose (04May04)
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

#ifndef EGGTODXF_H
#define EGGTODXF_H

#include "pandatoolbase.h"

#include "eggToSomething.h"
#include "eggToDXFLayer.h"

class EggGroupNode;

////////////////////////////////////////////////////////////////////
//       Class : EggToDXF
// Description : A program to read an egg file and write a DXF file.
////////////////////////////////////////////////////////////////////
class EggToDXF : public EggToSomething {
public:
  EggToDXF();

  void run();

  bool _use_polyline;

private:
  void get_layers(EggGroupNode *group);
  void write_tables(ostream &out);
  void write_entities(ostream &out);

  EggToDXFLayers _layers;
};

#endif

