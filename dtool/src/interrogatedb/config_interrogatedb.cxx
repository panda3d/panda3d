// Filename: config_interrogatedb.C
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "config_interrogatedb.h"
#include "interrogate_request.h"

#include <dconfig.h>

Configure(config_interrogatedb);
NotifyCategoryDef(interrogatedb, "");

ConfigureFn(config_interrogatedb) {
  //  interrogate_request_library("types");
}

const DSearchPath &
get_interrogatedb_path() {
  static DSearchPath *interrogatedb_path = NULL;
  if (interrogatedb_path == (DSearchPath *)NULL) {
    interrogatedb_path = new DSearchPath(".");
    interrogatedb_path->append_path
      (config_interrogatedb.GetString("ETC_PATH", "."));
  }
  return *interrogatedb_path;
}

