// Filename: test_glut_win.cxx
// Created by:  mike (02Feb00)
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
