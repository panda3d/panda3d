// Filename: config_flt.h
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_FLT_H
#define CONFIG_FLT_H

#include <pandatoolbase.h>

#include <notifyCategoryProxy.h>

NotifyCategoryDeclNoExport(flt);

extern const bool flt_error_abort;

extern void init_libflt();

#endif
