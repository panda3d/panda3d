// Filename: outputFile.h
// Created by:  drose (11Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef WITHOUTPUTFILE_H
#define WITHOUTPUTFILE_H

#include <pandatoolbase.h>

#include "programBase.h"
#include <filename.h>

////////////////////////////////////////////////////////////////////
// 	 Class : WithOutputFile
// Description : This is the bare functionality (intended to be
//               inherited from along with ProgramBase or some
//               derivative) for a program that might generate an
//               output file.
//
//               This provides the has_output_filename() and
//               get_output_filename() methods.
////////////////////////////////////////////////////////////////////
class WithOutputFile {
public:
  WithOutputFile(bool allow_last_param, bool allow_stdout,
		 bool binary_output);
  virtual ~WithOutputFile();

  ostream &get_output();
  bool has_output_filename() const;
  Filename get_output_filename() const;

protected:
  bool check_last_arg(ProgramBase::Args &args, int minimum_args);
  bool verify_output_file_safe() const;

protected:
  bool _allow_last_param;
  bool _allow_stdout;
  bool _binary_output;
  string _preferred_extension;
  bool _got_output_filename;
  Filename _output_filename;

private:
  ofstream _output_stream;
  ostream *_output_ptr;
};

#endif


