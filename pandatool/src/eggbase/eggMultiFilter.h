// Filename: eggMultiFilter.h
// Created by:  drose (02Nov00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGMULTIFILTER_H
#define EGGMULTIFILTER_H

#include <pandatoolbase.h>

#include "eggMultiBase.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggMultiFilter
// Description : This is a base class for a program that reads in a
//               number of egg files, operates on them, and writes
//               them out again (presumably to a different directory).
////////////////////////////////////////////////////////////////////
class EggMultiFilter : public EggMultiBase {
public:
  EggMultiFilter(bool allow_empty = false);

protected:
  virtual bool handle_args(Args &args);
  virtual bool post_command_line();

  Filename get_output_filename(const Filename &source_filename) const;
  void write_eggs();

protected:
  bool _allow_empty;
  bool _got_output_filename;
  Filename _output_filename;
  bool _got_output_dirname;
  Filename _output_dirname;
  bool _inplace;
};

#endif


