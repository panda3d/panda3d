// Filename: test_sgraph.cxx
// Created by:  mike (02Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "camera.h"

#include <notify.h>

int main() {
  nout << "running test_sgraph" << endl;
  PT(Camera) cam = new Camera("camera");
  return 0;
}
