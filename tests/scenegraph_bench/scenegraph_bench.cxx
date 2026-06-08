/**
 * Macro benchmark for the threaded PipelineCycler.  Renders a large instanced
 * scene offscreen through tinydisplay with a tiny render target, so the cull
 * traversal (cycler reads) dominates over rasterization, and drives a workload
 * -- transform animation or procedural node generation -- either on the main
 * thread or on background worker threads, under a configurable render
 * threading-model.  Reports fps, frame-time percentiles, and workload
 * throughput so the same source can be run across threading scenarios and on a
 * pre-EBR build for comparison.
 *
 * Exit: 0 = ran, 77 = no graphics pipe (skip), 1 = arg/setup error.
 */

#include "pandaFramework.h"
#include "load_prc_file.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "transformState.h"
#include "geom.h"
#include "geomNode.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "trueClock.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <deque>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Options {
  double duration = 5.0;
  std::string threading_model;          // "" / "Cull" / "cull/draw"
  std::string display;                  // "" (tinydisplay aux) / pandagl / ...
  int objects = 20000;                  // initial animated scene size
  int groups = 8;
  std::string workload = "animate";     // "animate" | "generate"
  int workers = 0;                      // 0 = workload on main thread
  bool shared = false;                  // animate: workers share the object pool
  int anim_per_frame = 2000;            // main-thread animate batch
  int gen_batch = 200;                  // nodes per generate step
  int gen_window = 4000;                // per-generator live-node cap
  int readers = 0;                      // read-only traverser threads
  bool always_cow = false;              // force pipeline-always-cow
  bool complex = false;                 // build a deep, varied static scene
  int depth = 7;                        // complex: hierarchy depth
  int branch = 4;                       // complex: children per internal node
};

std::atomic<bool> g_stop{false};
std::atomic<long long> g_anim_ops{0};
std::atomic<long long> g_gen_nodes{0};
std::atomic<long long> g_read_ops{0};

// One shared single-triangle Geom, instanced into every object.  Real geometry
// (rather than a synthetic set_bounds()) gives every node a genuine, cached
// bounding volume computed the same way Panda computes it in production, and
// keeps the draw thread doing real (if tiny) work -- so both the cull and draw
// pipeline stages exercise the cycler.  Built once before any worker starts.
PT(Geom) g_geom;

void build_shared_geom() {
  PT(GeomVertexData) vdata =
    new GeomVertexData("tri", GeomVertexFormat::get_v3(), Geom::UH_static);
  vdata->set_num_rows(3);
  GeomVertexWriter vw(vdata, "vertex");
  vw.add_data3(0.0f, 0.0f, 0.0f);
  vw.add_data3(1.0f, 0.0f, 0.0f);
  vw.add_data3(0.0f, 0.0f, 1.0f);
  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
  tris->add_vertices(0, 1, 2);
  tris->close_primitive();
  g_geom = new Geom(vdata);
  g_geom->add_primitive(tris);
}

NodePath make_object(NodePath parent, std::mt19937 &rng) {
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);
  PT(GeomNode) gn = new GeomNode("obj");
  gn->add_geom(g_geom);
  NodePath obj = parent.attach_new_node(gn);
  obj.set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
  return obj;
}

// Build a deep, varied static subtree: every internal node carries a
// non-identity transform (set_hpr) and a RenderState (set_color), so the cull
// traversal performs real net-transform (Eigen) and net-state composition at
// each node -- representative of a complex scene's steady-state render cost,
// without any per-frame scene mutation.  Leaves carry the shared geom.
void build_complex(NodePath parent, int depth, int branch, std::mt19937 &rng,
                   std::vector<NodePath> &leaves) {
  std::uniform_real_distribution<float> pos(-8.0f, 8.0f);
  std::uniform_real_distribution<float> ang(0.0f, 360.0f);
  std::uniform_real_distribution<float> col(0.2f, 1.0f);
  if (depth <= 0) {
    PT(GeomNode) gn = new GeomNode("leaf");
    gn->add_geom(g_geom);
    NodePath np = parent.attach_new_node(gn);
    np.set_pos(pos(rng), pos(rng), pos(rng));
    leaves.push_back(np);
    return;
  }
  NodePath node = parent.attach_new_node("n");
  node.set_pos(pos(rng), pos(rng), pos(rng));
  node.set_hpr(ang(rng), ang(rng), ang(rng));
  node.set_color(col(rng), col(rng), col(rng), 1.0f);
  for (int i = 0; i < branch; ++i) {
    build_complex(node, depth - 1, branch, rng, leaves);
  }
}

void animate_worker(std::vector<NodePath> objs, int seed) {
  std::mt19937 rng((unsigned)seed * 2654435761u + 1u);
  std::uniform_int_distribution<size_t> pick(0, objs.empty() ? 0 : objs.size() - 1);
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed) && !objs.empty()) {
    NodePath &o = objs[pick(rng)];
    o.set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
    ++n;
  }
  g_anim_ops.fetch_add(n, std::memory_order_relaxed);
}

