// Filename: config_notify.h
// Created by:  drose (29Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NOTIFY_H
#define CONFIG_NOTIFY_H

#include <dtoolbase.h>
#include "dconfig.h"

ConfigureDecl(config_notify, EXPCL_DTOOL, EXPTP_DTOOL);

bool get_assert_abort();

#endif /* __CONFIG_NOTIFY_H__ */
