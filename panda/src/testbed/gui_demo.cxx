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

// static PT(GuiFrame) global_frame;

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
  //  l2->set_foreground_color(1., 1., 0., 1.);
  //  l4->set_foreground_color(1., 1., 0., 1.);
  //  l3->set_background_color(1., 1., 1., 0.5);
  //  l4->set_background_color(1., 1., 1., 0.5);
  //  b1->manage(mgr, event_handler);
  // test 4
  //  GuiRollover* r1 = new GuiRollover("r1",
  //				    GuiLabel::make_simple_text_label("1",
  //								     font),
  //				    GuiLabel::make_simple_text_label("!",
  //								     font));
  //  GuiRollover* r2 = new GuiRollover("r2",
  //				    GuiLabel::make_simple_text_label("2",
  //								     font),
  //				    GuiLabel::make_simple_text_label("@",
  //								     font));
  //  GuiRollover* r3 = new GuiRollover("r3",
  //				    GuiLabel::make_simple_text_label("3",
  //								     font),
  //				    GuiLabel::make_simple_text_label("#",
  //								     font));
  //  GuiRollover* r4 = new GuiRollover("r4",
  //				    GuiLabel::make_simple_text_label("4",
  //								     font),
  //				    GuiLabel::make_simple_text_label("$",
  //								     font));
  //  GuiRollover* r5 = new GuiRollover("r5",
  //				    GuiLabel::make_simple_text_label("5",
  //								     font),
  //				    GuiLabel::make_simple_text_label("%",
  //								     font));
  //  GuiRollover* r6 = new GuiRollover("r6",
  //				    GuiLabel::make_simple_text_label("6",
  //								     font),
  //				    GuiLabel::make_simple_text_label("^",
  //								     font));
  //  GuiRollover* r7 = new GuiRollover("r7",
  //				    GuiLabel::make_simple_text_label("7",
  //								     font),
  //				    GuiLabel::make_simple_text_label("&",
  //								     font));
  //  GuiRollover* r8 = new GuiRollover("r8",
  //				    GuiLabel::make_simple_text_label("8",
  //								     font),
  //				    GuiLabel::make_simple_text_label("*",
  //								     font));
  //  GuiRollover* r9 = new GuiRollover("r9",
  //				    GuiLabel::make_simple_text_label("9",
  //								     font),
  //				    GuiLabel::make_simple_text_label("(",
  //								     font));
  //  GuiRollover* r0 = new GuiRollover("r0",
  //				    GuiLabel::make_simple_text_label("0",
  //								     font),
  //				    GuiLabel::make_simple_text_label(")",
  //								     font));
  //  GuiFrame* f1 = new GuiFrame("test4");
  //  f1->add_item(r1);
  //  f1->add_item(r2);
  //  f1->pack_item(r2, GuiFrame::UNDER, r1);
  //  f1->pack_item(r2, GuiFrame::RIGHT, r1);
  //  f1->add_item(r3);
  //  f1->pack_item(r3, GuiFrame::UNDER, r2);
  //  f1->pack_item(r3, GuiFrame::RIGHT, r2);
  //  f1->add_item(r4);
  //  f1->pack_item(r4, GuiFrame::UNDER, r3);
  //  f1->pack_item(r4, GuiFrame::RIGHT, r2);
  //  f1->add_item(r5);
  //  f1->pack_item(r5, GuiFrame::UNDER, r4);
  //  f1->pack_item(r5, GuiFrame::LEFT, r4);
  //  f1->add_item(r6);
  //  f1->pack_item(r6, GuiFrame::UNDER, r5);
  //  f1->pack_item(r6, GuiFrame::LEFT, r5);
  //  f1->add_item(r7);
  //  f1->pack_item(r7, GuiFrame::ABOVE, r6);
  //  f1->pack_item(r7, GuiFrame::LEFT, r6);
  //  f1->add_item(r8);
  //  f1->pack_item(r8, GuiFrame::ABOVE, r7);
  //  f1->pack_item(r8, GuiFrame::LEFT, r7);
  //  f1->add_item(r9);
  //  f1->pack_item(r9, GuiFrame::ABOVE, r8);
  //  f1->pack_item(r9, GuiFrame::LEFT, r7);
  //  f1->add_item(r0);
  //  f1->pack_item(r0, GuiFrame::ABOVE, r9);
  //  f1->pack_item(r0, GuiFrame::RIGHT, r9);
  //  f1->set_scale(0.1);
  //  f1->set_pos(LVector3f::rfu(0., 0., -0.25));
  //  f1->manage(mgr, event_handler);
  //  cerr << *f1;
  // test 5
  //  GuiLabel* l1 = GuiLabel::make_simple_text_label("on", font);
  //  GuiLabel* l2 = GuiLabel::make_simple_text_label("off", font);
  //  GuiLabel* l3 = GuiLabel::make_simple_text_label("over", font);
  //  GuiLabel* l4 = GuiLabel::make_simple_text_label("easy", font);
  //  l1->set_background_color(1., 1., 1., 0.3);
  //  l2->set_background_color(1., 1., 1., 0.3);
  //  l3->set_background_color(1., 1., 1., 0.3);
  //  l4->set_background_color(1., 1., 1., 0.3);
  //  GuiRollover* r1 = new GuiRollover("r1", l1, l2);
  //  GuiRollover* r2 = new GuiRollover("r2", l3, l4);
  //  GuiFrame* f1 = new GuiFrame("test5");
  //  f1->add_item(r1);
  //  f1->add_item(r2);
  //  f1->pack_item(r2, GuiFrame::UNDER, r1);
  //  f1->set_scale(0.1);
  //  f1->manage(mgr, event_handler);
  //  float w1, w2, w3, w4, w;
  //  w1 = l1->get_width();
  //  w2 = l2->get_width();
  //  w3 = l3->get_width();
  //  w4 = l4->get_width();
  //  w = (w1>w2)?w1:w2;
  //  w = (w>w3)?w:w3;
  //  w = (w>w4)?w:w4;
  //  l1->set_width(w);
  //  l2->set_width(w);
  //  l3->set_width(w);
  //  l4->set_width(w);
  //  global_frame = f1;
  // test 6  (the greg test)
  GuiFrame* f1 = new GuiFrame("canids");
  GuiLabel* b1l1 = GuiLabel::make_simple_text_label("dingo", font);
  b1l1->set_foreground_color(0., 0., 0., 1.);
  b1l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b1l2 = GuiLabel::make_simple_text_label("dingo", font);
  b1l2->set_foreground_color(0., 0., 0., 1.);
  b1l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b1l3 = GuiLabel::make_simple_text_label("dingo", font);
  b1l3->set_foreground_color(1., 1., 1., 1.);
  b1l3->set_background_color(0., 0., 0., 1.);
  GuiButton* b1 = new GuiButton("dingo", b1l1, b1l2, b1l3, b1l3, b1l1);
  b1->set_scale(0.1);
  f1->add_item(b1);
  GuiLabel* b2l1 = GuiLabel::make_simple_text_label("jackel", font);
  b2l1->set_foreground_color(0., 0., 0., 1.);
  b2l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b2l2 = GuiLabel::make_simple_text_label("jackel", font);
  b2l2->set_foreground_color(0., 0., 0., 1.);
  b2l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b2l3 = GuiLabel::make_simple_text_label("jackel", font);
  b2l3->set_foreground_color(1., 1., 1., 1.);
  b2l3->set_background_color(0., 0., 0., 1.);
  GuiButton* b2 = new GuiButton("jackel", b2l1, b2l2, b2l3, b2l3, b2l1);
  b2->set_scale(0.1);
  f1->add_item(b2);
  GuiLabel* b3l1 = GuiLabel::make_simple_text_label("hyena", font);
  b3l1->set_foreground_color(0., 0., 0., 1.);
  b3l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b3l2 = GuiLabel::make_simple_text_label("hyena", font);
  b3l2->set_foreground_color(0., 0., 0., 1.);
  b3l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b3l3 = GuiLabel::make_simple_text_label("hyena", font);
  b3l3->set_foreground_color(1., 1., 1., 1.);
  b3l3->set_background_color(0., 0., 0., 1.);
  GuiButton* b3 = new GuiButton("hyena", b3l1, b3l2, b3l3, b3l3, b3l1);
  b3->set_scale(0.1);
  f1->add_item(b3);
  GuiLabel* b4l1 = GuiLabel::make_simple_text_label("wolf", font);
  b4l1->set_foreground_color(0., 0., 0., 1.);
  b4l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b4l2 = GuiLabel::make_simple_text_label("wolf", font);
  b4l2->set_foreground_color(0., 0., 0., 1.);
  b4l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b4l3 = GuiLabel::make_simple_text_label("wolf", font);
  b4l3->set_foreground_color(1., 1., 1., 1.);
  b4l3->set_background_color(0., 0., 0., 1.);
  GuiButton* b4 = new GuiButton("wolf", b4l1, b4l2, b4l3, b4l3, b4l1);
  b4->set_scale(0.1);
  f1->add_item(b4);
  GuiLabel* b5l1 = GuiLabel::make_simple_text_label("fox", font);
  b5l1->set_foreground_color(0., 0., 0., 1.);
  b5l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b5l2 = GuiLabel::make_simple_text_label("fox", font);
  b5l2->set_foreground_color(0., 0., 0., 1.);
  b5l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b5l3 = GuiLabel::make_simple_text_label("fox", font);
  b5l3->set_foreground_color(1., 1., 1., 1.);
  b5l3->set_background_color(0., 0., 0., 1.);
  GuiButton* b5 = new GuiButton("fox", b5l1, b5l2, b5l3, b5l3, b5l1);
  b5->set_scale(0.1);
  f1->add_item(b5);
  f1->pack_item(b2, GuiFrame::UNDER, b1);
  f1->pack_item(b3, GuiFrame::UNDER, b2);
  f1->pack_item(b4, GuiFrame::UNDER, b3);
  f1->pack_item(b5, GuiFrame::UNDER, b4);
  float w, w1, w2;
  w1 = b1l1->get_width();
  w2 = b2l1->get_width();
  w = (w1>w2)?w1:w2;
  w2 = b3l1->get_width();
  w = (w>w2)?w:w2;
  w2 = b4l1->get_width();
  w = (w>w2)?w:w2;
  w2 = b5l1->get_width();
  w = (w>w2)?w:w2;
  b1l1->set_width(w);
  b1l2->set_width(w);
  b1l3->set_width(w);
  b2l1->set_width(w);
  b2l2->set_width(w);
  b2l3->set_width(w);
  b3l1->set_width(w);
  b3l2->set_width(w);
  b3l3->set_width(w);
  b4l1->set_width(w);
  b4l2->set_width(w);
  b4l3->set_width(w);
  b5l1->set_width(w);
  b5l2->set_width(w);
  b5l3->set_width(w);
  f1->manage(mgr, event_handler);
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
