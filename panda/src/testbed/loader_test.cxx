// Filename: loader_test.cxx
// Created by:  
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

#include "eventHandler.h"
#include "chancfg.h"
#include "get_rel_pos.h"
#include "loader.h"
#include "pt_NamedNode.h"
#include "framework.h"

uint model_id;
const int MAX_LOOPS = 100;

void event_p(CPT_Event) {
  //model_id = loader.request_load("jafar-statue.egg");
  //model_id = loader.request_load("camera.egg");
  //model_id = loader.request_load("box.egg");
  //model_id = loader.request_load("yup-axis.egg");
  //model_id = loader.request_load("hand.egg");

  //model_id = loader.request_load("frowney.egg", "");
  //model_id = loader.request_load("trolley.bam", "");
  //model_id = loader.request_load("smiley.egg", "");
  //model_id = loader.request_load("jack.bam", "");
  model_id = loader.request_load("herc-6000.egg", "");
}

void event_c(CPT_Event) {
  if (loader.check_load(model_id))
    cerr << "load is complete" << endl;
  else
    cerr << "loading not finished yet" << endl;
}

void event_s(CPT_Event) {
  PT_Node model = loader.fetch_load(model_id);
  if (model != (NamedNode*)0L)
    new RenderRelation(render, model);
  else
    cerr << "null model!" << endl;
}

void loader_keys(EventHandler& eh) {
  eh.add_hook("p", event_p);
  eh.add_hook("c", event_c);
  eh.add_hook("s", event_s);
}

int main(int argc, char *argv[]) {
  define_keys = &loader_keys;
//  loader.fork_asynchronous_thread();
  return framework_main(argc, argv);
}
