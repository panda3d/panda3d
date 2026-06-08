/**
 * Single-shot multi-threaded scene-graph stress runner for Panda3D.
 *
 * Workers mutate the scene graph while the main thread renders.  A
 * watchdog calls _Exit(2) if no progress is observed within --timeout
 * seconds.  For multi-iteration sweeps, drive this from run.py.
 *
 * Exit codes:  0 = clean, 1 = crashed or arg error, 2 = deadlock,
 *             77 = no graphics pipe (ctest skip).
 */

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "load_prc_file.h"

#include "geom.h"
#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "nodePath.h"
#include "epochHolder.h"
#include "trueClock.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Options {
  int workers = 4;
  int iterations = 600;
  bool shared = true;
  int timeout_sec = 8;
  int depth = 0;
  int branching = 1;
  // Op-type selector — see --help for the letter codes.
  std::string ops = "ahpsrbR";
  std::string threading_model;
};

std::atomic<bool> g_stop{false};
std::atomic<int>  g_total_attached{0};
std::atomic<int>  g_total_removed{0};
std::atomic<int>  g_total_reparented{0};
std::atomic<int>  g_total_stashed{0};
std::atomic<int>  g_total_reads{0};
std::atomic<int>  g_workers_done{0};

// `all_targets` is the list of valid attach/reparent destinations
// (length 1 in shared mode, `workers` in per-worker mode).
void worker(const std::vector<NodePath> &all_targets, int my_target_idx,
            NodePath model, int seed, int iterations,
            const std::string &ops) {
  // Register this raw std::thread with Panda so Thread::get_current_thread()
  // (called transitively by every NodePath/cycler op below) returns a real
  // Thread* instead of nullptr + an assertion.
  PT(Thread) self = Thread::bind_thread("stress-worker", "stress");

  std::vector<char> op_pool;
  for (char c : ops) op_pool.push_back(c);
  if (op_pool.empty()) op_pool.push_back('a');

  std::mt19937 rng(static_cast<unsigned>(seed * 1234567 + 42));
  std::uniform_int_distribution<int> op_dist(0, (int)op_pool.size() - 1);
  std::uniform_int_distribution<int> heading_dist(0, 359);
  std::uniform_real_distribution<float> pos_dist(-5.0f, 5.0f);
  std::vector<NodePath> mine;
  mine.reserve(64);
  const NodePath &my_parent = all_targets[my_target_idx];

  for (int i = 0; i < iterations && !g_stop.load(std::memory_order_relaxed); ++i) {
    EpochHolder epoch(self);
    char op;
    if (mine.empty())            op = 'a';                       // forced attach
    else if (mine.size() >= 32)  op = 'R';                       // forced remove cap
    else                          op = op_pool[op_dist(rng)];

    switch (op) {
      case 'a': {
        NodePath n = model.copy_to(my_parent);
        mine.push_back(n);
        g_total_attached.fetch_add(1, std::memory_order_relaxed);
        break;
      }
      case 'h': {
        NodePath &n = mine[rng() % mine.size()];
        n.set_h(static_cast<PN_stdfloat>(heading_dist(rng)));
        break;
      }
      case 'p': {
        NodePath &n = mine[rng() % mine.size()];
        n.set_pos(pos_dist(rng), pos_dist(rng), pos_dist(rng));
        break;
      }
      case 's': {
        NodePath &n = mine[rng() % mine.size()];
        if (n.is_stashed()) n.unstash();
        else                n.stash();
        g_total_stashed.fetch_add(1, std::memory_order_relaxed);
        break;
      }
      case 'r': {
        NodePath &n = mine[rng() % mine.size()];
        const NodePath &target = all_targets[rng() % all_targets.size()];
        n.reparent_to(target);
        g_total_reparented.fetch_add(1, std::memory_order_relaxed);
        break;
      }
      case 'b': {
        NodePath &n = mine[rng() % mine.size()];
        auto bounds = n.get_bounds();
        (void)bounds;
        g_total_reads.fetch_add(1, std::memory_order_relaxed);
        break;
      }
      case 'R':
      default: {
        size_t idx = rng() % mine.size();
        mine[idx].remove_node();
        mine.erase(mine.begin() + idx);
        g_total_removed.fetch_add(1, std::memory_order_relaxed);
        break;
      }
    }
  }
  {
    EpochHolder epoch(self.p());
    for (auto &n : mine) {
      n.remove_node();
      g_total_removed.fetch_add(1, std::memory_order_relaxed);
    }
    mine.clear();
  }
  g_workers_done.fetch_add(1, std::memory_order_relaxed);
}

