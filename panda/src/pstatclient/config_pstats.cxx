// Filename: config_pstats.cxx
// Created by:  drose (09Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "config_pstats.h"

#include <dconfig.h>

Configure(config_pstats);
NotifyCategoryDef(pstats, "");

ConfigureFn(config_pstats) {
  init_libpstatclient();
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

////////////////////////////////////////////////////////////////////
//     Function: init_libpstatclient
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpstatclient() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
}

