// Filename: deadrec_rec.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <renderRelation.h>
#include <queuedConnectionManager.h>
#include <queuedConnectionListener.h>
#include <modelPool.h>
#include <ipc_thread.h>
#include <transformTransition.h>
#include <lerp.h>
#include <guiFrame.h>
#include <guiButton.h>
#include <guiSign.h>
#include <clockObject.h>

#include <dconfig.h>

NotifyCategoryDeclNoExport(deadrec);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

typedef set<PT(Connection)> Clients;

static PT_Node smiley;
static RenderRelation* my_arc;
static LPoint3f my_pos;
static int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);
static thread* monitor;
static bool stop_monitoring;
QueuedConnectionListener* listener;
QueuedConnectionManager cm;
Clients clients;
QueuedConnectionReader* reader;
static float clock_skew = 0.;
static bool doing_sync = false;
static float my_time;

enum TelemetryToken { T_End = 1, T_Pos, T_Vel, T_Num, T_Time, T_Sync };
enum PredictToken { P_Null, P_Linear };
enum CorrectToken { C_Pop, C_Lerp, C_Spline };

PT(AutonomousLerp) curr_lerp;
PredictToken curr_pred, pred_switch;
CorrectToken curr_corr, corr_switch;
PT(GuiButton) pnullButton;
PT(GuiButton) plinearButton;
PT(GuiButton) cpopButton;
PT(GuiButton) clerpButton;
PT(GuiButton) csplineButton;

static inline unsigned char* get_uint8(unsigned char* b, unsigned char& v) {
  v = b[0];
  return ++b;
}

static inline unsigned char* get_float64(unsigned char* b, float& f) {
  unsigned char t[8]; // 64-bits
  memcpy(t, b, 8);
  if (sizeof(float)==8) {
    memcpy(&f, t, 8);
  } else if (sizeof(double)==8) {
    double d;
    memcpy(&d, t, 8);
    f = d;
  } else {
    deadrec_cat->error() << "neither float or double are 64-bit" << endl;
    f = 0.;
  }
  return b+8;
}

