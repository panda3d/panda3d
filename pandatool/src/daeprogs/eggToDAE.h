// Filename: eggToDAE.h
// Created by:  pro-rsoft (04Oct08)
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

#ifndef EGGTODAE_H
#define EGGTODAE_H

#include "pandatoolbase.h"
#include "eggToSomething.h"
#include "eggGroup.h"
#include "eggTransform.h"

#include "pre_fcollada_include.h"
#include "FCollada.h"
#include "FCDocument/FCDSceneNode.h"

////////////////////////////////////////////////////////////////////
//       Class : EggToDAE
// Description : A program to read an egg file and write a DAE file.
////////////////////////////////////////////////////////////////////
class EggToDAE : public EggToSomething {
public:
  EggToDAE();

  void run();

private:
  FCDocument* _document;
  
  void process_node(FCDSceneNode* parent, const PT(EggGroup) node);
  void apply_transform(FCDSceneNode* to, const PT(EggGroup) from);

};

#endif

