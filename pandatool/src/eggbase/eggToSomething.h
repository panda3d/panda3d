/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToSomething.h
 * @author drose
 * @date 2000-02-15
 */

#ifndef EGGTOSOMETHING_H
#define EGGTOSOMETHING_H

#include "pandatoolbase.h"

#include "eggConverter.h"
#include "distanceUnit.h"

/**
 * This is the general base class for a file-converter program that reads some
 * model file format and generates an egg file.
 */
class EggToSomething : public EggConverter {
public:
  EggToSomething(const std::string &format_name,
                 const std::string &preferred_extension = std::string(),
                 bool allow_last_param = true,
                 bool allow_stdout = true);

  void add_units_options();

protected:
  void apply_units_scale(EggData *data);
  virtual void pre_process_egg_file();
  virtual bool handle_args(Args &args);

  DistanceUnit _input_units;
  DistanceUnit _output_units;
};

#endif
