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
#include "geom.h"
#include "geomNode.h"
#include "geomTriangles.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "material.h"
#include "pointLight.h"
#include "spotlight.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "pipeline.h"
#include "thread.h"
#include "genericThread.h"
#include "threadPriority.h"
#include "pmutex.h"
#include "mutexHolder.h"
#include "trueClock.h"
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace {

// Elapsed time in seconds from Panda's high-resolution monotonic clock.
inline double now_s() { return TrueClock::get_global_ptr()->get_long_time(); }

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
#ifdef CYCLER_BENCH_NO_EBR
void run_leak_check() {
  printf("leak-check: unavailable on the no-EBR baseline build\n");
}
#else
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
#endif  // CYCLER_BENCH_NO_EBR

// Run `op` for `iters` iterations with a short warmup; return ns/op.
template <class Op>
double bench(uint64_t iters, Op &&op) {
  uint64_t warm = std::min<uint64_t>(iters / 20, 5000);
  for (uint64_t i = 0; i < warm; ++i) op();
  auto t0 = now_s();
  for (uint64_t i = 0; i < iters; ++i) op();
  auto t1 = now_s();
  return (t1 - t0) * 1e9 / double(iters);
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
    auto t0 = now_s();
    for (uint64_t r = 0; r < reps; ++r) {
      for (const NodePath &np : nodes) {
        const TransformState *t = np.node()->get_transform();
        const RenderState *s = np.node()->get_state();
        // prevent the compiler from eliding
        g_sink = (uintptr_t)t ^ (uintptr_t)s;
      }
    }
    auto t1 = now_s();
    uint64_t ops = reps * n * 2;
    g_results.push_back({
        "read_walk_get_transform+state", depth, branching, ops,
        (t1 - t0) * 1e9 / double(ops)});
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
    auto t0 = now_s();
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
    auto t1 = now_s();
    uint64_t ops = reps * n * 2;  // one write + one read per node
    g_results.push_back({
        "animate_then_walk", depth, branching, ops,
        (t1 - t0) * 1e9 / double(ops)});
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

// Main-thread setup-operations storm.  Models the dominant real-world shape of
// scene-graph mutation: an application (or a script) building its scene from
// scratch on the main thread, entirely outside any frame/task epoch.  Each
// "op" is one node's full setup -- create + parent + initial transform +
// initial state attrib -- so it exercises four cyclers (the new node's, its
// parent's child list, plus the transform and state cyclers) per op.  A
// bounded fan-out keeps every parent's child-list write O(1) so the per-op cost
// stays flat (no quadratic growing-list artifact, even under --always-cow).
//
// This is exactly the unframed online-main-thread path: with `tag == unframed`
// each op is its own outermost critical section (publishing a slot, with
// reclamation taken in throttled batches rather than once per write); with
// `tag == framed` the whole storm is wrapped in one epoch so the ops merely
// nest.  Comparing the two isolates the cost of being unframed -- and, under
// --always-cow where every op retires a CData, confirms the reclaim throttle
// keeps the unframed case off the per-write advance/reclaim cliff.
void run_setup_storm(uint64_t scale, const char *tag) {
  const uint64_t count = 20000 * scale;
  CPT(TransformState) xform = TransformState::make_pos(LVecBase3(1, 2, 3));
  CPT(RenderAttrib) ca = ColorAttrib::make_flat(LColor(0.5f, 0.5f, 0.5f, 1));

  std::vector<NodePath> built;
  built.reserve(count);
  PT(PandaNode) root_node = new PandaNode("setup_root");
  NodePath root(root_node);

  // Build: create + parent + set transform + set state.  Attach breadth-first
  // into a balanced 8-ary tree (a FIFO of not-yet-full parents) so both the
  // depth (~log8 count) and every parent's fan-out (<=8) stay bounded.  That
  // keeps each op O(1): no deep-chain bounds-stale walk and no growing
  // child-list COW to swamp the measurement (the cost we want is the per-op
  // cycler/epoch work, not tree shape).
  std::vector<NodePath> parents;
  parents.reserve(count + 1);
  parents.push_back(root);
  size_t head = 0;
  int kids = 0;
  auto t0 = now_s();
  for (uint64_t i = 0; i < count; ++i) {
    NodePath n = parents[head].attach_new_node("n");
    n.set_transform(xform);
    n.set_attrib(ca);
    built.push_back(n);
    parents.push_back(n);
    if (++kids >= 8) { ++head; kids = 0; }
  }
  auto t1 = now_s();
  g_results.push_back({std::string("setup_build_") + tag, 0, 0, count,
      (t1 - t0) * 1e9 / double(count)});

  // Teardown is itself a main-thread structural-write storm; measure it too.
  // Remove deepest-first so each removed node is a leaf.
  auto t2 = now_s();
  for (auto it = built.rbegin(); it != built.rend(); ++it) {
    it->remove_node();
  }
  auto t3 = now_s();
  g_results.push_back({std::string("setup_teardown_") + tag, 0, 0, count,
      (t3 - t2) * 1e9 / double(count)});
  root.remove_node();
}

// ---- Macro workloads: realistic main-thread scene construction -------------
// These model what an app does at load/setup time -- build many model subtrees,
// generate content procedurally, wire up lighting -- all on the main thread,
// outside any frame.  They are the construction (cycler-write) half of "load a
// level"; disk I/O and GPU upload are deliberately excluded so the number
// reflects the scene-graph cost that EBR actually changes.  All run unframed,
// the way real setup code does (no surrounding task/frame epoch).

PT(Geom) make_unit_geom() {
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
  PT(Geom) g = new Geom(vdata);
  g->add_primitive(tris);
  return g;
}

// One "model": a small transformed/colored hierarchy over a few GeomNode leaves
// that share `geom` (loaders instance geometry rather than rebuild it per
// model).  ~7 nodes -- the shape of a typical prop.
NodePath make_model(NodePath parent, Geom *geom, Material *mat, int i) {
  NodePath root = parent.attach_new_node("model");
  root.set_pos((float)(i % 97), (float)(i % 53), (float)(i % 71));
  root.set_hpr((float)(i % 360), 0.0f, 0.0f);
  root.set_material(mat);
  for (int part = 0; part < 3; ++part) {
    NodePath grp = root.attach_new_node("part");
    grp.set_pos((float)part, 0.0f, 0.0f);
    grp.set_color((part & 1) ? 1.0f : 0.3f, 0.5f, 0.7f, 1.0f);
    PT(GeomNode) gn = new GeomNode("mesh");
    gn->add_geom(geom);
    NodePath mesh = grp.attach_new_node(gn);
    mesh.set_scale(1.0f + 0.01f * (float)part);
  }
  return root;
}

// "Load" many models into a scene (instanced geom, fresh nodes/transforms/state
// per model), then unload them.
void run_model_load(uint64_t scale) {
  const uint64_t models = 1500 * scale;
  PT(Geom) geom = make_unit_geom();
  PT(Material) mat = new Material("m");
  PT(PandaNode) scene_node = new PandaNode("scene");
  NodePath scene(scene_node);

  std::vector<NodePath> parents;        // balanced fan-out, no huge child list
  parents.push_back(scene);
  size_t head = 0;
  int kids = 0;
  std::vector<NodePath> built;
  built.reserve(models);

  auto t0 = now_s();
  for (uint64_t i = 0; i < models; ++i) {
    NodePath m = make_model(parents[head], geom, mat, (int)i);
    built.push_back(m);
    parents.push_back(m);
    if (++kids >= 16) { ++head; kids = 0; }
  }
  auto t1 = now_s();
  g_results.push_back({"model_load (per model)", 0, 0, models,
      (t1 - t0) * 1e9 / double(models)});

  auto t2 = now_s();
  for (auto it = built.rbegin(); it != built.rend(); ++it) it->remove_node();
  auto t3 = now_s();
  g_results.push_back({"model_unload (per model)", 0, 0, models,
      (t3 - t2) * 1e9 / double(models)});
}

// Procedural scatter: many GeomNodes bucketed into spatial chunks, each given a
// full transform (pos/hpr/scale) and a color -- a terrain/foliage/particle
// generate pass.
void run_procedural_gen(uint64_t scale) {
  const uint64_t objs = 20000 * scale;
  PT(Geom) geom = make_unit_geom();
  PT(PandaNode) root_node = new PandaNode("proc");
  NodePath root(root_node);
  const int CHUNKS = 256;
  std::vector<NodePath> chunks;
  chunks.reserve(CHUNKS);
  for (int c = 0; c < CHUNKS; ++c) {
    NodePath ch = root.attach_new_node("chunk");
    ch.set_pos((float)((c % 16) * 100), (float)((c / 16) * 100), 0.0f);
    chunks.push_back(ch);
  }
  std::vector<NodePath> built;
  built.reserve(objs);

  auto t0 = now_s();
  for (uint64_t i = 0; i < objs; ++i) {
    PT(GeomNode) gn = new GeomNode("obj");
    gn->add_geom(geom);
    NodePath o = chunks[i % CHUNKS].attach_new_node(gn);
    o.set_pos((float)(i % 97), (float)((i * 7) % 89), (float)((i * 13) % 53));
    o.set_hpr((float)(i % 360), (float)((i * 3) % 360), 0.0f);
    o.set_scale(1.0f + 0.001f * (float)(i % 100));
    o.set_color((i & 1) ? 1.0f : 0.2f, 0.6f, 0.4f, 1.0f);
    built.push_back(o);
  }
  auto t1 = now_s();
  g_results.push_back({"procedural_gen (per obj)", 0, 0, objs,
      (t1 - t0) * 1e9 / double(objs)});

  auto t2 = now_s();
  for (auto it = built.rbegin(); it != built.rend(); ++it) it->remove_node();
  auto t3 = now_s();
  g_results.push_back({"procedural_clear (per obj)", 0, 0, objs,
      (t3 - t2) * 1e9 / double(objs)});
}

// Lighting setup: create a mix of lights, then assign three to every object
// (forward per-object light lists) -- each set_light composes a LightAttrib
// into that object's RenderState.
void run_lighting_setup(uint64_t scale) {
  const uint64_t targets = 8000 * scale;
  const int NLIGHTS = 16;
  PT(PandaNode) root_node = new PandaNode("lit");
  NodePath root(root_node);

  std::vector<NodePath> lights;
  lights.reserve(NLIGHTS);
  auto tc0 = now_s();
  for (int k = 0; k < NLIGHTS; ++k) {
    PT(PandaNode) ln;
    switch (k % 4) {
      case 0:  ln = new PointLight("pl"); break;
      case 1:  ln = new Spotlight("sl"); break;
      case 2:  ln = new DirectionalLight("dl"); break;
      default: ln = new AmbientLight("al"); break;
    }
    NodePath lp = root.attach_new_node(ln);
    lp.set_pos((float)k, (float)k, 10.0f);
    lights.push_back(lp);
  }
  auto tc1 = now_s();
  g_results.push_back({"light_create (per light)", 0, 0, (uint64_t)NLIGHTS,
      (tc1 - tc0) * 1e9 / double(NLIGHTS)});

  // Target objects, bucketed so their own construction isn't quadratic.
  const int BUCKETS = 128;
  std::vector<NodePath> buckets;
  buckets.reserve(BUCKETS);
  for (int b = 0; b < BUCKETS; ++b) buckets.push_back(root.attach_new_node("b"));
  std::vector<NodePath> objs;
  objs.reserve(targets);
  for (uint64_t i = 0; i < targets; ++i) {
    objs.push_back(buckets[i % BUCKETS].attach_new_node("o"));
  }

  auto t0 = now_s();
  for (uint64_t i = 0; i < targets; ++i) {
    objs[i].set_light(lights[i % NLIGHTS]);
    objs[i].set_light(lights[(i + 1) % NLIGHTS]);
    objs[i].set_light(lights[(i + 2) % NLIGHTS]);
  }
  auto t1 = now_s();
  const uint64_t apps = targets * 3;
  g_results.push_back({"light_apply (per set_light)", 0, 0, apps,
      (t1 - t0) * 1e9 / double(apps)});
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

  // With --workload, run only the group(s) that can produce a matching result,
  // so each workload measures against a clean retire queue (the shared EBR queue
  // otherwise lets later workloads absorb earlier ones' reclaim batches).
  auto should_run = [&](std::initializer_list<const char *> names) {
    if (only_workload.empty()) return true;
    for (const char *n : names) {
      if (std::string(n).find(only_workload) != std::string::npos) return true;
    }
    return false;
  };

  // Run platform-primitive micros once up front (depth/branch are 0 for them).
  if (should_run({"atomic_acq_load", "atomic_cas_loop", "tls_get_current_thread",
                  "mutex_acq_rel", "epoch_enter_leave"})) {
    run_micro(main_thread, scale);
  }

  // Main-thread setup storm.  `unframed` is the natural app/scripting path
  // (each op its own outermost section, throttled reclaim).  On an EBR build we
  // also run a `framed` variant (the whole storm wrapped in one epoch so the
  // ops nest) so the cost of being unframed is visible side by side; the
  // baseline build has no epochs, so only `unframed` is meaningful there.
  if (should_run({"setup_build", "setup_teardown"})) {
    run_setup_storm(scale, "unframed");
#ifndef CYCLER_BENCH_NO_EBR
    main_thread->epoch_enter();
    run_setup_storm(scale, "framed");
    main_thread->epoch_leave();
#endif
  }

  // Realistic macro construction workloads (unframed, as setup code runs).
  if (should_run({"model_load", "model_unload"})) {
    run_model_load(scale);
  }
  if (should_run({"procedural_gen", "procedural_clear"})) {
    run_procedural_gen(scale);
  }
  if (should_run({"light_create", "light_apply"})) {
    run_lighting_setup(scale);
  }

  if (should_run({"read_walk", "set_pos", "set_transform", "set_attrib",
                  "reparent_storm", "get_bounds"})) {
    for (int depth : {5, 10, 15}) {
      for (int branching : {1, 4}) {
#ifndef CYCLER_BENCH_NO_EBR
        if (wrap_frame) {
          main_thread->epoch_enter();
        }
#endif
        run_size(depth, branching, scale);
#ifndef CYCLER_BENCH_NO_EBR
        if (wrap_frame) {
          main_thread->epoch_leave();
        }
#endif
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