static void* internal_monitor(void*) {
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "internal monitoring thread started" << endl;
  while (!stop_monitoring) {
    // check for new clients
    while (listener->new_connection_available()) {
      PT(Connection) rv;
      NetAddress address;
      PT(Connection) new_connection;
      if (listener->get_new_connection(rv, address, new_connection)) {
	if (deadrec_cat->is_debug())
	  deadrec_cat->debug() << "Got connection from " << address << endl;
	reader->add_connection(new_connection);
	clients.insert(new_connection);
      }
    }
    // check for reset clients
    while (cm.reset_connection_available()) {
      PT(Connection) connection;
      if (cm.get_reset_connection(connection)) {
	if (deadrec_cat->is_debug())
	  deadrec_cat->debug() << "Lost connection from "
			       << connection->get_address() << endl;
	clients.erase(connection);
	cm.close_connection(connection);
      }
    }
    // process all available datagrams
    while (reader->data_available()) {
      NetDatagram datagram;
      if (reader->get_data(datagram)) {
	unsigned char* buff = (unsigned char*)(datagram.get_data());
	unsigned char byte;
	TelemetryToken t;
	buff = get_uint8(buff, byte);
	t = (TelemetryToken)byte;
	while (t != T_End) {
	  switch (t) {
	  case T_Pos:
	    {
	      float x, y, z;
	      buff = get_float64(get_float64(get_float64(buff, x), y), z);
	      my_pos = LPoint3f(x, y, z);
	    }
	    break;
	  case T_Vel:
	    if (deadrec_cat->is_debug())
	      deadrec_cat->debug() << "got T_Num" << endl;
	    break;
	  case T_Num:
	    if (deadrec_cat->is_debug())
	      deadrec_cat->debug() << "got T_Num" << endl;
	    break;
	  case T_Time:
	    {
	      float x;
	      buff = get_float64(buff, x);
	      if (doing_sync) {
		clock_skew = ClockObject::get_global_clock()->get_time() - x;
		doing_sync = false;
		cerr << "setting clock skew to: " << clock_skew << endl;
	      } else
		my_time = x + clock_skew;
	    }
	    break;
	  case T_Sync:
	    doing_sync = true;
	    break;
	  default:
	    deadrec_cat->warning() << "got bad token in datagram (" << (int)t
				   << ")" << endl;
	  }
	  buff = get_uint8(buff, byte);
	  t = (TelemetryToken)byte;
	}
	// unpack and deal with the datagram now
	// DO THIS
	// part of this includes logic on when to shutdown, I hope
      }
    }
    // sleep for about 100 milliseconds
    ipc_traits::sleep(0, 100000);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "internal monitoring thread exiting" << endl;
  return (void*)0L;
}

static void predict_event(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-down"));
}

static void correct_event(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-down"));
}

typedef void event_func(CPT_Event);

static inline GuiButton* make_button(const string& name, Node* font,
				     EventHandler& eh, event_func func) {
  GuiLabel* l1 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l2 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l3 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l4 = GuiLabel::make_simple_text_label(name, font);
  GuiLabel* l5 = GuiLabel::make_simple_text_label(name, font);
  // up
  l1->set_background_color(1., 1., 1., 0.);
  // up-rollover
  l2->set_background_color(1., 1., 1., 0.5);
  // down
  l3->set_background_color(1., 1., 1., 0.);
  // down-rollover
  l4->set_background_color(1., 1., 1., 0.5);
  // 'inactive'
  l5->set_background_color(1., 0., 0., 0.7);
  GuiButton* b1 = new GuiButton(name, l1, l2, l3, l4, l5);
  eh.add_hook(name + "-down", func);
  eh.add_hook(name + "-down-rollover", func);
  return b1;
}

static void deadrec_setup(EventHandler& eh) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
  // prepair to get a connection
  PT(Connection) rendezvous = cm.open_TCP_server_rendezvous(hostport, 5);
  if (rendezvous.is_null()) {
    deadrec_cat->fatal() << "cannot get port " << hostport << endl;
    exit(0);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "Listening for connections on port " << hostport
			 << endl;
  listener = new QueuedConnectionListener(&cm, 0);
  listener->add_connection(rendezvous);
  reader = new QueuedConnectionReader(&cm, 1);
  stop_monitoring = false;
  monitor = thread::create(internal_monitor, (void*)0L,
			   thread::PRIORITY_NORMAL);
  // create an interface
  GuiManager* mgr = GuiManager::get_ptr(main_win, mak);
  PT_Node font = ModelPool::load_model("ttf-comic");
  GuiFrame* f1 = new GuiFrame("predicters");
  GuiLabel* l1 = GuiLabel::make_simple_text_label("predicter:", font);
  l1->set_background_color(1., 1., 1., 0.);
  GuiSign* s1 = new GuiSign("predicter", l1);
  s1->set_scale(0.08);
  f1->add_item(s1);
  pnullButton = make_button("null", font, eh, predict_event);
  pnullButton->set_scale(0.08);
  f1->add_item(pnullButton);
  plinearButton = make_button("linear", font, eh, predict_event);
  plinearButton->set_scale(0.08);
  f1->add_item(plinearButton);
  f1->pack_item(pnullButton, GuiFrame::UNDER, s1);
  f1->pack_item(pnullButton, GuiFrame::ALIGN_LEFT, s1);
  f1->pack_item(plinearButton, GuiFrame::UNDER, s1);
  f1->pack_item(plinearButton, GuiFrame::RIGHT, pnullButton, 0.02);
  f1->align_to_left(0.05);
  f1->align_to_top(0.05);
  f1->recompute();
  f1->manage(mgr, eh);
  GuiFrame* f2 = new GuiFrame("correctors");
  GuiLabel* l2 = GuiLabel::make_simple_text_label("corrector:", font);
  l2->set_background_color(1., 1., 1., 0.);
  GuiSign* s2 = new GuiSign("corrector", l2);
  s2->set_scale(0.08);
  f2->add_item(s2);
  cpopButton = make_button("pop", font, eh, correct_event);
  cpopButton->set_scale(0.08);
  f2->add_item(cpopButton);
  clerpButton = make_button("lerp", font, eh, correct_event);
  clerpButton->set_scale(0.08);
  f2->add_item(clerpButton);
  csplineButton = make_button("spline", font, eh, correct_event);
  csplineButton->set_scale(0.08);
  f2->add_item(csplineButton);
  f2->pack_item(cpopButton, GuiFrame::UNDER, s2);
  f2->pack_item(cpopButton, GuiFrame::ALIGN_LEFT, s2);
  f2->pack_item(clerpButton, GuiFrame::UNDER, s2);
  f2->pack_item(clerpButton, GuiFrame::RIGHT, cpopButton, 0.02);
  f2->pack_item(csplineButton, GuiFrame::UNDER, s2);
  f2->pack_item(csplineButton, GuiFrame::RIGHT, clerpButton, 0.02);
  f2->align_to_left(0.05);
  f2->align_to_bottom(0.05);
  f2->recompute();
  f2->manage(mgr, eh);
}

static void update_smiley(void) {
  LMatrix4f mat = LMatrix4f::translate_mat(my_pos);
  my_arc->set_transition(new TransformTransition(mat));
}

static void event_frame(CPT_Event) {
  update_smiley();
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup(eh);

  eh.add_hook("NewFrame", event_frame);
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
