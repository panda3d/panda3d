// Filename: config_pstats.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pstats.h"

#include <dconfig.h>

Configure(config_pstats);
NotifyCategoryDef(pstats, "");

ConfigureFn(config_pstats) {
}

string get_pstats_name() {
  return config_pstats.GetString("pstats-name", "Panda Stats");
}

double get_pstats_max_rate() {
  return config_pstats.GetDouble("pstats-max-rate", 30.0);
}

const string pstats_host = config_pstats.GetString("pstats-host", "localhost");
const int pstats_port = config_pstats.GetInt("pstats-port", 5180);
const double pstats_target_frame_rate = config_pstats.GetDouble("pstats-target-frame-rate", 30.0);

// The rest are different in that they directly control the server,
// not the client.
const bool pstats_scroll_mode = config_pstats.GetBool("pstats-scroll-mode", true);
const double pstats_history = config_pstats.GetDouble("pstats-history", 30.0);
