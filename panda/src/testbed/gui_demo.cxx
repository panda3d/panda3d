#include "framework.h"

#include <eventHandler.h>
#include <chancfg.h>
#include <string>
#include <renderModeTransition.h>
#include <colorTransition.h>
#include <colorBlendTransition.h>
#include <cullFaceTransition.h>
#include <depthTestTransition.h>
#include <depthWriteTransition.h>
#include <textureTransition.h>
#include <textureApplyTransition.h>
#include <lightTransition.h>
#include <materialTransition.h>
#include <transformTransition.h>
#include <transparencyTransition.h>
#include <drawBoundsTransition.h>
#include <pruneTransition.h>
#include <get_rel_pos.h>
#include <boundingSphere.h>
#include <geomSphere.h>
#include <geomNode.h>
#include <notify.h>
#include <directionalLight.h>
#include <renderRelation.h>
#include <camera.h>
#include <frustum.h>
#include <orthoProjection.h>
#include <perspectiveProjection.h>
#include <textNode.h>
#include <shaderTransition.h>
#include <colorMatrixTransition.h>
#include <alphaTransformTransition.h>
#include <lensFlareNode.h>
#include <texture.h>
#include <texturePool.h>
#include <spotlight.h>
#include <nodePath.h>
#include <pta_Colorf.h>
#include <pta_float.h>
#include <pt_Node.h>
#include <modelPool.h>

#include <guiManager.h>
#include <guiRollover.h>
#include <guiButton.h>

//From framework
extern PT(GeomNode) geomnode;
extern RenderRelation* first_arc;

static void setup_gui(void) {
  GuiManager* mgr = GuiManager::get_ptr(main_win, mak);
  PT_Node font = ModelPool::load_model("ttf-comic");
  // test 1
  //  mgr->add_region(new GuiRegion("test1", 0., 0.25, 0., 0.25));
  //  mgr->add_label(GuiLabel::make_simple_text_label("test2", font));
  // test 2
  //  GuiLabel* l1 = GuiLabel::make_simple_text_label("off", font);
  //  GuiLabel* l2 = GuiLabel::make_simple_text_label("on", font);
  //  GuiRollover* r1 = new GuiRollover("test2", l1, l2);
  //  r1->manage(mgr, event_handler);
  // test 3
  GuiLabel* l1 = GuiLabel::make_simple_text_label("up", font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("upr", font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("down", font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("downr", font);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("none", font);
  GuiButton* b1 = new GuiButton("test3", l1, l2, l3, l4, l5);
  b1->manage(mgr, event_handler);
}

static void event_2(CPT_Event) {
  static bool is_setup = false;
  if (!is_setup) {
    setup_gui();
    is_setup = true;
  }
}

void demo_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  event_handler.add_hook("2", event_2);
}

int main(int argc, char *argv[]) {
  define_keys = &demo_keys;
  return framework_main(argc, argv);
}
