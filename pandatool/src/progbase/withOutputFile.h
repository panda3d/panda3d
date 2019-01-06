/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file withOutputFile.h
 * @author drose
 * @date 2001-04-11
 */

#ifndef WITHOUTPUTFILE_H
#define WITHOUTPUTFILE_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "filename.h"

/**
 * This is the bare functionality (intended to be inherited from along with
 * ProgramBase or some derivative) for a program that might generate an output
 * file.
 *
 * This provides the has_output_filename() and get_output_filename() methods.
 */
class WithOutputFile {
public:
  WithOutputFile(bool allow_last_param, bool allow_stdout,
                 bool binary_output);
  virtual ~WithOutputFile();

  std::ostream &get_output();
  void close_output();
  bool has_output_filename() const;
  Filename get_output_filename() const;

protected:
  INLINE void set_binary_output(bool binary_output);

  bool check_last_arg(ProgramBase::Args &args, int minimum_args);
  bool verify_output_file_safe() const;

protected:
  bool _allow_last_param;
  bool _allow_stdout;
  bool _binary_output;
  std::string _preferred_extension;
  bool _got_output_filename;
  Filename _output_filename;

private:
  std::ofstream _output_stream;
  std::ostream *_output_ptr;
  bool _owns_output_ptr;
};

#include "withOutputFile.I"

#endif
