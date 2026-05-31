/**
 * Single-threaded micro/mini benchmark for Panda3D's scene-graph cycler
 * machinery.  Establishes a baseline against which the upcoming EBR
 * migration will be compared.  Output is a Markdown table on stdout so
 * results can be pasted directly into PR descriptions.
 *
 * No window is opened — every workload exercises the cycler directly
 * through high-level (NodePath) and low-level (raw CDReader /
 * CDStageWriter) paths.
 *
 *   ./cycler_bench [--scale N] [--workload NAME]
 */
#include "pandaFramework.h"
#include "load_prc_file.h"
#include "pandaNode.h"
#include "nodePath.h"
#include "transformState.h"
#include "renderState.h"
#include "colorAttrib.h"
#include "pipeline.h"
#include "thread.h"
#include "genericThread.h"
#include "threadPriority.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace {

using SteadyClock = std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;

struct Row {
  std::string workload;
  int depth = 0;
  int branching = 0;
  uint64_t iters = 0;
  double ns_per_op = 0;
};

std::vector<Row> g_results;
volatile uintptr_t g_sink = 0;

// Background-reader coordination.  Readers register with EBR at a chosen
// pipeline stage and continuously read a dedicated tree while the main
// thread runs the write workloads.  This models "multiple threads, but
// all scene-graph mutation serialized on one thread":
//   * readers at the writer's stage (stage 0) -> the per-stage in-place
//     fast path is disabled, every write COWs;
//   * readers at a downstream stage (1, 2, ...) -> the writer is the sole
//     occupant of stage 0, so the fast path still engages.
std::atomic<int> g_readers_ready{0};
std::atomic<bool> g_readers_stop{false};
bool g_leak_check = false;

// Hammer one node's transform between two fixed states under forced COW,
// draining EBR periodically, and watch a transform's refcount: it stays flat
// if reclamation is balanced, grows without bound if a reference leaks.
void run_leak_check() {
  PT(PandaNode) n = new PandaNode("leak");
  NodePath np(n);
  CPT(TransformState) t1 = TransformState::make_pos(LVecBase3(1, 0, 0));
  CPT(TransformState) t2 = TransformState::make_pos(LVecBase3(2, 0, 0));
  Thread *self = Thread::get_current_thread();
  int base = t1->get_ref_count();
  printf("leak-check: t1 ref_count baseline=%d (held by local CPT + cache)\n", base);
  const int chunk = 200000;
  for (int c = 0; c < 8; ++c) {
    for (int i = 0; i < chunk; ++i) {
      np.set_transform((i & 1) ? t1 : t2);
    }
    // Drain retired generations so their references are released.
    for (int d = 0; d < 4; ++d) self->yield_quiescent();
    printf("leak-check: after %8d writes, t1 ref_count=%d\n",
           (c + 1) * chunk, t1->get_ref_count());
  }
  np.set_transform(TransformState::make_identity());
  for (int d = 0; d < 8; ++d) self->yield_quiescent();
  printf("leak-check: after detach+drain, t1 ref_count=%d (expect ~baseline)\n",
         t1->get_ref_count());
}

// Run `op` for `iters` iterations with a short warmup; return ns/op.
template <class Op>
double bench(uint64_t iters, Op &&op) {
  uint64_t warm = std::min<uint64_t>(iters / 20, 5000);
  for (uint64_t i = 0; i < warm; ++i) op();
  auto t0 = SteadyClock::now();
  for (uint64_t i = 0; i < iters; ++i) op();
  auto t1 = SteadyClock::now();
  return double(duration_cast<nanoseconds>(t1 - t0).count()) / double(iters);
}

// Build a (depth, branching) tree under `root`; collect every node into
// `out`.  Each level has `branching` children; the chain continues down
// the first child.  Returns the deepest leaf.
NodePath build_tree(NodePath root, int depth, int branching,
                    std::vector<NodePath> &out) {
  out.push_back(root);
  NodePath n = root;
  for (int i = 0; i < depth; ++i) {
    NodePath next = n.attach_new_node("level");
    out.push_back(next);
    for (int b = 1; b < branching; ++b) {
      NodePath leaf = n.attach_new_node("branch");
      out.push_back(leaf);
    }
    n = next;
  }
  return n;
}

