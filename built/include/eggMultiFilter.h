/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggMultiFilter.h
 * @author drose
 * @date 2000-11-02
 */

#ifndef EGGMULTIFILTER_H
#define EGGMULTIFILTER_H

#include "pandatoolbase.h"

#include "eggMultiBase.h"

/**
 * This is a base class for a program that reads in a number of egg files,
 * operates on them, and writes them out again (presumably to a different
 * directory).
 */
class EggMultiFilter : public EggMultiBase {
public:
  EggMultiFilter(bool allow_empty = false);

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  Filename get_output_filename(const Filename &source_filename) const;
  virtual void write_eggs();

protected:
  bool _allow_empty;
  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _inplace;
  Filename _input_filename;
  Filename _filename;
  bool _got_input_filename;

  bool _read_only;
};

#endif
