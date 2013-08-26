// Filename: config_ffmpeg.cxx
// Created by:  rdb (23Aug13)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_ffmpeg.h"
#include "dconfig.h"
#include "ffmpegVideo.h"
#include "ffmpegVideoCursor.h"
#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"

#include "movieTypeRegistry.h"

extern "C" {
  #include "libavcodec/avcodec.h"
}

ConfigureDef(config_ffmpeg);
NotifyCategoryDef(ffmpeg, "");

ConfigureFn(config_ffmpeg) {
  init_libffmpeg();
}

ConfigVariableInt ffmpeg_max_readahead_frames
("ffmpeg-max-readahead-frames", 2,
 PRC_DESC("The maximum number of frames ahead which an ffmpeg decoder thread "
          "should read in advance of actual playback.  Set this to 0 to "
          "decode ffmpeg videos in the main thread."));

ConfigVariableBool ffmpeg_show_seek_frames
("ffmpeg-show-seek-frames", true,
 PRC_DESC("Set this true to allow showing the intermediate results of seeking "
          "through the ffmpeg stream to a target frame, or false to hold the "
          "current frame until the target frame is achieved.  This has the "
          "biggest effect on videos that are too expensive to decode in real "
          "time: when this is true, the video can be seen to animate at least "
          "a little bit; when it is false, you may get long periods of one "
          "held frame."));

ConfigVariableBool ffmpeg_support_seek
("ffmpeg-support-seek", true,
 PRC_DESC("True to use the av_seek_frame() function to seek within ffmpeg "
          "video files.  If this is false, Panda will only seek within a "
          "file by reading it from the beginning until the desired point, "
          "which can be much slower.  Set this false only if you suspect "
          "a problem with av_seek_frame()."));

ConfigVariableBool ffmpeg_global_lock
("ffmpeg-global-lock", false,
 PRC_DESC("Set this true to enable a single global mutex across *all* ffmpeg "
          "operations.  Leave this false to use the mutex only for "
          "the ffmpeg operations that are generally known to be "
          "not thread-safe.  This will negatively affect ffmpeg performance, "
          "especially when decoding multiple videos at once (including the "
          "left and right channels of a stereo video).  Set this true only "
          "if you suspect a problem with ffmpeg's own thread-safe nature."));

ConfigVariableEnum<ThreadPriority> ffmpeg_thread_priority
("ffmpeg-thread-priority", TP_normal,
 PRC_DESC("The default thread priority at which to start ffmpeg decoder "
          "threads."));

ConfigVariableInt ffmpeg_read_buffer_size
("ffmpeg-read-buffer-size", 4096,
 PRC_DESC("The size in bytes of the buffer used when reading input files. "
          "This is important for performance.  A typical size is that of a "
          "cache page, e.g. 4kb."));

////////////////////////////////////////////////////////////////////
//     Function: init_libffmpeg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libffmpeg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FfmpegVirtualFile::register_protocol();

  FfmpegVideo::init_type();
  FfmpegVideoCursor::init_type();
  FfmpegAudio::init_type();
  FfmpegAudioCursor::init_type();

  FfmpegVideo::register_with_read_factory();
  FfmpegVideoCursor::register_with_read_factory();

  // Register ffmpeg as catch-all audio/video type.
  MovieTypeRegistry *reg = MovieTypeRegistry::get_global_ptr();
  reg->register_audio_type(&FfmpegAudio::make, "*");
  reg->register_video_type(&FfmpegVideo::make, "*");
}
