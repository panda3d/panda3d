/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggFilter.h
 * @author drose
 * @date 2000-02-14
 */

#ifndef EGGFILTER_H
#define EGGFILTER_H

#include "pandatoolbase.h"

#include "eggReader.h"
#include "eggWriter.h"

/**
 * This is the base class for a program that reads an egg file, operates on
 * it, and writes another egg file out.
 */
class EggFilter : public EggReader, public EggWriter {
public:
  EggFilter(bool allow_last_param = false, bool allow_stdout = true);

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();
};

#endif
