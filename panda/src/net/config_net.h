// Filename: config_net.h
// Created by:  drose (25Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_NET_H
#define CONFIG_NET_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

// Configure variables for net package.

NotifyCategoryDecl(net, EXPCL_PANDA, EXPTP_PANDA);

extern int get_net_max_write_queue();
extern int get_net_max_response_queue();
extern bool get_net_error_abort();

#endif
