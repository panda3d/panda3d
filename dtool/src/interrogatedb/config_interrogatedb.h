// Filename: config_interrogatedb.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_INTERROGATEDB_H
#define CONFIG_INTERROGATEDB_H

#include <dtoolbase.h>
#include <notifyCategoryProxy.h>
#include <dSearchPath.h>

NotifyCategoryDecl(interrogatedb, EXPCL_DTOOL, EXPTP_DTOOL);

const DSearchPath &get_interrogatedb_path();

#endif
