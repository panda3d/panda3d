// Filename: config_downloader.cxx
// Created by:  mike (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

#include <dconfig.h>
#include <get_config_path.h>

Configure(config_downloader);
NotifyCategoryDef(downloader, "");

// How often we write to disk is determined by this ratio which is
// relative to the downloader-byte-rate (e.g. if disk-write-ratio is 4,
// we will write every 4 seconds if the frequency is 0.2)
const int downloader_disk_write_frequency =
        config_downloader.GetInt("downloader-disk-write-frequency", 4);

// We'd like this to be about 1 second worth of download assuming a
// 28.8Kb connection (28.8Kb / 8 = 3600 bytes per second).
const int downloader_byte_rate = 
        config_downloader.GetInt("downloader-byte-rate", 3600);

// Frequency of download chunk requests in seconds (or fractions of)
// (Estimated 200 msec round-trip to server).
const float downloader_frequency =
        config_downloader.GetFloat("downloader-frequency", 0.2);

const int downloader_timeout =
        config_downloader.GetInt("downloader-timeout", 15);

const int downloader_timeout_retries =
        config_downloader.GetInt("downloader-timeout-retries", 5);

const int decompressor_buffer_size =
        config_downloader.GetInt("decompressor-buffer-size", 4096);

const float decompressor_frequency =
        config_downloader.GetFloat("decompressor-frequency", 0.2);

const int extractor_buffer_size =
        config_downloader.GetInt("extractor-buffer-size", 4096);

const float extractor_frequency =
        config_downloader.GetFloat("extractor-frequency", 0.2);

const int patcher_buffer_size =
        config_downloader.GetInt("patcher-buffer-size", 4096);

ConfigureFn(config_downloader) {
}
