// Filename: config_notify.h
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NOTIFY_H
#define CONFIG_NOTIFY_H

#include <dtoolbase.h>
#include "dconfig.h"

ConfigureDecl(config_notify, EXPCL_DTOOLCONFIG, EXPTP_DTOOLCONFIG);

bool get_assert_abort();
bool get_notify_timestamp();

#endif /* __CONFIG_NOTIFY_H__ */
