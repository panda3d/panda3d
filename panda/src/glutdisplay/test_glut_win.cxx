// Filename: test_glut_win.cxx
// Created by:  mike (02Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <iostream>

#ifdef WIN32_VC
#include <windows.h>
#endif
#include <GL/glut.h>

void display_func(void) {
  cerr << "in display func" << endl;
  glutPostRedisplay();
}

int main() {
  glutInitWindowSize(256, 256);
  glutCreateWindow("test glut window");
  glutDisplayFunc(display_func);
  glutMainLoop();
  return 0;
}
