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
#include <guiFrame.h>

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
  //  r1->set_scale(0.1);
  //  r1->set_pos(LVector3f::rfu(0.25, 0., 0.25));
  //  r1->manage(mgr, event_handler);
  // test 3
  //  GuiLabel* l1 = GuiLabel::make_simple_text_label("up", font);
  //  GuiLabel* l2 = GuiLabel::make_simple_text_label("upr", font);
  //  GuiLabel* l3 = GuiLabel::make_simple_text_label("down", font);
  //  GuiLabel* l4 = GuiLabel::make_simple_text_label("downr", font);
  //  GuiLabel* l5 = GuiLabel::make_simple_text_label("none", font);
  //  GuiButton* b1 = new GuiButton("test3", l1, l2, l3, l4, l5);
  //  b1->set_scale(0.1);
  //  b1->set_pos(LVector3f::rfu(-0.25, 0., 0.25));
  //  b1->manage(mgr, event_handler);
  // test 4
  GuiRollover* r1 = new GuiRollover("r1",
				    GuiLabel::make_simple_text_label("1",
								     font),
				    GuiLabel::make_simple_text_label("!",
								     font));
  GuiRollover* r2 = new GuiRollover("r2",
				    GuiLabel::make_simple_text_label("2",
								     font),
				    GuiLabel::make_simple_text_label("@",
								     font));
  GuiRollover* r3 = new GuiRollover("r3",
				    GuiLabel::make_simple_text_label("3",
								     font),
				    GuiLabel::make_simple_text_label("#",
								     font));
  GuiRollover* r4 = new GuiRollover("r4",
				    GuiLabel::make_simple_text_label("4",
								     font),
				    GuiLabel::make_simple_text_label("$",
								     font));
  GuiRollover* r5 = new GuiRollover("r5",
				    GuiLabel::make_simple_text_label("5",
								     font),
				    GuiLabel::make_simple_text_label("%",
								     font));
  GuiRollover* r6 = new GuiRollover("r6",
				    GuiLabel::make_simple_text_label("6",
								     font),
				    GuiLabel::make_simple_text_label("^",
								     font));
  GuiRollover* r7 = new GuiRollover("r7",
				    GuiLabel::make_simple_text_label("7",
								     font),
				    GuiLabel::make_simple_text_label("&",
								     font));
  GuiRollover* r8 = new GuiRollover("r8",
				    GuiLabel::make_simple_text_label("8",
								     font),
				    GuiLabel::make_simple_text_label("*",
								     font));
  GuiRollover* r9 = new GuiRollover("r9",
				    GuiLabel::make_simple_text_label("9",
								     font),
				    GuiLabel::make_simple_text_label("(",
								     font));
  GuiRollover* r0 = new GuiRollover("r0",
				    GuiLabel::make_simple_text_label("0",
								     font),
				    GuiLabel::make_simple_text_label(")",
								     font));
  GuiFrame* f1 = new GuiFrame("test4");
  f1->add_item(r1);
  f1->add_item(r2);
  f1->pack_item(r2, GuiFrame::UNDER, r1);
  f1->pack_item(r2, GuiFrame::RIGHT, r1);
  f1->add_item(r3);
  f1->pack_item(r3, GuiFrame::UNDER, r2);
  f1->pack_item(r3, GuiFrame::RIGHT, r2);
  f1->add_item(r4);
  f1->pack_item(r4, GuiFrame::UNDER, r3);
  f1->pack_item(r4, GuiFrame::RIGHT, r2);
  f1->add_item(r5);
  f1->pack_item(r5, GuiFrame::UNDER, r4);
  f1->pack_item(r5, GuiFrame::RIGHT, r2);
  f1->add_item(r6);
  f1->pack_item(r6, GuiFrame::UNDER, r5);
  f1->pack_item(r6, GuiFrame::LEFT, r5);
  f1->add_item(r7);
  f1->pack_item(r7, GuiFrame::UNDER, r6);
  f1->pack_item(r7, GuiFrame::LEFT, r6);
  f1->add_item(r8);
  f1->pack_item(r8, GuiFrame::ABOVE, r7);
  f1->pack_item(r8, GuiFrame::LEFT, r7);
  f1->add_item(r9);
  f1->pack_item(r9, GuiFrame::ABOVE, r8);
  f1->pack_item(r9, GuiFrame::LEFT, r8);
  f1->add_item(r0);
  f1->pack_item(r0, GuiFrame::ABOVE, r9);
  f1->pack_item(r0, GuiFrame::LEFT, r8);
  f1->set_scale(0.1);
  f1->set_pos(LVector3f::rfu(0., 0., -0.25));
  f1->manage(mgr, event_handler);
  cerr << *f1;
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
