// Filename: deadrec_rec.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"

static void deadrec_setup(void) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup();
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
