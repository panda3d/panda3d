/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmBba.h
 * @author drose
 * @date 2011-03-02
 */

#ifndef PFMBBA_H
#define PFMBBA_H

#include "pandatoolbase.h"
#include "programBase.h"
#include "filename.h"
#include "pvector.h"
#include "nodePath.h"
#include "luse.h"

class PfmFile;

/**
 * Generates a bounding-box description of a pfm file.
 */
class PfmBba : public ProgramBase {
public:
  PfmBba();

  void run();
  bool process_pfm(const Filename &input_filename, PfmFile &file);

protected:
  virtual bool handle_args(Args &args);

private:
  typedef pvector<Filename> Filenames;
  Filenames _input_filenames;

  bool _got_zero_special;
  bool _got_output_filename;
  Filename _output_filename;
};

#endif
