#include <eventHandler.h>
#include <chancfg.h>
#include <get_rel_pos.h>
#include <loader.h>
#include <pt_NamedNode.h>
#include <framework.h>

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
