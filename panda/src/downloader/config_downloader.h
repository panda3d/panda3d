// Filename: config_downloader.h
// Created by:  mike (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DOWNLOADER_H
#define CONFIG_DOWNLOADER_H

#include <pandabase.h>
#include <notifyCategoryProxy.h>

NotifyCategoryDecl(downloader, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

extern const int downloader_buffer_size;
extern const float downloader_frequency;
extern const float downloader_bandwidth;
extern const int downloader_timeout;

extern const int decompressor_buffer_size;
extern const float decompressor_frequency;

extern const int extractor_buffer_size;
extern const float extractor_frequency;

extern const int patcher_buffer_size;

#endif
