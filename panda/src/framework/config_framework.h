// Filename: config_framework.h
// Created by:  drose (06Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_FRAMEWORK_H
#define CONFIG_FRAMEWORK_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <collideMask.h>

NotifyCategoryDecl(framework, EXPCL_EMPTY, EXPCL_EMPTY);

// Configure variables for framework package.
extern const double drive_height;
extern const CollideMask drive_mask;

#endif
