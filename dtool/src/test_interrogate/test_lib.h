/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_lib.h
 * @author frang
 * @date 2000-06-15
 */

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