void run_size(int depth, int branching, uint64_t scale) {
  PT(PandaNode) root_node = new PandaNode("root");
  NodePath root(root_node);
  std::vector<NodePath> nodes;
  NodePath leaf = build_tree(root, depth, branching, nodes);
  const uint64_t n = nodes.size();

  // -------- read-side: high-level walk --------
  {
    uint64_t reps = std::max<uint64_t>(1, 200000 * scale / n);
    auto t0 = SteadyClock::now();
    for (uint64_t r = 0; r < reps; ++r) {
      for (const NodePath &np : nodes) {
        const TransformState *t = np.node()->get_transform();
        const RenderState *s = np.node()->get_state();
        // prevent the compiler from eliding
        g_sink = (uintptr_t)t ^ (uintptr_t)s;
      }
    }
    auto t1 = SteadyClock::now();
    uint64_t ops = reps * n * 2;
    g_results.push_back({
        "read_walk_get_transform+state", depth, branching, ops,
        double(duration_cast<nanoseconds>(t1 - t0).count()) / double(ops)});
  }

  // -------- write-side: set_pos hot loop --------
  {
    uint64_t iters = 100000 * scale;
    int counter = 0;
    double ns = bench(iters, [&]{
      leaf.set_pos((float)(counter & 0xff), 0, 0);
      ++counter;
    });
    g_results.push_back({"set_pos", depth, branching, iters, ns});
  }

  // -------- write-side: set_transform hot loop --------
  {
    uint64_t iters = 100000 * scale;
    CPT(TransformState) xform = TransformState::make_pos(LVecBase3(1, 2, 3));
    double ns = bench(iters, [&]{
      leaf.set_transform(xform);
    });
    g_results.push_back({"set_transform", depth, branching, iters, ns});
  }

  // -------- write-side: set_attrib hot loop --------
  {
    uint64_t iters = 100000 * scale;
    CPT(RenderAttrib) ca = ColorAttrib::make_flat(LColor(1, 0, 0, 1));
    double ns = bench(iters, [&]{
      leaf.set_attrib(ca);
    });
    g_results.push_back({"set_attrib_color", depth, branching, iters, ns});
  }

  // -------- mixed: animate then walk --------
  {
    uint64_t reps = std::max<uint64_t>(1, 50000 * scale / n);
    int counter = 0;
    auto t0 = SteadyClock::now();
    for (uint64_t r = 0; r < reps; ++r) {
      // animate every node
      for (NodePath &np : nodes) {
        np.set_pos((float)(counter & 0x7f), 0, 0);
        ++counter;
      }
      // then walk reads
      for (const NodePath &np : nodes) {
        const TransformState *t = np.node()->get_transform();
        g_sink = (uintptr_t)t;
      }
    }
    auto t1 = SteadyClock::now();
    uint64_t ops = reps * n * 2;  // one write + one read per node
    g_results.push_back({
        "animate_then_walk", depth, branching, ops,
        double(duration_cast<nanoseconds>(t1 - t0).count()) / double(ops)});
  }

  // -------- structural: reparent storm --------
  {
    // pick two parents and ping-pong one child between them
    if (nodes.size() >= 3) {
      uint64_t iters = 5000 * scale;
      NodePath p1 = nodes[1];
      NodePath p2 = nodes[nodes.size() - 1];
      NodePath mover = root.attach_new_node("mover");
      int counter = 0;
      double ns = bench(iters, [&]{
        mover.reparent_to((++counter & 1) ? p1 : p2);
      });
      g_results.push_back({"reparent_storm", depth, branching, iters, ns});
      mover.remove_node();
    }
  }

  // -------- get_bounds walk (touches cache) --------
  {
    uint64_t iters = 50000 * scale;
    double ns = bench(iters, [&]{
      CPT(BoundingVolume) b = root.get_bounds();
      g_sink = (uintptr_t)b.p();
    });
    g_results.push_back({"get_bounds_root", depth, branching, iters, ns});
  }
}

