// Filename: test_lib.cxx
// Created by:  frang (15Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "test_lib.h"

int non_member1(float x, float y) {
   return (int)(x + y);
}

my_class1::my_class1(void) {}

my_class1::my_class1(int) {}

my_class1::~my_class1(void) {}

float my_class1::method1(int x) { return (float)(x); }

int my_class1::method2(float x) { return (int)(x); }

int stupid_global;

#include "dconfig.h"

Configure(test_lib);

ConfigureFn(test_lib) {
  cerr << "In test_lib configure function!" << endl;
}

ConfigureLibSym;
