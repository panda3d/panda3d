// Filename: config_downloader.cxx
// Created by:  mike (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

#include <dconfig.h>
#include <get_config_path.h>

Configure(config_downloader);
NotifyCategoryDef(downloader, "");

// We'd like this to be about 1 second worth of download assuming a
// 28.8Kb connection (28.8Kb / 8 = 3600 bytes per second).
const int downloader_buffer_size = 
	config_downloader.GetInt("downloader-buffer-size", 3600);

// Frequency of download chunk requests in seconds (or fractions of)
// (Estimated 200 msec round-trip to server).
const float downloader_frequency =
	config_downloader.GetFloat("downloader-frequency", 0.2);

// Estimated available bandwidth (assume a 28.8Kb connection, so again
// we have 3600 bytes per second).
const float downloader_bandwidth =
	config_downloader.GetFloat("downloader-bandwidth", 3600.0);

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