// Procedural generation: stream subtrees in under a private root, removing the
// oldest once the live window is exceeded (models async load/unload).
void generate_worker(NodePath gen_root, int seed, int batch, int window) {
  std::mt19937 rng((unsigned)seed * 40503u + 7u);
  std::deque<NodePath> live;
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed)) {
    for (int i = 0; i < batch; ++i) {
      live.push_back(make_object(gen_root, rng));
      ++n;
      if ((int)live.size() > window) {
        live.front().remove_node();
        live.pop_front();
      }
    }
  }
  g_gen_nodes.fetch_add(n, std::memory_order_relaxed);
}

void reader_worker(std::vector<NodePath> objs, int seed) {
  long long n = 0;
  while (!g_stop.load(std::memory_order_relaxed)) {
    for (const NodePath &o : objs) {
      const TransformState *t = o.node()->get_transform();
      n += (t != nullptr);
    }
  }
  g_read_ops.fetch_add(n, std::memory_order_relaxed);
}

int parse(int argc, char **argv, Options &o) {
  for (int i = 1; i < argc; ++i) {
    auto next = [&](int &dst) { if (i + 1 < argc) dst = atoi(argv[++i]); };
    if (!strcmp(argv[i], "--duration") && i + 1 < argc) o.duration = atof(argv[++i]);
    else if (!strcmp(argv[i], "--threading-model") && i + 1 < argc) o.threading_model = argv[++i];
    else if (!strcmp(argv[i], "--display") && i + 1 < argc) o.display = argv[++i];
    else if (!strcmp(argv[i], "--objects")) next(o.objects);
    else if (!strcmp(argv[i], "--groups")) next(o.groups);
    else if (!strcmp(argv[i], "--workload") && i + 1 < argc) o.workload = argv[++i];
    else if (!strcmp(argv[i], "--workers")) next(o.workers);
    else if (!strcmp(argv[i], "--shared")) o.shared = true;
    else if (!strcmp(argv[i], "--anim-per-frame")) next(o.anim_per_frame);
    else if (!strcmp(argv[i], "--gen-batch")) next(o.gen_batch);
    else if (!strcmp(argv[i], "--gen-window")) next(o.gen_window);
    else if (!strcmp(argv[i], "--readers")) next(o.readers);
    else if (!strcmp(argv[i], "--always-cow")) o.always_cow = true;
    else if (!strcmp(argv[i], "--complex")) o.complex = true;
    else if (!strcmp(argv[i], "--depth")) next(o.depth);
    else if (!strcmp(argv[i], "--branch")) next(o.branch);
    else { fprintf(stderr, "unknown arg: %s\n", argv[i]); return 1; }
  }
  return 0;
}

}  // namespace

