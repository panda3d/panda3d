// Filename: config_glxdisplay.h
// Created by:  cary (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_GLXDISPLAY_H__
#define __CONFIG_GLXDISPLAY_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(glxdisplay, EXPCL_PANDAGL, EXPTP_PANDAGL);

extern bool gl_show_fps_meter;
extern float gl_fps_meter_update_interval;

#endif /* __CONFIG_GLXDISPLAY_H__ */
