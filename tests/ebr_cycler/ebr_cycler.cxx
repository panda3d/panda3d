/**
 * Standalone correctness test for the EBR-backed PipelineCycler.
 *
 * Idea: a tiny CycleData subclass with two fields that must satisfy an
 * invariant (`_b == _a * _a`).  One thread writes both fields in
 * sequence inside a CycleDataStageWriter (always-COW); another thread
 * reads both inside a CycleDataReader and asserts the invariant
 * holds.  Pre-EBR the writer's in-place mutation could let a reader
 * observe `_a` and `_b` out of sync; post-EBR the reader's pointer
 * resolves to an immutable snapshot, so the invariant must hold over
 * millions of iterations.
 *
 * Exit codes: 0 = clean, 1 = invariant violated.
 */

#include "pandaFramework.h"
#include "load_prc_file.h"
#include "cycleData.h"
#include "pipelineCycler.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageWriter.h"
#include "nodeReferenceCount.h"
#include "thread.h"
#include "epochManager.h"
#include "epochHolder.h"
#include "pandaNode.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

class TestCData : public CycleData {
public:
  TestCData() = default;
  TestCData(const TestCData &copy) : CycleData(copy), _a(copy._a), _b(copy._b) {}
  CycleData *make_copy() const override { return new TestCData(*this); }
  TypeHandle get_parent_type() const override { return TypedReferenceCount::get_class_type(); }

  uint64_t _a = 0;
  uint64_t _b = 0;
};

