// Filename: config_tform.h
// Created by:  drose (23Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_TFORM_H
#define CONFIG_TFORM_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(tform, EXPCL_PANDA, EXPTP_PANDA);

// Configure variables for tform package.
extern const double EXPCL_PANDA drive_forward_speed;
extern const double EXPCL_PANDA drive_reverse_speed;
extern const double EXPCL_PANDA drive_rotate_speed;
extern const double EXPCL_PANDA drive_vertical_dead_zone;
extern const double EXPCL_PANDA drive_vertical_center;
extern const double EXPCL_PANDA drive_horizontal_dead_zone;
extern const double EXPCL_PANDA drive_horizontal_center;
extern const double EXPCL_PANDA drive_vertical_ramp_up_time;
extern const double EXPCL_PANDA drive_vertical_ramp_down_time;
extern const double EXPCL_PANDA drive_horizontal_ramp_up_time;
extern const double EXPCL_PANDA drive_horizontal_ramp_down_time;

#endif
