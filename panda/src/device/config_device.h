// Filename: config_device.h
// Created by:  drose (04May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DEVICE_H
#define CONFIG_DEVICE_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(device, EXPCL_PANDA, EXPTP_PANDA);

extern const bool asynchronous_clients;

#endif
