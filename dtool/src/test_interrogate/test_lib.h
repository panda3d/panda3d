// Filename: test_lib.h
// Created by:  frang (15Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef __TEST_LIB_H__
#define __TEST_LIB_H__

int non_member1(float x, float);

class my_class1 {
public:
  my_class1(void);
  my_class1(int);

  float method1(int);

  int _member1;
private:
  ~my_class1(void);

  int method2(float);

  int _member2;
};

#endif /* __TEST_LIB_H__ */
