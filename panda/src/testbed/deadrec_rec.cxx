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
static LPoint3f target_pos;
static LPoint3f telemetry_pos;
static LVector3f my_vel;
static LVector3f target_vel;
static LVector3f telemetry_vel;
static int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);
static thread* monitor;
static bool stop_monitoring;
QueuedConnectionListener* listener;
QueuedConnectionManager cm;
Clients clients;
QueuedConnectionReader* reader;
static float clock_skew = 0.;
static bool doing_sync = false;
static float my_time, target_time, telemetry_time;
static bool new_telemetry;
static bool reinit_correction, reinit_prediction;

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
	      telemetry_pos = LPoint3f(x, y, z);
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
		telemetry_time = x + clock_skew;
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
	new_telemetry = true;
      }
    }
    // sleep for about 100 milliseconds
    ipc_traits::sleep(0, 100000);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "internal monitoring thread exiting" << endl;
  return (void*)0L;
}

static void predict_event_up(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-up"));
  event_handler.remove_hook(s + "-up", predict_event_up);
  event_handler.remove_hook(s + "-up-rollover", predict_event_up);
  switch (curr_pred) {
  case P_Null:
    pnullButton->up();
    break;
  case P_Linear:
    plinearButton->up();
    break;
  }
  curr_pred = pred_switch;
  switch (curr_pred) {
  case P_Null:
    pnullButton->inactive();
    break;
  case P_Linear:
    plinearButton->inactive();
    break;
  default:
    deadrec_cat->error() << "switching predictor to invalid type ("
			 << (int)curr_pred << ")" << endl;
  }
  reinit_prediction = true;
}

static void predict_event(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-down"));
  if (s == "null") {
    if (curr_pred != P_Null) {
      pred_switch = P_Null;
      event_handler.add_hook(s + "-up", predict_event_up);
      event_handler.add_hook(s + "-up-rollover", predict_event_up);
    }
  } else if (s == "linear") {
    if (curr_pred != P_Linear) {
      pred_switch = P_Linear;
      event_handler.add_hook(s + "-up", predict_event_up);
      event_handler.add_hook(s + "-up-rollover", predict_event_up);
    }
  } else {
    deadrec_cat->error() << "got invalid button event '" << s << "'" << endl;
  }
}

static void correct_event_up(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-up"));
  event_handler.remove_hook(s + "-up", correct_event_up);
  event_handler.remove_hook(s + "-up-rollover", correct_event_up);
  switch (curr_corr) {
  case C_Pop:
    cpopButton->up();
    break;
  case C_Lerp:
    clerpButton->up();
    break;
  case C_Spline:
    csplineButton->up();
    break;
  }
  curr_corr = corr_switch;
  switch (curr_corr) {
  case C_Pop:
    cpopButton->inactive();
    break;
  case C_Lerp:
    clerpButton->inactive();
    break;
  case C_Spline:
    csplineButton->inactive();
    break;
  default:
    deadrec_cat->error() << "switching corrector to invalid type ("
			 << (int)curr_corr << ")" << endl;
  }
  reinit_correction = true;
}

static void correct_event(CPT_Event e) {
  string s = e->get_name();
  s = s.substr(0, s.find("-down"));
  if (s == "pop") {
    if (curr_corr != C_Pop) {
      corr_switch = C_Pop;
      event_handler.add_hook(s + "-up", correct_event_up);
      event_handler.add_hook(s + "-up-rollover", correct_event_up);
    }
  } else if (s == "lerp") {
    if (curr_corr != C_Lerp) {
      corr_switch = C_Lerp;
      event_handler.add_hook(s + "-up", correct_event_up);
      event_handler.add_hook(s + "-up-rollover", correct_event_up);
    }
  } else if (s == "spline") {
    if (curr_corr != C_Spline) {
      corr_switch = C_Spline;
      event_handler.add_hook(s + "-up", correct_event_up);
      event_handler.add_hook(s + "-up-rollover", correct_event_up);
    }
  } else {
    deadrec_cat->error() << "got invalid button event '" << s << "'" << endl;
  }
}

typedef void (*event_func)(CPT_Event);

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
  curr_pred = P_Null;
  pnullButton->inactive();
  curr_corr = C_Pop;
  cpopButton->inactive();
  reinit_correction = true;
  reinit_prediction = true;
}

inline static void predict_null(void) {
  static bool have_vel = false;

  if (reinit_prediction) {
    have_vel = false;
    target_vel = LVector3f(0., 0., 0.);
    reinit_prediction = false;
  }
  if (have_vel) {
    if (new_telemetry)
      target_vel = target_pos - telemetry_pos;
  } else {
    if (new_telemetry)
      have_vel = true;
  }
  target_pos = telemetry_pos;
}

