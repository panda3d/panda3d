/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToMaya.h
 * @author drose
 * @date 2005-08-11
 */

#ifndef EGGTOMAYA_H
#define EGGTOMAYA_H

#include "pandatoolbase.h"

#include "eggToSomething.h"
#include "mayaApi.h"

#include "programBase.h"

/**
 * A program to read an egg file and write a maya file.
 */
class EggToMaya : public EggToSomething {
public:
  EggToMaya();

  bool run();

private:
  virtual bool handle_args(ProgramBase::Args &args);

  PT(MayaApi) open_api();

  bool _convert_anim;
  bool _convert_model;
  bool _respect_normals;
  bool _run_server;
};

#endif
