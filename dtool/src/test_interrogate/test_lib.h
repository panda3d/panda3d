// Filename: test_lib.h
// Created by:  frang (15Jun00)
//
///////////////////////////////////////////////////////////////////////

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
