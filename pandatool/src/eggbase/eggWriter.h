// Filename: eggWriter.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGWRITER_H
#define EGGWRITER_H

#include <pandatoolbase.h>

#include "eggBase.h"

#include <filename.h>

#include <fstream.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggWriter
// Description : This is the base class for a program that generates
//               an egg file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class EggWriter : virtual public EggBase {
public:
  EggWriter(bool allow_last_param = false, bool allow_stdout = true);

  ostream &get_output();
  bool has_output_filename() const;
  Filename get_output_filename() const;

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  bool verify_output_file_safe() const;

protected:
  bool _allow_last_param;
  bool _allow_stdout;
  bool _got_output_filename;
  Filename _output_filename;

private:
  ofstream _output_stream;
  ostream *_output_ptr;
};

#endif