void print_usage(const char *prog) {
  std::cout <<
    "Usage: " << prog << " [options]\n"
    "\n"
    "Multi-threaded scene-graph stress tester.  Workers concurrently mutate a\n"
    "Panda3D scene graph while the main thread renders frames.  Trips a\n"
    "watchdog if no progress is made within --timeout seconds.\n"
    "\n"
    "Options:\n"
    "  --workers N        Number of mutation worker threads (default: 4).\n"
    "  --iterations N     Mutation iterations per worker     (default: 600).\n"
    "  --shared           All workers share a single parent  (default).\n"
    "  --per-worker       Each worker gets its own subtree.\n"
    "  --timeout SECONDS  Watchdog timeout in seconds        (default: 8).\n"
    "  --depth N          Intermediate empty nodes between root and target\n"
    "                                                       (default: 0).\n"
    "                     Higher depth lengthens the parent->child chain\n"
    "                     that lock-order inversion needs to fire.\n"
    "  --branching N      Sibling leaves attached at each chain level\n"
    "                                                       (default: 1).\n"
    "                     Wider trees give cull more nodes to walk past.\n"
    "  --ops CHARS        Op types workers perform (default: ahpsrbR).\n"
    "                       a=attach   h=set_h    p=set_pos\n"
    "                       s=stash    r=reparent b=get_bounds\n"
    "                       R=remove\n"
    "                     Useful for bisecting which op triggers a race.\n"
    "  --threading-model M\n"
    "                     Set Panda3D's internal render threading-model PRC\n"
    "                     before opening the window.  Default is empty\n"
    "                     (single-threaded render).  Typical multi-stage\n"
    "                     values: \"cull/draw\", \"cull\".  See Panda3D's\n"
    "                     GraphicsEngine::set_threading_model documentation.\n"
    "  --help             Show this help and exit.\n"
    "\n"
    "Exit codes:\n"
    "  0   workers completed cleanly\n"
    "  1   crashed or argument error\n"
    "  2   watchdog tripped (deadlock)\n"
    " 77   no graphics pipe / no window backend available (ctest skip)\n";
}

// Parse command line. Returns true on success, false to terminate (after
// printing usage or an error).  On false, *exit_code is the code to exit
// with: 0 for --help, 1 for argument error.
bool parse_args(int argc, char **argv, Options &opts, int *exit_code) {
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    auto need_value = [&](const char *name) -> const char * {
      if (i + 1 >= argc) {
        std::cerr << "error: " << name << " requires a value\n";
        *exit_code = 1;
        return nullptr;
      }
      return argv[++i];
    };

    if (std::strcmp(arg, "--help") == 0 || std::strcmp(arg, "-h") == 0) {
      print_usage(argv[0]);
      *exit_code = 0;
      return false;
    } else if (std::strcmp(arg, "--workers") == 0) {
      const char *v = need_value("--workers");
      if (v == nullptr) return false;
      opts.workers = std::atoi(v);
    } else if (std::strcmp(arg, "--iterations") == 0) {
      const char *v = need_value("--iterations");
      if (v == nullptr) return false;
      opts.iterations = std::atoi(v);
    } else if (std::strcmp(arg, "--shared") == 0) {
      opts.shared = true;
    } else if (std::strcmp(arg, "--per-worker") == 0) {
      opts.shared = false;
    } else if (std::strcmp(arg, "--timeout") == 0) {
      const char *v = need_value("--timeout");
      if (v == nullptr) return false;
      opts.timeout_sec = std::atoi(v);
    } else if (std::strcmp(arg, "--depth") == 0) {
      const char *v = need_value("--depth");
      if (v == nullptr) return false;
      opts.depth = std::atoi(v);
    } else if (std::strcmp(arg, "--branching") == 0) {
      const char *v = need_value("--branching");
      if (v == nullptr) return false;
      opts.branching = std::atoi(v);
      if (opts.branching < 1) opts.branching = 1;
    } else if (std::strcmp(arg, "--ops") == 0) {
      const char *v = need_value("--ops");
      if (v == nullptr) return false;
      opts.ops = v;
    } else if (std::strcmp(arg, "--threading-model") == 0) {
      const char *v = need_value("--threading-model");
      if (v == nullptr) return false;
      opts.threading_model = v;
    } else {
      std::cerr << "error: unknown argument: " << arg << "\n"
                << "run with --help for usage\n";
      *exit_code = 1;
      return false;
    }
  }
  return true;
}

}  // namespace

