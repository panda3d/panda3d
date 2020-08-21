/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_client.cxx
 * @author drose
 * @date 2000-07-09
 */

#include "config_pstatclient.h"
#include "pStatClient.h"
#include "pStatCollector.h"
#include "thread.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"
#include "netAddress.h"
#include "connection.h"
#include "netDatagram.h"

#include <signal.h>
#include <algorithm>

static bool user_interrupted = false;

// Define a signal handler so we can clean up on control-C.
void signal_handler(int) {
  user_interrupted = true;
}

struct SampleData {
  const char *category;
  double min_ms;
  double max_ms;
  bool is_level;
};

SampleData dataset_zero[] = {
  { "Draw", 10, 10, false },
  { "Cull", 5, 6, false },
  { "App", 0, 5, false },
  { "Texture memory", 8000000, 100000, true },
  { nullptr },
};

SampleData dataset_one[] = {
  { "Draw", 10, 12, false },
  { "Squeak", 25, 30, false },
  { nullptr },
};

SampleData dataset_two[] = {
  { "Squeak", 40, 45, false },
  { "Cull", 20, 22, false },
  { "Draw", 10, 20, false },
  { "Animation", 0, 0, false },
  { "Animation:mickey", 5, 6, false },
  { "Animation:donald", 5, 6, false },
  { "Animation:goofy", 5, 6, false },
  { "Animation:pluto", 5, 6, false },
  { nullptr },
};

#define NUM_DATASETS 3
SampleData *datasets[NUM_DATASETS] = {
  dataset_zero, dataset_one, dataset_two
};


class WaitRequest {
public:
  double _time;
  int _index;
  bool _start;

  bool operator < (const WaitRequest &other) const {
    return _time < other._time;
  }
};

int
main(int argc, char *argv[]) {
  std::string hostname = "localhost";
  int port = pstats_port;

  if (argc > 1) {
    hostname = argv[1];
  }
  if (argc > 2) {
    port = atoi(argv[2]);
    if (port == 0) {
      nout << "Invalid port number: " << argv[2] << "\n";
      exit(1);
    }
  }
  if (argc > 4) {
    nout << "test_client host port [dataset]\n";
    exit(1);
  }

  notify(SIGINT, &signal_handler);

  PStatClient *client = PStatClient::get_global_pstats();
  client->set_client_name("Bogus Stats");

  if (!client->connect(hostname, port)) {
    nout << "Couldn't connect.\n";
    exit(1);
  }

  srand(time(nullptr));

  int ds_index;
  if (argc > 3) {
    ds_index = atoi(argv[3]);
  } else {
    // Pick a random Dataset.
    ds_index = (int)((double)NUM_DATASETS * rand() / (RAND_MAX + 1.0));
  }
  if (ds_index < 0 || ds_index >= NUM_DATASETS) {
    nout << "Invalid dataset; choose a number in the range 0 to "
         << NUM_DATASETS - 1 << "\n";
    exit(1);
  }

  SampleData *ds = datasets[ds_index];

  pvector<PStatCollector> _collectors;
  int i = 0;
  while (ds[i].category != nullptr) {
    _collectors.push_back(PStatCollector(ds[i].category));
    if (ds[i].is_level) {
      _collectors[i].set_level(ds[i].min_ms);
    }
    i++;
  }

  while (!user_interrupted && client->is_connected()) {
    PStatClient::main_tick();

    double total_ms = 0.0;
    double now = client->get_real_time();
    double start = now;

    typedef pvector<WaitRequest> Wait;
    Wait wait;

    // Make up some random intervals to "wait".
    for (i = 0; i < (int)_collectors.size(); i++) {
      if (ds[i].is_level) {
        // Make up an amount to adddelete to the level this frame.
        double increment = ds[i].max_ms * (rand() / (RAND_MAX + 1.0) - 0.5);
        _collectors[i].add_level(increment);

      } else {
        // A bit of random jitter so the collectors might overlap some.
        double jitter_ms = (5.0 * rand() / (RAND_MAX + 1.0));

        WaitRequest wr;
        wr._time = now + jitter_ms / 1000.0;
        wr._index = i;
        wr._start = true;
        wait.push_back(wr);

        double ms_range = ds[i].max_ms - ds[i].min_ms;
        double ms = (double)ds[i].min_ms +
          (ms_range * rand() / (RAND_MAX + 1.0));
        now += ms / 1000.0;
        total_ms += ms;

        wr._time = now + jitter_ms / 1000.0;
        wr._start = false;
        wait.push_back(wr);
      }
    }

    // Put the wait requests in order, to allow for the jitter, and invoke
    // them.

    static const double delay = 1.0;
    _collectors[0].stop(client->get_main_thread(), start + delay);

    sort(wait.begin(), wait.end());
    Wait::const_iterator wi;
    for (wi = wait.begin(); wi != wait.end(); ++wi) {
      const WaitRequest &wr = (*wi);
      if (wr._start) {
        _collectors[wr._index].start(client->get_main_thread(), wr._time + delay);
      } else {
        _collectors[wr._index].stop(client->get_main_thread(), wr._time + delay);
      }
    }

    _collectors[0].start(client->get_main_thread(), now + total_ms / 1000 + delay);

    // Now actually wait some approximation of the time we said we did.
    Thread::sleep(total_ms / 1000.0 + delay);
  }

  return (0);
}
