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
#include <guiSign.h>
#include <guiListBox.h>
#include <guiChooser.h>

//From framework
extern PT(GeomNode) geomnode;
extern RenderRelation* first_arc;

// static PT(GuiFrame) global_frame;

static void test1(GuiManager* mgr, Node* font) {
  //  mgr->add_region(new GuiRegion("test1", 0., 0.25, 0., 0.25));
  //  mgr->add_label(GuiLabel::make_simple_text_label("test2", font));
}

static void test2(GuiManager* mgr, Node* font) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label("off", font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("on", font);
  GuiRollover* r1 = new GuiRollover("test2", l1, l2);
  r1->set_scale(0.1);
  r1->set_pos(LVector3f::rfu(0.25, 0., 0.25));
  r1->thaw();
  r1->manage(mgr, event_handler);
}

static void test3(GuiManager* mgr, Node* font) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label("up", font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("upr", font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("down", font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("downr", font);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("none", font);
  GuiButton* b1 = new GuiButton("test3", l1, l2, l3, l4, l5);
  b1->set_scale(0.1);
  b1->set_pos(LVector3f::rfu(-0.25, 0., 0.25));
  l2->set_foreground_color(1., 1., 0., 1.);
  l4->set_foreground_color(1., 1., 0., 1.);
  l3->set_background_color(1., 1., 1., 0.5);
  l4->set_background_color(1., 1., 1., 0.5);
  b1->thaw();
  b1->manage(mgr, event_handler);
  GuiLabel* ll1 = GuiLabel::make_simple_text_label("lup", font);
  GuiLabel* ll2 = GuiLabel::make_simple_text_label("lupr", font);
  GuiLabel* ll3 = GuiLabel::make_simple_text_label("ldown", font);
  GuiLabel* ll4 = GuiLabel::make_simple_text_label("ldownr", font);
  GuiLabel* ll5 = GuiLabel::make_simple_text_label("lnone", font);
  GuiButton* b2 = new GuiButton("test3_2", ll1, ll2, ll3, ll4, ll5);
  b2->set_scale(0.1);
  b2->set_pos(LVector3f::rfu(0.25, 0., 0.25));
  ll2->set_foreground_color(1., 1., 0., 1.);
  ll4->set_foreground_color(1., 1., 0., 1.);
  ll3->set_background_color(1., 1., 1., 0.5);
  ll4->set_background_color(1., 1., 1., 0.5);
  b2->thaw();
  b2->manage(mgr, event_handler);
}

static void test4(GuiManager* mgr, Node* font) {
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
  f1->pack_item(r5, GuiFrame::LEFT, r4);
  f1->add_item(r6);
  f1->pack_item(r6, GuiFrame::UNDER, r5);
  f1->pack_item(r6, GuiFrame::LEFT, r5);
  f1->add_item(r7);
  f1->pack_item(r7, GuiFrame::ABOVE, r6);
  f1->pack_item(r7, GuiFrame::LEFT, r6);
  f1->add_item(r8);
  f1->pack_item(r8, GuiFrame::ABOVE, r7);
  f1->pack_item(r8, GuiFrame::LEFT, r7);
  f1->add_item(r9);
  f1->pack_item(r9, GuiFrame::ABOVE, r8);
  f1->pack_item(r9, GuiFrame::LEFT, r7);
  f1->add_item(r0);
  f1->pack_item(r0, GuiFrame::ABOVE, r9);
  f1->pack_item(r0, GuiFrame::RIGHT, r9);
  f1->set_scale(0.1);
  f1->set_pos(LVector3f::rfu(0., 0., -0.25));
  f1->thaw();
  f1->manage(mgr, event_handler);
  //  cerr << *f1;
}

// PT(GuiFrame) global_frame;

static void test5(GuiManager* mgr, Node* font) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label("on", font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("off", font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("over", font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("easy", font);
  l1->set_background_color(1., 1., 1., 0.3);
  l2->set_background_color(1., 1., 1., 0.3);
  l3->set_background_color(1., 1., 1., 0.3);
  l4->set_background_color(1., 1., 1., 0.3);
  GuiRollover* r1 = new GuiRollover("r1", l1, l2);
  GuiRollover* r2 = new GuiRollover("r2", l3, l4);
  GuiFrame* f1 = new GuiFrame("test5");
  f1->add_item(r1);
  f1->add_item(r2);
  f1->pack_item(r2, GuiFrame::UNDER, r1);
  f1->set_scale(0.1);
  float w1, w2, w3, w4, w;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w3 = l3->get_width();
  w4 = l4->get_width();
  w = (w1>w2)?w1:w2;
  w = (w>w3)?w:w3;
  w = (w>w4)?w:w4;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  f1->thaw();
  f1->manage(mgr, event_handler);
  //  global_frame = f1;
}

static void test6(GuiManager* mgr, Node* font) {
  // the greg test
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
  f1->pack_item(b2, GuiFrame::UNDER, b1, 0.05);
  f1->pack_item(b2, GuiFrame::ALIGN_LEFT, b1);
  f1->pack_item(b3, GuiFrame::UNDER, b2, 0.05);
  f1->pack_item(b3, GuiFrame::ALIGN_LEFT, b2);
  f1->pack_item(b4, GuiFrame::UNDER, b3, 0.05);
  f1->pack_item(b4, GuiFrame::ALIGN_LEFT, b3);
  f1->pack_item(b5, GuiFrame::UNDER, b4, 0.05);
  f1->pack_item(b5, GuiFrame::ALIGN_LEFT, b4);
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
  f1->recompute();
  f1->thaw();
  f1->manage(mgr, event_handler);
}

/*
GuiManager* g_mgr;
PT(GuiButton) b1;
PT(GuiButton) b4;
PT(GuiButton) b5;
PT(GuiFrame) f1;
static bool frame_state = true;

static void test7(GuiManager* mgr, Node* font) {
  f1 = new GuiFrame("canids");
  GuiLabel* b1l1 = GuiLabel::make_simple_text_label("dingo", font);
  b1l1->set_foreground_color(0., 0., 0., 1.);
  b1l1->set_background_color(1., 1., 1., 1.);
  GuiLabel* b1l2 = GuiLabel::make_simple_text_label("dingo", font);
  b1l2->set_foreground_color(0., 0., 0., 1.);
  b1l2->set_background_color(1., 1., 0., 1.);
  GuiLabel* b1l3 = GuiLabel::make_simple_text_label("dingo", font);
  b1l3->set_foreground_color(1., 1., 1., 1.);
  b1l3->set_background_color(0., 0., 0., 1.);
  b1 = new GuiButton("dingo", b1l1, b1l2, b1l3, b1l3, b1l1);
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
  b4 = new GuiButton("wolf", b4l1, b4l2, b4l3, b4l3, b4l1);
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
  b5 = new GuiButton("fox", b5l1, b5l2, b5l3, b5l3, b5l1);
  b5->set_scale(0.1);
  f1->pack_item(b2, GuiFrame::UNDER, b1);
  f1->pack_item(b2, GuiFrame::LEFT, b1);
  f1->pack_item(b3, GuiFrame::UNDER, b1);
  f1->pack_item(b3, GuiFrame::RIGHT, b1);
  f1->pack_item(b4, GuiFrame::UNDER, b1);
  f1->pack_item(b4, GuiFrame::ALIGN_LEFT, b1);
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
  f1->recompute();
  f1->thaw();
  b5->thaw();
  f1->manage(mgr, event_handler);
}
*/

/*
GuiManager* g_mgr;
PT(GuiSign) s1;
PT(GuiSign) s4;
PT(GuiSign) s5;
PT(GuiFrame) f1;
static bool frame_state = true;

static void test8(GuiManager* mgr, Node* font) {
  f1 = new GuiFrame("canids");
  GuiLabel* l1 = GuiLabel::make_simple_text_label("dingo", font);
  l1->set_foreground_color(0., 0., 0., 1.);
  l1->set_background_color(1., 1., 1., 1.);
  s1 = new GuiSign("dingo", l1);
  s1->set_scale(0.1);
  f1->add_item(s1);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("jackel", font);
  l2->set_foreground_color(0., 0., 0., 1.);
  l2->set_background_color(1., 1., 1., 1.);
  GuiSign* s2 = new GuiSign("jackel", l2);
  s2->set_scale(0.1);
  f1->add_item(s2);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("hyena", font);
  l3->set_foreground_color(0., 0., 0., 1.);
  l3->set_background_color(1., 1., 1., 1.);
  GuiSign* s3 = new GuiSign("jackel", l3);
  s3->set_scale(0.1);
  f1->add_item(s3);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("wolf", font);
  l4->set_foreground_color(0., 0., 0., 1.);
  l4->set_background_color(1., 1., 1., 1.);
  s4 = new GuiSign("wolf", l4);
  s4->set_scale(0.1);
  f1->add_item(s4);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("fox", font);
  l5->set_foreground_color(0., 0., 0., 1.);
  l5->set_background_color(1., 1., 1., 1.);
  s5 = new GuiSign("fox", l5);
  s5->set_scale(0.1);
  f1->pack_item(s2, GuiFrame::UNDER, s1);
  f1->pack_item(s2, GuiFrame::LEFT, s1);
  f1->pack_item(s3, GuiFrame::UNDER, s1);
  f1->pack_item(s3, GuiFrame::RIGHT, s1);
  f1->pack_item(s4, GuiFrame::UNDER, s1);
  f1->pack_item(s4, GuiFrame::ALIGN_LEFT, s1);
  float w, w1, w2;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w = (w1>w2)?w1:w2;
  w2 = l3->get_width();
  w = (w>w2)?w:w2;
  w2 = l4->get_width();
  w = (w>w2)?w:w2;
  w2 = l5->get_width();
  w = (w>w2)?w:w2;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  l5->set_width(w);
  f1->recompute();
  f1->thaw();
  s5->thaw();
  f1->manage(mgr, event_handler);
}
*/

/*
GuiManager* g_mgr;
PT(GuiLabel) l4;
PT(GuiFrame) f1;
static bool frame_state = true;

static void test9(GuiManager* mgr, Node* font) {
  f1 = new GuiFrame("canids");
  GuiLabel* l1 = GuiLabel::make_simple_text_label("dingo", font);
  l1->set_foreground_color(0., 0., 0., 1.);
  l1->set_background_color(1., 1., 1., 1.);
  GuiSign* s1 = new GuiSign("dingo", l1);
  s1->set_scale(0.1);
  f1->add_item(s1);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("jackel", font);
  l2->set_foreground_color(0., 0., 0., 1.);
  l2->set_background_color(1., 1., 1., 1.);
  GuiSign* s2 = new GuiSign("jackel", l2);
  s2->set_scale(0.1);
  f1->add_item(s2);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("hyena", font);
  l3->set_foreground_color(0., 0., 0., 1.);
  l3->set_background_color(1., 1., 1., 1.);
  GuiSign* s3 = new GuiSign("jackel", l3);
  s3->set_scale(0.1);
  f1->add_item(s3);
  l4 = GuiLabel::make_simple_text_label("wolf", font);
  l4->set_foreground_color(0., 0., 0., 1.);
  l4->set_background_color(1., 1., 1., 1.);
  GuiSign* s4 = new GuiSign("wolf", l4);
  s4->set_scale(0.1);
  f1->add_item(s4);
  f1->pack_item(s2, GuiFrame::UNDER, s1);
  f1->pack_item(s2, GuiFrame::LEFT, s1);
  f1->pack_item(s3, GuiFrame::UNDER, s1);
  f1->pack_item(s3, GuiFrame::RIGHT, s1);
  f1->pack_item(s4, GuiFrame::UNDER, s1);
  f1->pack_item(s4, GuiFrame::ALIGN_LEFT, s1);
  float w, w1, w2;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w = (w1>w2)?w1:w2;
  w2 = l3->get_width();
  w = (w>w2)?w:w2;
  w2 = l4->get_width();
  w = (w>w2)?w:w2;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  f1->recompute();
  f1->thaw();
  s4->thaw();
  f1->manage(mgr, event_handler);
}
*/

/*
PT(GuiSign) s1;
PT(GuiSign) s2;
PT(GuiSign) s3;
static bool prior_state = true;

static void test10(GuiManager* mgr, Node* font) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label("A", font);
  l1->set_foreground_color(0., 0., 0., 1.);
  l1->set_background_color(1., 0., 0., 1.);
  s1 = new GuiSign("A", l1);
  s1->set_scale(0.1);
  s1->set_priority(GuiItem::P_Low);
  s1->thaw();
  s1->manage(mgr, event_handler);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("B", font);
  l2->set_foreground_color(0., 0., 0., 1.);
  l2->set_background_color(0., 1., 0., 1.);
  s2 = new GuiSign("B", l2);
  s2->set_scale(0.1);
  s2->set_pos(LVector3f::rfu(0.05, 0., 0.05));
  s2->set_priority(GuiItem::P_Normal);
  s2->thaw();
  s2->manage(mgr, event_handler);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("C", font);
  l3->set_foreground_color(0., 0., 0., 1.);
  l3->set_background_color(0., 0., 1., 1.);
  s3 = new GuiSign("C", l3);
  s3->set_scale(0.1);
  s3->set_pos(LVector3f::rfu(0.1, 0., 0.1));
  s3->set_priority(GuiItem::P_High);
  s3->thaw();
  s3->manage(mgr, event_handler);
}
*/

PT(GuiListBox) lb1;

static void test11(GuiManager* mgr, Node* font) {
  GuiLabel* ul = GuiLabel::make_simple_text_label("upup", font);
  GuiSign* us = new GuiSign("up_arrow", ul);
  us->set_scale(0.1);
  GuiLabel* dl = GuiLabel::make_simple_text_label("dndn", font);
  GuiSign* ds = new GuiSign("down_arrow", dl);
  ds->set_scale(0.1);
  lb1 = new GuiListBox("list_box", 4, us, ds);
  GuiLabel* l1 = GuiLabel::make_simple_text_label("hyena", font);
  GuiSign* s1 = new GuiSign("hyena", l1);
  s1->set_scale(0.1);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("dingo", font);
  GuiSign* s2 = new GuiSign("dingo", l2);
  s2->set_scale(0.1);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("jackal", font);
  GuiSign* s3 = new GuiSign("jackal", l3);
  s3->set_scale(0.1);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("wolf", font);
  GuiSign* s4 = new GuiSign("wolf", l4);
  s4->set_scale(0.1);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("fox", font);
  GuiSign* s5 = new GuiSign("fox", l5);
  s5->set_scale(0.1);
  float w, w1, w2;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w = (w1>w2)?w1:w2;
  w2 = l3->get_width();
  w = (w>w2)?w:w2;
  w2 = l4->get_width();
  w = (w>w2)?w:w2;
  w2 = l5->get_width();
  w = (w>w2)?w:w2;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  l5->set_width(w);
  ul->set_background_color(0., 0., 0., 1.);
  dl->set_background_color(0., 0., 0., 1.);
  l1->set_background_color(0., 0., 0., 1.);
  l2->set_background_color(0., 0., 0., 1.);
  l3->set_background_color(0., 0., 0., 1.);
  l4->set_background_color(0., 0., 0., 1.);
  l5->set_background_color(0., 0., 0., 1.);
  lb1->add_item(s1);
  lb1->add_item(s2);
  lb1->add_item(s3);
  lb1->add_item(s4);
  lb1->add_item(s5);
  lb1->thaw();
  lb1->manage(mgr, event_handler);
  cout << *lb1;
}

static void test12(GuiManager* mgr, Node* font) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label("up", font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("upr", font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("down", font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("downr", font);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("none", font);
  GuiButton* b1 = new GuiButton("test12", l1, l2, l3, l4, l5);
  b1->set_scale(0.1);
  b1->set_pos(LVector3f::rfu(-0.25, 0., 0.25));
  l2->set_foreground_color(1., 1., 0., 1.);
  l4->set_foreground_color(1., 1., 0., 1.);
  l1->set_background_color(0., 0., 0., 1.);
  l2->set_background_color(0., 0., 0., 1.);
  l3->set_background_color(1., 1., 1., 0.5);
  l4->set_background_color(1., 1., 1., 0.5);
  b1->thaw();
  b1->manage(mgr, event_handler);
  b1->set_behavior_event("demo-event-thing");
  b1->start_behavior();
}

static void test13(GuiManager* mgr, Node* font) {
  GuiLabel* ul1 = GuiLabel::make_simple_text_label("upup", font);
  GuiLabel* ul2 = GuiLabel::make_simple_text_label("upup", font);
  GuiLabel* ul3 = GuiLabel::make_simple_text_label("upup", font);
  GuiButton* ub = new GuiButton("up_arrow", ul1, ul2, ul3);
  ub->set_scale(0.1);
  GuiLabel* dl1 = GuiLabel::make_simple_text_label("dndn", font);
  GuiLabel* dl2 = GuiLabel::make_simple_text_label("dndn", font);
  GuiLabel* dl3 = GuiLabel::make_simple_text_label("dndn", font);
  GuiButton* db = new GuiButton("down_arrow", dl1, dl2, dl3);
  db->set_scale(0.1);
  ub->set_behavior_event("demo-event-thing");
  db->set_behavior_event("demo-event-thing");
  lb1 = new GuiListBox("list_box", 4, ub, db);
  GuiLabel* l1 = GuiLabel::make_simple_text_label("hyena", font);
  GuiSign* s1 = new GuiSign("hyena", l1);
  s1->set_scale(0.1);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("dingo", font);
  GuiSign* s2 = new GuiSign("dingo", l2);
  s2->set_scale(0.1);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("jackal", font);
  GuiSign* s3 = new GuiSign("jackal", l3);
  s3->set_scale(0.1);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("wolf", font);
  GuiSign* s4 = new GuiSign("wolf", l4);
  s4->set_scale(0.1);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("fox", font);
  GuiSign* s5 = new GuiSign("fox", l5);
  s5->set_scale(0.1);
  float w, w1, w2;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w = (w1>w2)?w1:w2;
  w2 = l3->get_width();
  w = (w>w2)?w:w2;
  w2 = l4->get_width();
  w = (w>w2)?w:w2;
  w2 = l5->get_width();
  w = (w>w2)?w:w2;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  l5->set_width(w);
  ul1->set_background_color(0., 0., 0., 1.);
  ul2->set_background_color(0., 0., 0., 1.);
  ul3->set_background_color(0., 0., 0., 1.);
  ul2->set_foreground_color(1., 0., 0., 1.);
  ul3->set_foreground_color(1., 1., 1., 0.5);
  dl1->set_background_color(0., 0., 0., 1.);
  dl2->set_background_color(0., 0., 0., 1.);
  dl3->set_background_color(0., 0., 0., 1.);
  dl2->set_foreground_color(1., 0., 0., 1.);
  dl3->set_foreground_color(1., 1., 1., 0.5);
  l1->set_background_color(0., 0., 0., 1.);
  l2->set_background_color(0., 0., 0., 1.);
  l3->set_background_color(0., 0., 0., 1.);
  l4->set_background_color(0., 0., 0., 1.);
  l5->set_background_color(0., 0., 0., 1.);
  lb1->add_item(s1);
  lb1->add_item(s2);
  lb1->add_item(s3);
  lb1->add_item(s4);
  lb1->add_item(s5);
  lb1->thaw();
  lb1->manage(mgr, event_handler);
  lb1->start_behavior();
}

PT(GuiChooser) ch1;

static void test14(GuiManager* mgr, Node* font) {
  GuiLabel* nl1 = GuiLabel::make_simple_text_label("next", font);
  GuiLabel* nl2 = GuiLabel::make_simple_text_label("next", font);
  GuiLabel* nl3 = GuiLabel::make_simple_text_label("next", font);
  GuiButton* nb = new GuiButton("next_button", nl1, nl2, nl3);
  nb->set_scale(0.1);
  nb->set_pos(LVector3f::rfu(0.25, 0., -0.25));
  GuiLabel* pl1 = GuiLabel::make_simple_text_label("prev", font);
  GuiLabel* pl2 = GuiLabel::make_simple_text_label("prev", font);
  GuiLabel* pl3 = GuiLabel::make_simple_text_label("prev", font);
  GuiButton* pb = new GuiButton("prev_button", pl1, pl2, pl3);
  pb->set_scale(0.1);
  pb->set_pos(LVector3f::rfu(-0.25, 0., -0.25));
  nb->set_behavior_event("demo-event-thing");
  pb->set_behavior_event("demo-event-thing");
  ch1 = new GuiChooser("chooser", pb, nb);
  GuiLabel* l1 = GuiLabel::make_simple_text_label("hyena", font);
  GuiSign* s1 = new GuiSign("hyena", l1);
  s1->set_scale(0.1);
  GuiLabel* l2 = GuiLabel::make_simple_text_label("dingo", font);
  GuiSign* s2 = new GuiSign("dingo", l2);
  s2->set_scale(0.1);
  GuiLabel* l3 = GuiLabel::make_simple_text_label("jackal", font);
  GuiSign* s3 = new GuiSign("jackal", l3);
  s3->set_scale(0.1);
  GuiLabel* l4 = GuiLabel::make_simple_text_label("wolf", font);
  GuiSign* s4 = new GuiSign("wolf", l4);
  s4->set_scale(0.1);
  GuiLabel* l5 = GuiLabel::make_simple_text_label("fox", font);
  GuiSign* s5 = new GuiSign("fox", l5);
  s5->set_scale(0.1);
  float w, w1, w2;
  w1 = l1->get_width();
  w2 = l2->get_width();
  w = (w1>w2)?w1:w2;
  w2 = l3->get_width();
  w = (w>w2)?w:w2;
  w2 = l4->get_width();
  w = (w>w2)?w:w2;
  w2 = l5->get_width();
  w = (w>w2)?w:w2;
  l1->set_width(w);
  l2->set_width(w);
  l3->set_width(w);
  l4->set_width(w);
  l5->set_width(w);
  nl1->set_background_color(0., 0., 0., 1.);
  nl2->set_background_color(0., 0., 0., 1.);
  nl3->set_background_color(0., 0., 0., 1.);
  nl2->set_foreground_color(1., 0., 0., 1.);
  nl3->set_foreground_color(1., 1., 1., 0.5);
  pl1->set_background_color(0., 0., 0., 1.);
  pl2->set_background_color(0., 0., 0., 1.);
  pl3->set_background_color(0., 0., 0., 1.);
  pl2->set_foreground_color(1., 0., 0., 1.);
  pl3->set_foreground_color(1., 1., 1., 0.5);
  l1->set_background_color(0., 0., 0., 1.);
  l2->set_background_color(0., 0., 0., 1.);
  l3->set_background_color(0., 0., 0., 1.);
  l4->set_background_color(0., 0., 0., 1.);
  l5->set_background_color(0., 0., 0., 1.);
  ch1->add_item(s1);
  ch1->add_item(s2);
  ch1->add_item(s3);
  ch1->add_item(s4);
  ch1->add_item(s5);
  ch1->thaw();
  ch1->manage(mgr, event_handler);
}

static void setup_gui(void) {
  GuiManager* mgr = GuiManager::get_ptr(main_win, mak, (Node*)0L);
  PT_Node font = ModelPool::load_model("ttf-comic");
  // test 1
  //  test1(mgr, font);
  // test 2
  //  test2(mgr, font);
  // test 3
  //  test3(mgr, font);
  // test 4
  //  test4(mgr, font);
  // test 5
  //  test5(mgr, font);
  // test 6
  //  test6(mgr, font);
  // test 7
  //  test7(mgr, font);
  //  g_mgr = mgr;
  // test 8
  //  test8(mgr, font);
  //  g_mgr = mgr;
  // test 9
  //  test9(mgr, font);
  //  g_mgr = mgr;
  // test 10
  //  test10(mgr, font);
  // test 11
  //  test11(mgr, font);
  // test 12
  //  test12(mgr, font);
  // test 13
  //  test13(mgr, font);
  // test 14
  test14(mgr, font);
}

static void event_2(CPT_Event) {
  static bool is_setup = false;
  if (!is_setup) {
    setup_gui();
    is_setup = true;
  }
}

/*
// for test 7
static void event_3(CPT_Event) {
  if (frame_state) {
    f1->remove_item(b4);
    f1->add_item(b5);
    f1->pack_item(b5, GuiFrame::UNDER, b1);
    f1->pack_item(b5, GuiFrame::ALIGN_LEFT, b1);
    b5->manage(g_mgr, event_handler);
  } else {
    f1->remove_item(b5);
    f1->add_item(b4);
    f1->pack_item(b4, GuiFrame::UNDER, b1);
    f1->pack_item(b4, GuiFrame::ALIGN_LEFT, b1);
    b4->manage(g_mgr, event_handler);
  }
  f1->recompute();
  frame_state = !frame_state;
}
*/

/*
// for test 8
static void event_3(CPT_Event) {
  if (frame_state) {
    f1->remove_item(s4);
    f1->add_item(s5);
    f1->pack_item(s5, GuiFrame::UNDER, s1);
    f1->pack_item(s5, GuiFrame::ALIGN_LEFT, s1);
    s5->manage(g_mgr, event_handler);
  } else {
    f1->remove_item(s5);
    f1->add_item(s4);
    f1->pack_item(s4, GuiFrame::UNDER, s1);
    f1->pack_item(s4, GuiFrame::ALIGN_LEFT, s1);
    s4->manage(g_mgr, event_handler);
  }
  f1->recompute();
  frame_state = !frame_state;
}
*/

/*
// for test 9
static void event_3(CPT_Event) {
  if (frame_state) {
    l4->set_text("fox");
  } else {
    l4->set_text("wolf");
  }
  f1->recompute();
  frame_state = !frame_state;
}
*/

/*
// for test 10
static void event_3(CPT_Event) {
  if (prior_state) {
    s1->set_priority(GuiItem::P_High);
    s3->set_priority(GuiItem::P_Low);
  } else {
    s1->set_priority(GuiItem::P_Low);
    s3->set_priority(GuiItem::P_High);
  }
  prior_state = !prior_state;
}
*/

/*
// for test 11, 13
static void event_3(CPT_Event) {
  lb1->scroll_up();
  cout << *lb1;
}
*/

// for test 14
static void event_3(CPT_Event) {
  ch1->move_prev();
}

/*
// for test11, 13
static void event_4(CPT_Event) {
  lb1->scroll_down();
  cout << *lb1;
}
*/

// for test 14
static void event_4(CPT_Event) {
  ch1->move_next();
}

static void event_demo(CPT_Event) {
  cout << "got demo-event-thing event!" << endl;
}

void gui_keys(EventHandler&) {
  new RenderRelation( lights, dlight );
  have_dlight = true;

  event_handler.add_hook("2", event_2);
  // for tests 7-11, 13-14
  event_handler.add_hook("3", event_3);
  // for test 11, 13-14
  event_handler.add_hook("4", event_4);
  event_handler.add_hook("demo-event-thing", event_demo);
}

int main(int argc, char *argv[]) {
  define_keys = &gui_keys;
  return framework_main(argc, argv);
}
