// Filename: fltTrans.h
// Created by:  drose (11Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTTRANS_H
#define FLTTRANS_H

#include <pandatoolbase.h>

#include <programBase.h>
#include <withOutputFile.h>

#include <dSearchPath.h>

////////////////////////////////////////////////////////////////////
// 	 Class : FltTrans
// Description : A program to read a flt file and write an equivalent
//               flt file, possibly performing some minor operations
//               along the way.
////////////////////////////////////////////////////////////////////
class FltTrans : public ProgramBase, public WithOutputFile {
public:
  FltTrans();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
  DSearchPath _texture_path;
  bool _got_new_version;
  double _new_version;
};

#endif