// Platform-primitive microbenchmarks — these isolate the cost of the
// individual building blocks the cycler/EBR stack relies on, so a
// platform-specific regression (e.g. MSVC atomics vs glibc, Windows
// CRITICAL_SECTION vs pthread_mutex) shows up directly instead of being
// hidden inside higher-level workloads.
void run_micro(Thread *self, uint64_t scale) {
  // 1. std::atomic<uintptr_t> acquire load — pure atomic-load cost.
  {
    std::atomic<uintptr_t> a{0xdeadbeefULL};
    uint64_t iters = 5000000 * scale;
    double ns = bench(iters, [&]{
      g_sink = a.load(std::memory_order_acquire);
    });
    g_results.push_back({"atomic_acq_load", 0, 0, iters, ns});
  }
  // 2. std::atomic<uintptr_t> acq_rel CAS loop — pure CAS cost.
  {
    std::atomic<uintptr_t> a{0};
    uint64_t iters = 2000000 * scale;
    uintptr_t expected = 0;
    double ns = bench(iters, [&]{
      uintptr_t e = expected;
      a.compare_exchange_strong(e, e + 1, std::memory_order_acq_rel);
      expected = e + 1;
    });
    g_results.push_back({"atomic_cas_loop", 0, 0, iters, ns});
  }
  // 3. Thread::get_current_thread — TLS-lookup cost (Windows TLS slot vs
  // Linux __thread).  The cycler reads this on every write_stage.
  {
    uint64_t iters = 5000000 * scale;
    double ns = bench(iters, [&]{
      Thread *t = Thread::get_current_thread();
      g_sink = (uintptr_t)t;
    });
    g_results.push_back({"tls_get_current_thread", 0, 0, iters, ns});
  }
  // 4. Mutex acquire+release — Windows CRITICAL_SECTION vs Linux
  // pthread_mutex.  The cycler holds a per-cycler mutex briefly on every
  // write_stage.
  {
    Mutex m;
    uint64_t iters = 2000000 * scale;
    double ns = bench(iters, [&]{
      MutexHolder h(m);
      g_sink ^= (uintptr_t)&m;
    });
    g_results.push_back({"mutex_acq_rel", 0, 0, iters, ns});
  }
  // 5. epoch_enter/leave — EBR's per-critical-section atomic bookkeeping
  // (no-op on the master baseline so the row is the pure overhead delta).
  {
    uint64_t iters = 2000000 * scale;
#ifndef CYCLER_BENCH_NO_EBR
    double ns = bench(iters, [&]{
      self->epoch_enter();
      self->epoch_leave();
    });
#else
    (void)self;
    double ns = bench(iters, [&]{
      g_sink ^= 1;
    });
#endif
    g_results.push_back({"epoch_enter_leave", 0, 0, iters, ns});
  }
}

void print_table() {
  printf("| workload                       | depth | branch |     iters | ns/op   |\n");
  printf("|--------------------------------|------:|-------:|----------:|--------:|\n");
  for (const Row &r : g_results) {
    printf("| %-30s | %5d | %6d | %9llu | %7.1f |\n",
           r.workload.c_str(), r.depth, r.branching,
           (unsigned long long)r.iters, r.ns_per_op);
  }
}

}  // namespace

