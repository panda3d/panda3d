// Filename: testCopy.h
// Created by:  drose (31Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TESTCOPY_H
#define TESTCOPY_H

#include <pandatoolbase.h>

#include "cvsCopy.h"

////////////////////////////////////////////////////////////////////
// 	 Class : TestCopy
// Description : A program to copy ordinary files into the cvs tree.
//               Mainly to test CVSCopy.
////////////////////////////////////////////////////////////////////
class TestCopy : public CVSCopy {
public:
  TestCopy();

  void run();

protected:
  virtual bool copy_file(const Filename &source, const Filename &dest,
			 CVSSourceDirectory *dir, void *extra_data,
			 bool new_file);
};

#endif
