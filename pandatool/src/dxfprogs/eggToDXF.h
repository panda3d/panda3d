// Filename: eggToDXF.h
// Created by:  drose (04May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