int main(int argc, char **argv) {
  uint64_t scale = 1;
  std::string only_workload;
  int reader_threads = 0;
  int reader_stage = 0;
  bool wrap_frame = false;
  bool always_cow = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--scale") == 0 && i + 1 < argc) {
      scale = std::strtoull(argv[++i], nullptr, 10);
    } else if (std::strcmp(argv[i], "--workload") == 0 && i + 1 < argc) {
      only_workload = argv[++i];
    } else if (std::strcmp(argv[i], "--reader-threads") == 0 && i + 1 < argc) {
      reader_threads = (int)std::strtol(argv[++i], nullptr, 10);
    } else if (std::strcmp(argv[i], "--reader-stage") == 0 && i + 1 < argc) {
      reader_stage = (int)std::strtol(argv[++i], nullptr, 10);
    } else if (std::strcmp(argv[i], "--wrap-frame") == 0) {
      wrap_frame = true;
    } else if (std::strcmp(argv[i], "--always-cow") == 0) {
      always_cow = true;
    } else if (std::strcmp(argv[i], "--leak-check") == 0) {
      g_leak_check = true;
    } else if (std::strcmp(argv[i], "--help") == 0) {
      printf("Usage: %s [--scale N] [--workload NAME] "
             "[--reader-threads K] [--reader-stage S]\n", argv[0]);
      printf("  --scale N           Iteration count multiplier (default 1).\n");
      printf("  --workload S        Only run workloads whose name contains S.\n");
      printf("  --reader-threads K  Spawn K background reader threads that\n");
      printf("                      continuously read a dedicated tree while\n");
      printf("                      the main thread does all writes (default 0).\n");
      printf("  --reader-stage S    Pipeline stage for the reader threads\n");
      printf("                      (default 0 = writer's stage, disables the\n");
      printf("                      in-place fast path).  S>0 models a threaded\n");
      printf("                      render pipeline; the pipeline is configured\n");
      printf("                      with S+1 stages.\n");
      printf("  --wrap-frame        Wrap each workload run in one epoch\n");
      printf("                      critical section, emulating the framework\n");
      printf("                      per-frame wrapper so inner CDReaders are\n");
      printf("                      TLS-only (no per-read epoch atomic).\n");
      return 0;
    } else {
      fprintf(stderr, "unknown arg: %s\n", argv[i]);
      return 1;
    }
  }

  load_prc_file_data("cycler_bench",
      "notify-level fatal\n"
      "default-directnotify-level fatal\n"
      "notify-level-pgraph fatal\n");
  if (always_cow) {
    load_prc_file_data("cycler_bench", "pipeline-always-cow true\n");
    fprintf(stderr, "[cycler_bench] pipeline-always-cow true (fast path disabled)\n");
  }

  // Force panda init by constructing a framework but never opening a
  // window.  This ensures all type-handles are registered.
  PandaFramework framework;
  framework.open_framework(argc, argv);

  // A reader thread parked at stage S needs the pipeline to actually have
  // S+1 stages so reads at that stage are in range.  Configure it before
  // building any tree so every cycler is created multi-stage.
  if (reader_stage + 1 > Pipeline::get_render_pipeline()->get_num_stages()) {
    Pipeline::get_render_pipeline()->set_num_stages(reader_stage + 1);
  }

  if (g_leak_check) {
    run_leak_check();
    fflush(stdout);
    std::_Exit(0);
  }

  // Dedicated read-only tree for the background readers, plus the reader
  // threads themselves.  Built before the workloads so the steady-state
  // stage occupancy is established by the time we measure.
  PT(PandaNode) reader_root_node;
  std::vector<NodePath> reader_nodes;
  pvector<PT(GenericThread)> readers;
  if (reader_threads > 0) {
    reader_root_node = new PandaNode("reader_root");
    NodePath reader_root(reader_root_node);
    build_tree(reader_root, 10, 4, reader_nodes);

    const std::vector<NodePath> *rn = &reader_nodes;
    auto reader_fn = [rn, reader_stage]() {
      Thread *self = Thread::get_current_thread();
      if (reader_stage != 0) {
        self->set_pipeline_stage(reader_stage);
      }
      g_readers_ready.fetch_add(1, std::memory_order_release);
      uintptr_t local = 0;
      while (!g_readers_stop.load(std::memory_order_relaxed)) {
        for (const NodePath &np : *rn) {
          const TransformState *t = np.node()->get_transform(self);
          const RenderState *s = np.node()->get_state(self);
          local ^= (uintptr_t)t ^ (uintptr_t)s;
        }
      }
      g_sink ^= local;
    };

    for (int i = 0; i < reader_threads; ++i) {
      PT(GenericThread) rt =
        new GenericThread("reader", "reader", reader_fn);
      rt->start(TP_normal, true);
      readers.push_back(rt);
    }
    // Wait until every reader has registered at its stage and started
    // reading, so the fast-path gate has settled before we measure.
    while (g_readers_ready.load(std::memory_order_acquire) < reader_threads) {
      Thread::sleep(0.001);
    }
    fprintf(stderr, "[cycler_bench] %d reader thread(s) at stage %d "
            "(pipeline stages=%d)\n", reader_threads, reader_stage,
            Pipeline::get_render_pipeline()->get_num_stages());
  }

  Thread *main_thread = Thread::get_current_thread();
  // Run platform-primitive micros once up front (depth/branch are 0 for them).
  run_micro(main_thread, scale);
  for (int depth : {5, 10, 15}) {
    for (int branching : {1, 4}) {
      if (wrap_frame) {
        main_thread->epoch_enter();
      }
      run_size(depth, branching, scale);
      if (wrap_frame) {
        main_thread->epoch_leave();
      }
    }
  }

  // Stop the readers before tearing anything down.
  if (reader_threads > 0) {
    g_readers_stop.store(true, std::memory_order_relaxed);
    for (PT(GenericThread) &rt : readers) {
      rt->join();
    }
  }

  // Filter if requested.
  if (!only_workload.empty()) {
    std::vector<Row> filtered;
    for (const Row &r : g_results) {
      if (r.workload.find(only_workload) != std::string::npos) {
        filtered.push_back(r);
      }
    }
    g_results.swap(filtered);
  }

  print_table();
  fflush(stdout);

  // Skip framework.close_framework() — its destructors occasionally
  // touch threading internals after the benchmark workload, and we just
  // want clean numbers.
  std::_Exit(0);
}
