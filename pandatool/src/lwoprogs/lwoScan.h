// Filename: lwoScan.h
// Created by:  drose (30Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef LWOSCAN_H
#define LWOSCAN_H

#include <programBase.h>
#include <filename.h>

////////////////////////////////////////////////////////////////////
//       Class : LwoScan
// Description : A program to read a Lightwave file and report its
//               structure and contents.
////////////////////////////////////////////////////////////////////
class LwoScan : public ProgramBase {
public:
  LwoScan();

  void run();

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif

