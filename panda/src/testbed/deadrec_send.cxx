// Filename: deadrec_send.cxx
// Created by:  cary (15Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <renderRelation.h>
#include <queuedConnectionManager.h>
#include <modelPool.h>
#include <transformTransition.h>
#include <lerp.h>

#include <dconfig.h>

NotifyCategoryDeclNoExport(deadrec);
NotifyCategoryDef(deadrec, "");

Configure(deadrec);

ConfigureFn(deadrec) {
}

static PT_Node smiley;
static RenderRelation* my_arc;
static LPoint3f my_pos;
static LVector3f my_vel;
string hostname = deadrec.GetString("deadrec-rec", "localhost");
int hostport = deadrec.GetInt("deadrec-rec-port", 0xdead);

static QueuedConnectionManager cm;
PT(Connection) conn;
ConnectionWriter* writer;

enum TelemetryToken { T_End = 1, T_Pos, T_Vel, T_Num };

static inline NetDatagram& add_pos(NetDatagram& d) {
  d.add_uint8(T_Pos);
  d.add_float64(my_pos[0]);
  d.add_float64(my_pos[1]);
  d.add_float64(my_pos[2]);
  return d;
}

static inline NetDatagram& add_vel(NetDatagram& d) {
  d.add_uint8(T_Vel);
  d.add_float64(my_vel[0]);
  d.add_float64(my_vel[1]);
  d.add_float64(my_vel[2]);
  return d;
}

static inline void send(NetDatagram& d) {
  d.add_uint8(T_End);
  writer->send(d, conn);
}

static void event_frame(CPT_Event) {
  // send deadrec data
  NetDatagram d;
  send(add_pos(d));
}

static void deadrec_setup(void) {
  static bool done = false;
  if (done)
    return;
  // load smiley and put it in the scenegraph
  smiley = ModelPool::load_model("smiley");
  nassertv(smiley != (Node*)0L);
  my_arc = new RenderRelation(render, smiley);
  // open a connection to the receiver
  NetAddress host;
  if (!host.set_host(hostname, hostport)) {
    deadrec_cat->fatal() << "Unknown host: " << hostname << endl;
    exit(0);
  }
  conn = cm.open_TCP_client_connection(host, 5000);
  if (conn.is_null()) {
    deadrec_cat->fatal() << "no connection." << endl;
    exit(0);
  }
  if (deadrec_cat->is_debug())
    deadrec_cat->debug() << "opened TCP connection to " << hostname
			 << " on port " << conn->get_address().get_port()
			 << " and IP " << conn->get_address() << endl;
  writer = new ConnectionWriter(&cm, 0);
}

// the various motion generators

void update_smiley(void) {
  LMatrix4f mat = LMatrix4f::translate_mat(my_pos);
  my_arc->set_transition(new TransformTransition(mat));
}

enum MotionType { M_None, M_Line, M_Box, M_Circle, M_Random };
PT(AutonomousLerp) curr_lerp;
MotionType curr_type;

class MyPosFunctor : public LPoint3fLerpFunctor {
public:
  MyPosFunctor(LPoint3f start, LPoint3f end) : LPoint3fLerpFunctor(start,
								   end) {}
  MyPosFunctor(const MyPosFunctor& p) : LPoint3fLerpFunctor(p) {}
  virtual ~MyPosFunctor(void) {}
  virtual void operator()(float t) {
    LPoint3f p = interpolate(t);
    my_vel = p - my_pos;
    my_pos = p;
    update_smiley();
  }
public:
  // type stuff
  static TypeHandle get_class_type(void) { return _type_handle; }
  static void init_type(void) {
    LPoint3fLerpFunctor::init_type();
    register_type(_type_handle, "MyPosFunctor",
		  LPoint3fLerpFunctor::get_class_type());
  }
  virtual TypeHandle get_type(void) const { return get_class_type(); }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};
TypeHandle MyPosFunctor::_type_handle;

static void run_line(void) {
  static bool inited = false;
  static bool where = false;

  if (!inited) {
    MyPosFunctor::init_type();
    inited = true;
  }
  if (where) {
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos, LPoint3f::rfu(10., 0., 0.)),
			 5., new NoBlendType(), &event_handler);
    curr_lerp->set_end_event("lerp_done");
    curr_lerp->start();
  } else {
    curr_lerp =
      new AutonomousLerp(new MyPosFunctor(my_pos, LPoint3f::rfu(-10., 0., 0.)),
			 5., new NoBlendType(), &event_handler);
    curr_lerp->set_end_event("lerp_done");
    curr_lerp->start();
  }
  where = !where;
}

static void handle_lerp(void) {
  if (curr_lerp != (AutonomousLerp*)0L)
    curr_lerp->stop();
  curr_lerp = (AutonomousLerp*)0L;
  switch (curr_type) {
  case M_None:
    break;
  case M_Line:
    run_line();
    break;
  case M_Box:
    break;
  case M_Circle:
    break;
  case M_Random:
    break;
  default:
    deadrec_cat->error() << "unknown motion type (" << (int)curr_type << ")"
			 << endl;
  }
}

static void event_lerp(CPT_Event) {
  handle_lerp();
}

static void event_1(CPT_Event) {
  curr_type = M_Line;
  handle_lerp();
}

static void deadrec_keys(EventHandler& eh) {
  deadrec_setup();

  eh.add_hook("NewFrame", event_frame);
  eh.add_hook("1", event_1);
  eh.add_hook("lerp_done", event_lerp);
}

int main(int argc, char* argv[]) {
  define_keys = &deadrec_keys;
  return framework_main(argc, argv);
}
