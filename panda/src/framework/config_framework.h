// Filename: config_framework.h
// Created by:  drose (06Sep00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_FRAMEWORK_H
#define CONFIG_FRAMEWORK_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>
#include <collideMask.h>

NotifyCategoryDecl(framework, EXPCL_FRAMEWORK, EXPTP_FRAMEWORK);

// Configure variables for framework package.
extern const double EXPCL_FRAMEWORK drive_height;
extern const CollideMask EXPCL_FRAMEWORK drive_mask;

#endif
