// Filename: config_sgraphutil.h
// Created by:  drose (21Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_SGRAPHUTIL_H
#define CONFIG_SGRAPHUTIL_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

// Configure variables for sgraphutil package.

NotifyCategoryDecl(sgraphutil, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA const bool view_frustum_cull;
extern EXPCL_PANDA const bool fake_view_frustum_cull;
extern EXPCL_PANDA const bool implicit_app_traversal;


#endif
