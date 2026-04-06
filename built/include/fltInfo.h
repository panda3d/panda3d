/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltInfo.h
 * @author drose
 * @date 2001-09-05
 */

#ifndef FLTINFO_H
#define FLTINFO_H

#include "pandatoolbase.h"

#include "programBase.h"

class FltRecord;

/**
 * A program to read a flt file and report interesting things about it.
 */
class FltInfo : public ProgramBase {
public:
  FltInfo();

  void run();

protected:
  virtual bool handle_args(Args &args);

  void list_hierarchy(FltRecord *record, int indent_level);

  Filename _input_filename;
  bool _list_hierarchy;
};

#endif
