// Filename: config_chan.h
// Created by:  drose (28Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_CHAN_H
#define CONFIG_CHAN_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

// Configure variables for chan package.
NotifyCategoryDecl(chan, EXPCL_PANDA, EXPTP_PANDA);

EXPCL_PANDA extern const bool quantize_bam_channels;

#endif