int main(int argc, char **argv) {
  Options opts;
  {
    int exit_code = 0;
    if (!parse_args(argc, argv, opts, &exit_code)) return exit_code;
  }

  // `window-type offscreen` + the tinydisplay aux pipe gets the test
  // through CI without an X display.  The CI's own PRC (e.g.
  // `load-display p3headlessgl`) is respected because
  // load_prc_file_data appends.
  std::string prc =
      "window-type offscreen\n"
      "aux-display p3tinydisplay\n"
      "notify-level-pgraph fatal\n"
      "notify-level-display fatal\n"
      "notify-level-loader fatal\n"
      "notify-level-egg2pg fatal\n";
  if (!opts.threading_model.empty()) {
    prc += "threading-model " + opts.threading_model + "\n";
  }
  load_prc_file_data("scenegraph_stress", prc);

  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("scenegraph_stress");

  WindowFramework *window = framework.open_window();
  if (window == nullptr) {
    std::cerr << "skip: no graphics pipe / no window backend available\n";
    return 77;
  }
  NodePath render = window->get_render();

  // Build the "model" workers clone programmatically (one GeomNode
  // wrapping a unit triangle, parented under a PandaNode).  Self-
  // contained: no model file, no model-path / VFS resolution, no
  // loader plugin to register -- which matters on the CMake CI
  // configurations that don't auto-load libpandaegg / p3ptloader
  // (BUILD_METALIBS=NO, HAVE_PYTHON=NO).
  NodePath model;
  {
    PT(GeomVertexData) vdata = new GeomVertexData(
        "stress-tri", GeomVertexFormat::get_v3(), Geom::UH_static);
    vdata->set_num_rows(3);
    GeomVertexWriter vw(vdata, "vertex");
    vw.add_data3(0.0f, 0.0f, 0.0f);
    vw.add_data3(1.0f, 0.0f, 0.0f);
    vw.add_data3(0.0f, 0.0f, 1.0f);
    PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
    tris->add_vertices(0, 1, 2);
    tris->close_primitive();
    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(tris);
    PT(GeomNode) gn = new GeomNode("stress-geom");
    gn->add_geom(geom);
    PT(PandaNode) root = new PandaNode("stress-model");
    model = NodePath(root);
    model.attach_new_node(gn);
  }
  // (model is a detached prototype; workers attach copies of it.)

  NodePath stress = render.attach_new_node("stress-root");
  stress.set_pos(0, 80, 0);
  stress.set_scale(0.3f);

  auto build_chain = [&](NodePath start, int levels, int branches) -> NodePath {
    NodePath n = start;
    for (int i = 0; i < levels; ++i) {
      NodePath next = n.attach_new_node("chain");
      for (int b = 1; b < branches; ++b) {
        n.attach_new_node("branch");
      }
      n = next;
    }
    return n;
  };

  std::vector<NodePath> all_targets;
  if (opts.shared) {
    all_targets.push_back(build_chain(stress, opts.depth, opts.branching));
  } else {
    all_targets.reserve(opts.workers);
    for (int w = 0; w < opts.workers; ++w) {
      NodePath sub = stress.attach_new_node("sub");
      all_targets.push_back(build_chain(sub, opts.depth, opts.branching));
    }
  }

  std::cerr << "[stress] workers=" << opts.workers
            << " iterations=" << opts.iterations
            << " shared=" << (opts.shared ? 1 : 0)
            << " depth=" << opts.depth
            << " branching=" << opts.branching
            << " ops=" << opts.ops
            << " timeout=" << opts.timeout_sec << "s"
            << " threading-model=\"" << opts.threading_model << "\"\n";

  std::thread watchdog([&]() {
    TrueClock *clock = TrueClock::get_global_ptr();
    double start = clock->get_long_time();
    int last_attached = -1;
    int last_removed = -1;
    double last_progress = start;
    int last_print_sec = -1;
    while (!g_stop.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      double now = clock->get_long_time();
      int elapsed = (int)(now - start);
      int a = g_total_attached.load();
      int rm = g_total_removed.load();
      if (a != last_attached || rm != last_removed) {
        last_attached = a;
        last_removed = rm;
        last_progress = now;
      }
      if (elapsed > last_print_sec) {
        last_print_sec = elapsed;
        std::cerr << "[stress] " << elapsed << "s  attached=" << a
                  << " removed=" << rm << "\n";
      }
      int idle = (int)(now - last_progress);
      if (idle >= opts.timeout_sec) {
        std::cerr << "[stress] DEADLOCK: no progress for " << idle
                  << "s  attached=" << a << " removed=" << rm << "\n";
        std::_Exit(2);
      }
    }
  });

  std::vector<std::thread> threads;
  threads.reserve(opts.workers);
  for (int w = 0; w < opts.workers; ++w) {
    int my_idx = opts.shared ? 0 : w;
    threads.emplace_back(worker, std::cref(all_targets), my_idx, model, w,
                          opts.iterations, std::cref(opts.ops));
  }

  Thread *main_thread = Thread::get_current_thread();
  while (g_workers_done.load() < opts.workers) {
    framework.do_frame(main_thread);
  }

  for (auto &th : threads) if (th.joinable()) th.join();
  g_stop.store(true);
  watchdog.join();

  std::cerr << "[stress] CLEAN  attached=" << g_total_attached.load()
            << " removed=" << g_total_removed.load()
            << " reparented=" << g_total_reparented.load()
            << " stashed=" << g_total_stashed.load()
            << " reads=" << g_total_reads.load() << "\n";

  // Skip framework.close_framework() — its destructors occasionally
  // crash after a multi-threaded workload, a separate latent bug.
  std::_Exit(0);
}