// Waves of short-lived UNBOUND std::threads (no bind_thread -> all share the
// ExternalThread) reading concurrently with a writer and a reclaimer.  Hammers
// the thread_local participant register/unregister path against reclaim scans;
// a tear or UAF would mean the per-OS-thread epoch state is wrong.
static int run_unbound_churn(int seconds) {
  PipelineCycler<TestCData> cycler;
  std::atomic<bool> stop{false};
  std::atomic<uint64_t> read_iters{0}, write_iters{0}, tears{0}, threads_spawned{0};

  std::thread writer([&] {
    PT(Thread) th = Thread::bind_thread("writer", "writer");
    uint64_t v = 1;
    while (!stop.load(std::memory_order_relaxed)) {
      {
        EpochHolder epoch(th.p());
        CycleDataStageWriter<TestCData> cdw(cycler, 0, th.p());
        cdw->_a = v; cdw->_b = v * v;
      }
      ++v;
      write_iters.fetch_add(1, std::memory_order_relaxed);
    }
  });

  // Active reclaim driver: a bound thread that keeps advancing the epoch and
  // reclaiming, so retired snapshots are actually freed while readers run.
  std::thread reclaimer([&] {
    PT(Thread) th = Thread::bind_thread("reclaimer", "reclaimer");
    while (!stop.load(std::memory_order_relaxed)) {
      EpochManager::try_advance_epoch();
      EpochManager::try_reclaim();
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  });

  auto reader_burst = [&] {
    // UNBOUND: no bind_thread.  get_current_thread() yields the shared
    // ExternalThread; the fix keeps the epoch participant per-OS-thread.
    for (int i = 0; i < 2000 && !stop.load(std::memory_order_relaxed); ++i) {
      CycleDataReader<TestCData> cdr(cycler, Thread::get_current_thread());
      uint64_t a = cdr->_a, b = cdr->_b;
      if (a != 0 && b != a * a) tears.fetch_add(1, std::memory_order_relaxed);
      read_iters.fetch_add(1, std::memory_order_relaxed);
    }
  };

  auto t0 = std::chrono::steady_clock::now();
  while (std::chrono::duration_cast<std::chrono::seconds>(
           std::chrono::steady_clock::now() - t0).count() < seconds) {
    std::vector<std::thread> wave;
    for (int k = 0; k < 8; ++k) wave.emplace_back(reader_burst);
    threads_spawned.fetch_add(8, std::memory_order_relaxed);
    for (auto &t : wave) t.join();   // each thread exits -> participant dtor
  }
  stop.store(true);
  writer.join();
  reclaimer.join();

  std::fprintf(stderr,
    "[ebr_churn] unbound_threads=%llu reads=%llu writes=%llu tears=%llu\n",
    (unsigned long long)threads_spawned.load(),
    (unsigned long long)read_iters.load(),
    (unsigned long long)write_iters.load(),
    (unsigned long long)tears.load());
  if (tears.load() != 0) { std::fprintf(stderr, "FAIL\n"); return 1; }
  std::fprintf(stderr, "PASS\n");
  return 0;
}

// Forces the COW / deferred-publish path and checks that a thread reads its
// own in-flight writes through every path -- the read-your-writes invariant
// the deferred-publish model must preserve.  Reproduces the reported loadModel
// crash/NaN class on any platform.  Run under ASan to also catch the _pending lifetime check.
static int run_read_your_writes() {
  load_prc_file_data("ryw", "pipeline-always-cow true\n");
  Thread *th = Thread::get_current_thread();
  int failures = 0;

  // 1. Lock-free reader on the writing thread must see the in-flight write.
  {
    PipelineCycler<TestCData> cycler;
    EpochHolder epoch(th);
    {
      CycleDataWriter<TestCData> cdw(cycler, th);
      cdw->_a = 42; cdw->_b = 42 * 42;
      CycleDataReader<TestCData> cdr(cycler, th);
      if (cdr->_a != 42 || cdr->_b != 42 * 42) {
        std::fprintf(stderr, "FAIL read-your-writes (reader): a=%llu b=%llu\n",
                     (unsigned long long)cdr->_a, (unsigned long long)cdr->_b);
        ++failures;
      }
    }
    CycleDataReader<TestCData> cdr(cycler, th);
    if (cdr->_a != 42) {
      std::fprintf(stderr, "FAIL post-publish read: a=%llu\n",
                   (unsigned long long)cdr->_a);
      ++failures;
    }
  }

  // 2. Copy-constructing the cycler while a write is outstanding must
  //    capture the in-flight data, not the stale published pointer.
  {
    PipelineCycler<TestCData> cycler;
    EpochHolder epoch(th);
    CycleDataWriter<TestCData> cdw(cycler, th);
    cdw->_a = 99; cdw->_b = 99 * 99;
    PipelineCycler<TestCData> dup(cycler);
    CycleDataReader<TestCData> cdr(dup, th);
    if (cdr->_a != 99 || cdr->_b != 99 * 99) {
      std::fprintf(stderr, "FAIL read-your-writes (copy ctor): a=%llu b=%llu\n",
                   (unsigned long long)cdr->_a, (unsigned long long)cdr->_b);
      ++failures;
    }
  }

  // 3. _pending is node_ref'd at creation: a reader that received the in-flight
  //    copy and ref()/unref_delete()s it must not free the writer's live copy.
  {
    PipelineCycler<TestCData> cycler;
    EpochHolder epoch(th);
    CycleDataWriter<TestCData> cdw(cycler, th);
    cdw->_a = 7;
    TestCData *pending = cdw;
    pending->node_ref();
    node_unref_delete<CycleData>((CycleData *)pending);
    cdw->_b = 49;  // use-after-free here if the copy was freed
    if (cdw->_a != 7 || cdw->_b != 49) {
      std::fprintf(stderr, "FAIL refcount-at-creation: a=%llu b=%llu\n",
                   (unsigned long long)cdw->_a, (unsigned long long)cdw->_b);
      ++failures;
    }
  }

  std::fprintf(stderr, "[ryw] failures=%d\n%s\n", failures,
               failures == 0 ? "PASS" : "FAIL");
  return failures == 0 ? 0 : 1;
}

// Reproduces test_node_subclass_gc: under forced COW, does dropping the last
// reference to a parent free its child immediately, or is the child stranded
// in the EBR retire queue until a reclamation point?
class CountedNode : public PandaNode {
public:
  CountedNode(const std::string &name) : PandaNode(name) { ++_alive; }
  ~CountedNode() { --_alive; }
  static int _alive;
};
int CountedNode::_alive = 0;

static int run_gc_lifetime() {
  load_prc_file_data("gc", "pipeline-always-cow true\n");
  int leaked = 0;
  {
    PT(PandaNode) parent = new PandaNode("parent");
    PT(CountedNode) child = new CountedNode("child");
    parent->add_child(child);
    child = nullptr;  // only the parent's CData holds the child now
    int after_drop = CountedNode::_alive;
    parent = nullptr; // parent destructs; under always-COW its final CData
                      // (which still holds the child) is retired, not freed --
                      // so the child may linger until a reclamation point.
    int after_parent = CountedNode::_alive;
    // A real application reaches a full reclamation point every frame/tick
    // (Pipeline::cycle / AsyncTaskManager::poll drain the backlog unbounded).
    // Model that here via the explicit drain and require the child to be freed
    // then; a genuine leak (a missing node_ref balance) would survive it.
    Thread::get_current_thread()->yield_quiescent();
    int after_reclaim = CountedNode::_alive;
    std::fprintf(stderr,
                 "[gc] after_drop=%d after_parent=%d retired=%zu after_reclaim=%d\n",
                 after_drop, after_parent,
                 EpochManager::get_retired_count(), after_reclaim);
    leaked = (after_reclaim != 0) ? 1 : 0;
  }
  std::fprintf(stderr, "%s\n", leaked == 0 ? "PASS" : "LEAKED");
  return leaked;
}

int main(int argc, char **argv) {
  load_prc_file_data("test_ebr_cycler",
      "notify-level fatal\n"
      "default-directnotify-level fatal\n"
      "threading-model Cull\n");

  PandaFramework framework;
  framework.open_framework(argc, argv);

  bool churn = false;
  int seconds = 5;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--unbound-churn") == 0) churn = true;
    else if (std::strcmp(argv[i], "--read-your-writes") == 0) {
      std::_Exit(run_read_your_writes());
    }
    else if (std::strcmp(argv[i], "--gc-lifetime") == 0) {
      std::_Exit(run_gc_lifetime());
    }
    else seconds = std::atoi(argv[i]);
  }
  if (churn) {
    std::_Exit(run_unbound_churn(seconds));
  }

  PipelineCycler<TestCData> cycler;

  std::atomic<bool> stop{false};
  std::atomic<uint64_t> read_iters{0};
  std::atomic<uint64_t> write_iters{0};
  std::atomic<uint64_t> tears{0};

  std::thread writer([&] {
    PT(Thread) th = Thread::bind_thread("writer", "writer");
    uint64_t v = 1;
    while (!stop.load(std::memory_order_relaxed)) {
      {
        EpochHolder epoch(th.p());
        CycleDataStageWriter<TestCData> cdw(cycler, 0, th.p());
        cdw->_a = v;
        cdw->_b = v * v;
      }
      ++v;
      write_iters.fetch_add(1, std::memory_order_relaxed);
    }
  });

  std::thread reader([&] {
    PT(Thread) th = Thread::bind_thread("reader", "reader");
    while (!stop.load(std::memory_order_relaxed)) {
      CycleDataReader<TestCData> cdr(cycler, th);
      uint64_t a = cdr->_a;
      uint64_t b = cdr->_b;
      if (a != 0 && b != a * a) {
        tears.fetch_add(1, std::memory_order_relaxed);
        std::fprintf(stderr, "TEAR: a=%llu b=%llu (expected %llu)\n",
                     (unsigned long long)a, (unsigned long long)b,
                     (unsigned long long)(a * a));
      }
      read_iters.fetch_add(1, std::memory_order_relaxed);
    }
  });

  std::this_thread::sleep_for(std::chrono::seconds(seconds));
  stop.store(true);

  writer.join();
  reader.join();

  std::fprintf(stderr, "[ebr_test] writes=%llu reads=%llu tears=%llu\n",
               (unsigned long long)write_iters.load(),
               (unsigned long long)read_iters.load(),
               (unsigned long long)tears.load());

  if (tears.load() != 0) {
    std::fprintf(stderr, "FAIL\n");
    std::_Exit(1);
  }
  std::fprintf(stderr, "PASS\n");
  std::_Exit(0);
}
