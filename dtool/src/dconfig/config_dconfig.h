// Filename: config_dconfig.h
// Created by:  drose (15May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DCONFIG_H
#define CONFIG_DCONFIG_H

#ifdef WIN32_VC
/* C4231: extern before template instantiation */
/* MPG - For some reason, this one only works if it's here */
#pragma warning (disable : 4231)
#endif

#include <dtoolbase.h>

#include "notifyCategoryProxy.h"

NotifyCategoryDecl(dconfig, EXPCL_DTOOL, EXPTP_DTOOL);
NotifyCategoryDecl(microconfig, EXPCL_DTOOL, EXPTP_DTOOL);

#endif