int main(int argc, char **argv) {
  Options o;
  if (parse(argc, argv, o) != 0) return 1;

  std::string prc =
      "window-type offscreen\n";
  if (!o.display.empty()) {
    prc += "load-display " + o.display + "\n";
  }
  prc +=
      "aux-display p3tinydisplay\n"
      "win-size 16 16\n"
      "notify-level-pgraph fatal\n"
      "notify-level-display fatal\n"
      "notify-level-loader fatal\n"
      "notify-level-egg2pg fatal\n";
  if (!o.threading_model.empty()) {
    prc += "threading-model " + o.threading_model + "\n";
  }
  if (o.always_cow) {
    prc += "pipeline-always-cow true\n";
  }
  load_prc_file_data("scenegraph_bench", prc);

  PandaFramework framework;
  framework.open_framework(argc, argv);
  WindowFramework *window = framework.open_window();
  if (window == nullptr) {
    fprintf(stderr, "skip: no graphics pipe / no window backend available\n");
    return 77;
  }
  NodePath render = window->get_render();

  build_shared_geom();

  std::mt19937 rng(12345);

  // Build the initial scene: `groups` subtrees of instanced objects.  For the
  // generate workload this is just a small static backdrop; for animate it is
  // the pool the workers move.
  std::vector<std::vector<NodePath>> group_objs(std::max(1, o.groups));
  std::vector<NodePath> all_objs;
  int init_objects;
  if (o.complex) {
    // One deep, varied static tree (transforms + states at every node).  Leaves
    // populate all_objs.  Intended for steady-state render measurement with
    // --workers 0 --anim-per-frame 0 (no per-frame scene mutation).  Anchor it
    // in front of the camera so most of it stays in the frustum and the cull
    // actually traverses (and reads) the nodes, rather than skipping subtrees.
    NodePath croot = render.attach_new_node("complex-root");
    croot.set_pos(0.0f, 120.0f, 0.0f);
    build_complex(croot, o.depth, o.branch, rng, all_objs);
    init_objects = (int)all_objs.size();
  } else {
    init_objects = (o.workload == "generate") ? std::min(o.objects, 2000) : o.objects;
    for (int g = 0; g < (int)group_objs.size(); ++g) {
      NodePath grp = render.attach_new_node("group");
      int count = init_objects / (int)group_objs.size();
      for (int i = 0; i < count; ++i) {
        NodePath obj = make_object(grp, rng);
        group_objs[g].push_back(obj);
        all_objs.push_back(obj);
      }
    }
  }

  // Roots for procedural generation (one per generator worker, or one for main).
  std::vector<NodePath> gen_roots;
  int n_gen = (o.workload == "generate") ? std::max(1, o.workers) : 0;
  for (int i = 0; i < n_gen; ++i) {
    gen_roots.push_back(render.attach_new_node("gen-root"));
  }

  fprintf(stderr,
    "[bench] workload=%s workers=%d threading-model=\"%s\" objects=%d groups=%d "
    "readers=%d duration=%.1fs\n",
    o.workload.c_str(), o.workers, o.threading_model.c_str(),
    init_objects, o.groups, o.readers, o.duration);

  // Spawn workload + reader threads.
  std::vector<std::thread> threads;
  if (o.workload == "animate" && o.workers > 0) {
    for (int w = 0; w < o.workers; ++w) {
      std::vector<NodePath> mine = o.shared ? all_objs : group_objs[w % group_objs.size()];
      threads.emplace_back(animate_worker, std::move(mine), w + 1);
    }
  } else if (o.workload == "generate") {
    for (int w = 0; w < o.workers; ++w) {
      threads.emplace_back(generate_worker, gen_roots[w], w + 1, o.gen_batch, o.gen_window);
    }
  }
  for (int r = 0; r < o.readers; ++r) {
    threads.emplace_back(reader_worker, all_objs, 1000 + r);
  }

  // Main loop: render frames for the duration, doing the workload inline when
  // there are no worker threads.  Record per-frame times for percentiles.
  Thread *current_thread = Thread::get_current_thread();
  std::vector<double> frame_ms;
  frame_ms.reserve(100000);
  std::deque<NodePath> main_gen_live;
  std::uniform_int_distribution<size_t> pick(0, all_objs.empty() ? 0 : all_objs.size() - 1);
  std::uniform_real_distribution<float> pos(-50.0f, 50.0f);

  // Warm up: the first frames pay one-time setup (geom munge, state compose,
  // bounds init) that would otherwise swamp the steady-state measurement.
  for (int i = 0; i < 30; ++i) {
    framework.do_frame(current_thread);
  }

  TrueClock *clock = TrueClock::get_global_ptr();
  double t_start = clock->get_long_time();
  while (clock->get_long_time() - t_start < o.duration) {
    if (o.workers == 0) {
      if (o.workload == "animate") {
        for (int i = 0; i < o.anim_per_frame && !all_objs.empty(); ++i) {
          all_objs[pick(rng)].set_pos(pos(rng), pos(rng) + 200.0f, pos(rng));
        }
        g_anim_ops.fetch_add(o.anim_per_frame, std::memory_order_relaxed);
      } else {  // generate on the main thread
        for (int i = 0; i < o.gen_batch; ++i) {
          main_gen_live.push_back(make_object(gen_roots[0], rng));
          if ((int)main_gen_live.size() > o.gen_window) {
            main_gen_live.front().remove_node();
            main_gen_live.pop_front();
          }
        }
        g_gen_nodes.fetch_add(o.gen_batch, std::memory_order_relaxed);
      }
    }
    double f0 = clock->get_long_time();
    framework.do_frame(current_thread);
    frame_ms.push_back((clock->get_long_time() - f0) * 1000.0);
  }
  double elapsed = clock->get_long_time() - t_start;

  g_stop.store(true);
  for (auto &t : threads) t.join();

  // Report.
  std::sort(frame_ms.begin(), frame_ms.end());
  auto pct = [&](double p) {
    if (frame_ms.empty()) return 0.0;
    size_t idx = (size_t)(p * (frame_ms.size() - 1));
    return frame_ms[idx];
  };
  size_t frames = frame_ms.size();
  double fps = frames / elapsed;
  long long anim = g_anim_ops.load(), gen = g_gen_nodes.load(), reads = g_read_ops.load();

  printf("%-9s %5d %-9s %8zu %8.1f %8.3f %8.3f %12.0f %12.0f %12.0f\n",
         o.workload.c_str(), o.workers,
         o.threading_model.empty() ? "single" : o.threading_model.c_str(),
         frames, fps, pct(0.50), pct(0.99),
         anim / elapsed, gen / elapsed, reads / elapsed);
  fflush(stdout);

  std::_Exit(0);  // skip framework teardown (can touch threading internals post-run)
}
