// Filename: test_client.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pstats.h"
#include "pStatClient.h"
#include "pStatCollector.h"

#include <luse.h>
#include <queuedConnectionManager.h>
#include <queuedConnectionReader.h>
#include <connectionWriter.h>
#include <netAddress.h>
#include <connection.h>
#include <netDatagram.h>

#include <signal.h>
#include <algorithm>

static bool user_interrupted = false;

// Define a signal handler so we can clean up on control-C.
void signal_handler(int) {
  user_interrupted = true;
}

struct SampleData {
  const char *category;
  RGBColorf color;
  int min_ms;
  int max_ms;
};

SampleData dataset_zero[] = {
  { "Draw", RGBColorf(0,0,1), 10, 10 },
  { "Cull", RGBColorf(0,1,1), 5, 6 },
  { "App", RGBColorf(1,0,0), 0, 5 },
  { NULL },
};

SampleData dataset_one[] = {
  { "Draw", RGBColorf(0,0,1), 10, 12 },
  { "Squeak", RGBColorf(1,1,0), 25, 30 },
  { NULL },
};

SampleData dataset_two[] = {
  { "Squeak", RGBColorf(1,1,0), 40, 45 },
  { "Cull", RGBColorf(0,1,1), 20, 22 },
  { "Draw", RGBColorf(0,0,1), 10, 20 },
  { "Animation", RGBColorf(0.5,1,0.5), 0, 0 },
  { "Animation:mickey", RGBColorf(0,0,0), 5, 6 },
  { "Animation:donald", RGBColorf(0,0,0), 5, 6 },
  { "Animation:goofy", RGBColorf(0,0,0), 5, 6 },
  { "Animation:pluto", RGBColorf(0,0,0), 5, 6 },
  { NULL },
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
  string hostname = "localhost";
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

  signal(SIGINT, &signal_handler);

  PStatClient *client = PStatClient::get_global_pstats();
  client->set_client_name("Bogus Stats");

  if (!client->connect(hostname, port)) {
    nout << "Couldn't connect.\n";
    exit(1);
  }
  
  srand(time(NULL));

  int ds_index;
  if (argc > 3) {
    ds_index = atoi(argv[3]);
  } else {
    // Pick a random Dataset.
    ds_index = (int)((double)NUM_DATASETS * rand() / (RAND_MAX + 1.0));
  }
  if (ds_index < 0 || ds_index >= NUM_DATASETS) {
    nout << "Invalid datasets; choose a number in the range 0 to "
	 << NUM_DATASETS - 1 << "\n";
    exit(1);
  }

  SampleData *ds = datasets[ds_index];

  vector<PStatCollector> _collectors;
  int i = 0;
  while (ds[i].category != (const char *)NULL) {
    _collectors.push_back(PStatCollector(ds[i].category, ds[i].color, i));
    i++;
  }
  
  while (!user_interrupted && client->is_connected()) {
    client->get_main_thread().new_frame();

    double total_ms = 0.0;
    double now = client->get_clock().get_real_time();

    typedef vector<WaitRequest> Wait;
    Wait wait;

    // Make up some random intervals to "wait".
    for (i = 0; i < _collectors.size(); i++) {
      // A bit of random jitter so the collectors might overlap some.
      double jitter_ms = (5.0 * rand() / (RAND_MAX + 1.0));

      WaitRequest wr;
      wr._time = now + jitter_ms / 1000.0;
      wr._index = i;
      wr._start = true;
      wait.push_back(wr);

      int ms_range = ds[i].max_ms - ds[i].min_ms;
      double ms = (double)ds[i].min_ms + 
	((double)ms_range * rand() / (RAND_MAX + 1.0));
      now += ms / 1000.0;
      total_ms += ms;

      wr._time = now + jitter_ms / 1000.0;
      wr._start = false;
      wait.push_back(wr);
    }

    // Put the wait requests in order, to allow for the jitter, and
    // invoke them.
    sort(wait.begin(), wait.end());
    Wait::const_iterator wi;
    for (wi = wait.begin(); wi != wait.end(); ++wi) {
      const WaitRequest &wr = (*wi);
      if (wr._start) {
	_collectors[wr._index].start(client->get_main_thread(), wr._time);
      } else {
	_collectors[wr._index].stop(client->get_main_thread(), wr._time);
      }
    }

    // Now actually wait some approximation of the time we said we
    // did.
    PRIntervalTime sleep_timeout = 
      PR_MillisecondsToInterval((int)total_ms + 5);
    PR_Sleep(sleep_timeout);
  }

  return (0);
}