inline static void predict_linear(void) {
  static int state = 0;
  static LPoint3f A, B;
  static LVector3f V;
  static float A_time, B_time;
  static float time = 0.;

  if (reinit_prediction) {
    state = 0;
    reinit_prediction = false;
  }
  switch (state) {
  case 0:
    if (new_telemetry) {
      A = telemetry_pos;
      A_time = telemetry_time;
      V = LVector3f(0., 0., 0.);
      state = 1;
    }
    break;
  case 1:
    if (new_telemetry) {
      B = telemetry_pos;
      B_time = telemetry_time;
      V = B - A;
      V *= 1. / (B_time - A_time);
      time = 0.5;
      state = 2;
    }
    target_pos = A;
    target_vel = V;
    break;
  case 2:
    if (new_telemetry) {
      if (telemetry_time < A_time) {
	// before our two samples, ignore it
      } else if (telemetry_time > B_time) {
	// a sample in brave new territory
	A = B;
	A_time = B_time;
	B = telemetry_pos;
	B_time = telemetry_time;
	V = B - A;
	V *= 1. / (B_time - A_time);
	time = 0.;
      } else {
	// is between our two samples
	A = telemetry_pos;
	A_time = telemetry_time;
	V = B - A;
	V *= 1. / (B_time - A_time);
	time = 0.;
      }
    }
    if (time <= 0.) {
      float rtime = ClockObject::get_global_clock()->get_time() - A_time;
      target_pos = (rtime * V) + A;
      target_vel = V;
      time = 0.5;
    }
    time -= ClockObject::get_global_clock()->get_dt();
    break;
  default:
    deadrec_cat->error() << "got in invalid state in linear predictor ("
			 << state << ")" << endl;
  }
}

inline static void run_predict(void) {
  switch (curr_pred) {
  case P_Null:
    predict_null();
    break;
  case P_Linear:
    predict_linear();
    break;
  default:
    deadrec_cat->error() << "bad prediction type (" << (int)curr_pred << ")"
			 << endl;
  }
}

inline static void correction_pop(void) {
  my_pos = target_pos;
  reinit_correction = false;
}

inline static void correction_lerp(void) {
  static LPoint3f prev_pos, save_pos;
  static bool have_both = false;
  static float time;

  if (reinit_correction) {
    if (save_pos != target_pos) {
      prev_pos = save_pos = target_pos;
      reinit_correction = false;
      have_both = false;
    }
  } else {
    if (have_both) {
      if (save_pos != target_pos) {
	time = 0.;
	prev_pos = my_pos;
	save_pos = target_pos;
      } else {
	if (time < 0.5) {
	  // half second lerp
	  float tmp = time * 2.;
	  LVector3f vtmp = save_pos - prev_pos;
	  my_pos = (tmp * vtmp) + prev_pos;
	  time += ClockObject::get_global_clock()->get_dt();
	}
      }
    } else {
      if (save_pos != target_pos) {
	save_pos = target_pos;
	my_pos = prev_pos;
	time = 0.;
	have_both = true;
      }
    }
  }
}

inline static void correction_spline(void) {
  static LPoint3f A, B, C, D;
  static bool have_both = false;
  static LPoint3f prev_pos, save_pos;
  static LVector3f prev_vel, save_vel;
  static float time;

  if (reinit_correction) {
    if (save_pos != target_pos) {
      prev_pos = save_pos = target_pos;
      prev_vel = save_vel = target_vel;
      reinit_correction = false;
      have_both = false;
    }
  } else {
    if (have_both) {
      if (save_pos != target_pos) {
	time = 0.;
	prev_pos = my_pos;
	prev_vel = my_vel;
	save_pos = target_pos;
	save_vel = target_vel;
	A = (2. * (prev_pos - save_pos)) + prev_vel + save_vel;
	B = (3. * (save_pos - prev_pos)) - (2. * prev_vel) - save_vel;
	C = prev_vel;
	D = prev_pos;
      } else {
	if (time < 0.5) {
	  // half second lerp
	  float tmp = time * 2.;
	  my_pos = (tmp * tmp * tmp * A) + (tmp * tmp * B) + (tmp * C) + D;
	  my_vel = (3. * tmp * tmp * A) + (2. * tmp * B) + C;
	  time += ClockObject::get_global_clock()->get_dt();
	}
      }
    } else {
      if (save_pos != target_pos) {
	save_pos = target_pos;
	save_vel = target_vel;
	my_pos = prev_pos;
	my_vel = prev_vel;
	time = 0.;
	A = (2. * (prev_pos - save_pos)) + prev_vel + save_vel;
	B = (3. * (save_pos - prev_pos)) - (2. * prev_vel) - save_vel;
	C = prev_vel;
	D = prev_pos;
	have_both = true;
      }
    }
  }
}

inline static void run_correct(void) {
  switch (curr_corr) {
  case C_Pop:
    correction_pop();
    break;
  case C_Lerp:
    correction_lerp();
    break;
  case C_Spline:
    correction_spline();
    break;
  default:
    deadrec_cat->error() << "bad correction type (" << (int)curr_corr << ")"
			 << endl;
  }
}

inline static void update_smiley(void) {
  LMatrix4f mat = LMatrix4f::translate_mat(my_pos);
  my_arc->set_transition(new TransformTransition(mat));
}

static void event_frame(CPT_Event) {
  run_predict();
  run_correct();
  update_smiley();
  if (new_telemetry)
    new_telemetry = false;
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup(eh);

  eh.add_hook("NewFrame", event_frame);
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
