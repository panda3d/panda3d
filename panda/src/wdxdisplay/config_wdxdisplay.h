// Filename: config_wdxdisplay.h
// Created by:  mike (07Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_WDXDISPLAY_H__
#define __CONFIG_WDXDISPLAY_H__

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(wdxdisplay, EXPCL_PANDADX, EXPTP_PANDADX);

extern string IconFileName;

extern EXPCL_PANDADX void init_libwdxdisplay();

#endif /* __CONFIG_WDXDISPLAY_H__ */
