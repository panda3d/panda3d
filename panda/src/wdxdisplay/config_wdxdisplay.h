// Filename: config_wdxdisplay.h
// Created by:  mike (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_WDXDISPLAY_H__
#define __CONFIG_WDXDISPLAY_H__

#include <pandabase.h>
#include <filename.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(wdxdisplay, EXPCL_PANDADX, EXPTP_PANDADX);

extern Filename get_icon_filename();

extern EXPCL_PANDADX void init_libwdxdisplay();

#endif /* __CONFIG_WDXDISPLAY_H__ */
