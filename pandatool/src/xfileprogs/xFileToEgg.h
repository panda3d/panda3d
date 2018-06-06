/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileToEgg.h
 * @author drose
 * @date 2001-06-21
 */

#ifndef XFILETOEGG_H
#define XFILETOEGG_H

#include "pandatoolbase.h"
#include "somethingToEgg.h"
#include "xFileToEggConverter.h"

#include "dSearchPath.h"

/**
 * A program to read a DirectX "x" file and generate an egg file.
 */
class XFileToEgg : public SomethingToEgg {
public:
  XFileToEgg();

  void run();

public:
  bool _make_char;
  std::string _char_name;
  double _frame_rate;
  bool _keep_model;
  bool _keep_animation;
};

#endif
