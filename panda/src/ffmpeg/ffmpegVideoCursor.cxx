/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ffmpegVideoCursor.cxx
 * @author jyelon
 * @date 2007-08-01
 */

#include "ffmpegVideoCursor.h"
#include "config_ffmpeg.h"
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "mutexHolder.h"
#include "reMutexHolder.h"
#include "ffmpegVideo.h"
#include "bamReader.h"
extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libavutil/pixdesc.h>
#ifdef HAVE_SWSCALE
  #include <libswscale/swscale.h>
#endif
}

ReMutex FfmpegVideoCursor::_av_lock;
TypeHandle FfmpegVideoCursor::_type_handle;
TypeHandle FfmpegVideoCursor::FfmpegBuffer::_type_handle;

PStatCollector FfmpegVideoCursor::_fetch_buffer_pcollector("*:FFMPEG Video Decoding:Fetch");
PStatCollector FfmpegVideoCursor::_seek_pcollector("*:FFMPEG Video Decoding:Seek");
PStatCollector FfmpegVideoCursor::_export_frame_pcollector("*:FFMPEG Convert Video to BGR");

#if LIBAVUTIL_VERSION_INT < AV_VERSION_INT(52, 32, 100)
  #define AV_PIX_FMT_FLAG_ALPHA PIX_FMT_ALPHA
#endif

/**
 * This constructor is only used when reading from a bam file.
 */
FfmpegVideoCursor::
FfmpegVideoCursor() :
  _max_readahead_frames(0),
  _thread_priority(ffmpeg_thread_priority),
  _lock("FfmpegVideoCursor::_lock"),
  _action_cvar(_lock),
  _thread_status(TS_stopped),
  _seek_frame(0),
  _packet(nullptr),
  _format_ctx(nullptr),
  _video_ctx(nullptr),
  _convert_ctx(nullptr),
  _pixel_format((int)AV_PIX_FMT_NONE),
  _video_index(-1),
  _frame(nullptr),
  _frame_out(nullptr),
  _eof_known(false)
{
}

/**
 * Specifies the source of the video cursor.  This is normally called only by
 * the constructor or when reading from a bam file.
 */
void FfmpegVideoCursor::
init_from(FfmpegVideo *source) {
  nassertv(_thread == nullptr && _thread_status == TS_stopped);
  nassertv(source != nullptr);
  _source = source;
  _filename = _source->get_filename();

  if (!open_stream()) {
    cleanup();
    return;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
  _frame = av_frame_alloc();
  _frame_out = av_frame_alloc();
#else
  _frame = avcodec_alloc_frame();
  _frame_out = avcodec_alloc_frame();
#endif

  if ((_frame == nullptr)||(_frame_out == nullptr)) {
    cleanup();
    return;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
  _packet = av_packet_alloc();
#else
  _packet = new AVPacket;
  av_init_packet(_packet);
#endif

  fetch_packet(0);
  fetch_frame(-1);
  _initial_dts = _begin_frame;

  _current_frame = -1;
  _eof_known = false;
  _eof_frame = 0;

  ReMutexHolder av_holder(_av_lock);

  // Check if we got an alpha format.  Please note that some video codecs
  // (eg. libvpx) change the pix_fmt after decoding the first frame, which is
  // why we didn't do this earlier.
  switch (_video_ctx->pix_fmt) {
  case AV_PIX_FMT_GRAY8:
    _num_components = 1;
    _pixel_format = (int)AV_PIX_FMT_GRAY8;
    break;
  case AV_PIX_FMT_Y400A: // aka AV_PIX_FMT_YA8
    _num_components = 2;
    _pixel_format = (int)AV_PIX_FMT_Y400A;
    break;
  default:
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(_video_ctx->pix_fmt);
    if (desc && (desc->flags & AV_PIX_FMT_FLAG_ALPHA) != 0) {
      _num_components = 4;
      _pixel_format = (int)AV_PIX_FMT_BGRA;
    } else {
      _num_components = 3;
      _pixel_format = (int)AV_PIX_FMT_BGR24;
    }
    break;
  }

#ifdef HAVE_SWSCALE
  nassertv(_convert_ctx == nullptr);
  _convert_ctx = sws_getContext(_size_x, _size_y, _video_ctx->pix_fmt,
                                _size_x, _size_y, (AVPixelFormat)_pixel_format,
                                SWS_BILINEAR | SWS_PRINT_INFO, nullptr, nullptr, nullptr);
#else
  if (_video_ctx->pix_fmt != _pixel_format) {
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(_video_ctx->pix_fmt);
    ffmpeg_cat.error()
      << "Video with pixel format " << (desc ? desc->name : "?")
      << " needs conversion, but libswscale support is not enabled.\n";
    cleanup();
    return;
  }
#endif  // HAVE_SWSCALE

#ifdef HAVE_THREADS
  set_max_readahead_frames(ffmpeg_max_readahead_frames);
#endif  // HAVE_THREADS
}

/**
 *
 */
FfmpegVideoCursor::
FfmpegVideoCursor(FfmpegVideo *src) :
  _max_readahead_frames(0),
  _thread_priority(ffmpeg_thread_priority),
  _lock("FfmpegVideoCursor::_lock"),
  _action_cvar(_lock),
  _thread_status(TS_stopped),
  _seek_frame(0),
  _packet(nullptr),
  _format_ctx(nullptr),
  _video_ctx(nullptr),
  _convert_ctx(nullptr),
  _video_index(-1),
  _frame(nullptr),
  _frame_out(nullptr),
  _eof_known(false)
{
  init_from(src);
}

/**
 *
 */
FfmpegVideoCursor::
~FfmpegVideoCursor() {
  cleanup();
}

/**
 * Specifies the maximum number of frames that a sub-thread will attempt to
 * read ahead of the current frame.  Setting this to a nonzero allows the
 * video decoding to take place in a sub-thread, which smoothes out the video
 * decoding time by spreading it evenly over several frames.  Set this number
 * larger to increase the buffer between the currently visible frame and the
 * first undecoded frame; set it smaller to reduce memory consumption.
 *
 * Setting this to zero forces the video to be decoded in the main thread.  If
 * threading is not available in the Panda build, this value is always zero.
 */
void FfmpegVideoCursor::
set_max_readahead_frames(int max_readahead_frames) {
#ifndef HAVE_THREADS
  if (max_readahead_frames > 0) {
    ffmpeg_cat.warning()
      << "Couldn't set max_readahead_frames to " << max_readahead_frames
      << ": threading not available.\n";
    max_readahead_frames = 0;
  }
#endif  // HAVE_THREADS

  _max_readahead_frames = max_readahead_frames;
  if (_max_readahead_frames > 0) {
    if (_thread_status == TS_stopped) {
      start_thread();
    }
  } else {
    if (_thread_status != TS_stopped) {
      stop_thread();
    }
  }
}

/**
 * Returns the maximum number of frames that a sub-thread will attempt to read
 * ahead of the current frame.  See set_max_readahead_frames().
 */
int FfmpegVideoCursor::
get_max_readahead_frames() const {
  return _max_readahead_frames;
}

/**
 * Changes the thread priority of the thread that decodes the ffmpeg video
 * stream (if max_readahead_frames is nonzero).  Normally you shouldn't mess
 * with this, but there may be special cases where a precise balance of CPU
 * utilization between the main thread and the various ffmpeg service threads
 * may be needed.
 */
void FfmpegVideoCursor::
set_thread_priority(ThreadPriority thread_priority) {
  if (_thread_priority != thread_priority) {
    _thread_priority = thread_priority;
    if (is_thread_started()) {
      stop_thread();
      start_thread();
    }
  }
}

/**
 * Returns the current thread priority of the thread that decodes the ffmpeg
 * video stream (if max_readahead_frames is nonzero).  See
 * set_thread_priority().
 */
ThreadPriority FfmpegVideoCursor::
get_thread_priority() const {
  return _thread_priority;
}

/**
 * Explicitly starts the ffmpeg decoding thread after it has been stopped by a
 * call to stop_thread().  The thread is normally started automatically, so
 * there is no need to call this method unless you have previously called
 * stop_thread() for some reason.
 */
void FfmpegVideoCursor::
start_thread() {
  MutexHolder holder(_lock);

  if (_thread_status == TS_stopped && _max_readahead_frames > 0) {
    // Get a unique name for the thread's sync name.
    std::ostringstream strm;
    strm << (void *)this;
    _sync_name = strm.str();

    // Create and start the thread object.
    _thread_status = TS_wait;
    _thread = new GenericThread(_filename.get_basename(), _sync_name, st_thread_main, this);
    if (!_thread->start(_thread_priority, true)) {
      // Couldn't start the thread.
      _thread = nullptr;
      _thread_status = TS_stopped;
    }
  }
}

/**
 * Explicitly stops the ffmpeg decoding thread.  There is normally no reason
 * to do this unless you want to maintain precise control over what threads
 * are consuming CPU resources.  Calling this method will make the video
 * update in the main thread, regardless of the setting of
 * max_readahead_frames, until you call start_thread() again.
 */
void FfmpegVideoCursor::
stop_thread() {
  if (_thread_status != TS_stopped) {
    PT(GenericThread) thread = _thread;
    {
      MutexHolder holder(_lock);
      if (_thread_status != TS_stopped) {
        _thread_status = TS_shutdown;
      }
      _action_cvar.notify();
      _thread = nullptr;
    }

    // Now that we've released the lock, we can join the thread.
    thread->join();
  }

  // This is a good time to clean up all of the allocated frame objects.  It's
  // not really necessary to be holding the lock, since the thread is gone,
  // but we'll grab it anyway just in case someone else starts the thread up
  // again.
  MutexHolder holder(_lock);

  _readahead_frames.clear();
}

/**
 * Returns true if the thread has been started, false if not.  This will
 * always return false if max_readahead_frames is 0.
 */
bool FfmpegVideoCursor::
is_thread_started() const {
  return (_thread_status != TS_stopped);
}

/**
 * See MovieVideoCursor::set_time().
 */
bool FfmpegVideoCursor::
set_time(double timestamp, int loop_count) {
  int frame = (int)(timestamp / _video_timebase + 0.5);

  if (_eof_known) {
    if (loop_count == 0) {
      frame = frame % (_eof_frame + 1);
    } else {
      int last_frame = (_eof_frame + 1) * loop_count;
      if (frame < last_frame) {
        frame = frame % (_eof_frame + 1);
      } else {
        frame = _eof_frame;
      }
    }
  }

  // No point in trying to position before the first frame.
  frame = std::max(frame, _initial_dts);

  if (ffmpeg_cat.is_spam() && frame != _current_frame) {
    ffmpeg_cat.spam()
      << "set_time(" << timestamp << "): " << frame
      << ", loop_count = " << loop_count << "\n";
  }

  _current_frame = frame;
  if (_current_frame_buffer != nullptr) {
    // If we've previously returned a frame, don't bother asking for a next
    // one if that frame is still valid.
    return (_current_frame >= _current_frame_buffer->_end_frame ||
            _current_frame < _current_frame_buffer->_begin_frame);
  }

  // If our last request didn't return a frame, try again.
  return true;
}

/**
 * See MovieVideoCursor::fetch_buffer.
 */
PT(MovieVideoCursor::Buffer) FfmpegVideoCursor::
fetch_buffer() {
  MutexHolder holder(_lock);

  // If there was an error at any point, just return NULL.
  if (_format_ctx == nullptr) {
    return nullptr;
  }

  PT(FfmpegBuffer) frame;
  if (_thread_status == TS_stopped) {
    // Non-threaded case.  Just get the next frame directly.
    advance_to_frame(_current_frame);
    if (_frame_ready) {
      frame = do_alloc_frame();
      export_frame(frame);
    }

  } else {
    // Threaded case.  Wait for the thread to serve up the required frames.
    if (!_readahead_frames.empty()) {
      frame = _readahead_frames.front();
      _readahead_frames.pop_front();
      _action_cvar.notify();
      while (frame->_end_frame < _current_frame && !_readahead_frames.empty()) {
        // This frame is too old.  Discard it.
        if (ffmpeg_cat.is_debug()) {
          ffmpeg_cat.debug()
            << "ffmpeg for " << _filename.get_basename()
            << " at frame " << _current_frame << ", discarding frame at "
            << frame->_begin_frame << "\n";
        }
        frame = _readahead_frames.front();
        _readahead_frames.pop_front();
      }
      if (frame->_begin_frame > _current_frame) {
        // This frame is too new.  Empty all remaining frames and seek
        // backwards.
        if (ffmpeg_cat.is_debug()) {
          ffmpeg_cat.debug()
            << "ffmpeg for " << _filename.get_basename()
            << " at frame " << _current_frame << ", encountered too-new frame at "
            << frame->_begin_frame << "\n";
        }
        do_clear_all_frames();
        if (_thread_status == TS_wait || _thread_status == TS_seek || _thread_status == TS_readahead) {
          _thread_status = TS_seek;
          _seek_frame = _current_frame;
          _action_cvar.notify();
        }
      }
    }
    if (frame == nullptr || frame->_end_frame < _current_frame) {
      // No frame available, or the frame is too old.  Seek.
      if (_thread_status == TS_wait || _thread_status == TS_seek || _thread_status == TS_readahead) {
        _thread_status = TS_seek;
        _seek_frame = _current_frame;
        _action_cvar.notify();
      }
    }
  }

  if (frame != nullptr) {
    bool too_old = (frame->_end_frame < _current_frame && !ffmpeg_show_seek_frames);
    bool too_new = frame->_begin_frame > _current_frame;
    if (too_old || too_new) {
      // The frame is too old or too new.  Just recycle it.
      frame = nullptr;
    }
  }

  if (frame != nullptr) {
    _current_frame_buffer = frame;
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "ffmpeg for " << _filename.get_basename()
        << " at frame " << _current_frame << ", returning frame at "
        << frame->_begin_frame << "\n";
    }
  } else {
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "ffmpeg for " << _filename.get_basename()
        << " at frame " << _current_frame << ", returning NULL\n";
    }
  }
  return frame;
}

/**
 * May be called by a derived class to allocate a new Buffer object.
 */
PT(MovieVideoCursor::Buffer) FfmpegVideoCursor::
make_new_buffer() {
  PT(FfmpegBuffer) frame = new FfmpegBuffer(size_x() * size_y() * get_num_components(), _video_timebase);
  return frame;
}

/**
 * Opens the stream for the first time, or when needed internally.
 */
bool FfmpegVideoCursor::
open_stream() {
  nassertr(!_ffvfile.is_open(), false);

  // Hold the global lock while we open the file and create avcodec objects.
  ReMutexHolder av_holder(_av_lock);

  if (!_source->get_subfile_info().is_empty()) {
    // Read a subfile.
    if (!_ffvfile.open_subfile(_source->get_subfile_info())) {
      ffmpeg_cat.info()
        << "Couldn't open " << _source->get_subfile_info() << "\n";
      close_stream();
      return false;
    }

  } else {
    // Read a filename.
    if (!_ffvfile.open_vfs(_filename)) {
      ffmpeg_cat.info()
        << "Couldn't open " << _filename << "\n";
      close_stream();
      return false;
    }
  }

  nassertr(_format_ctx == nullptr, false);
  _format_ctx = _ffvfile.get_format_context();
  nassertr(_format_ctx != nullptr, false);

  if (avformat_find_stream_info(_format_ctx, nullptr) < 0) {
    ffmpeg_cat.info()
      << "Couldn't find stream info\n";
    close_stream();
    return false;
  }

  nassertr(_video_ctx == nullptr, false);

  // As of libavformat version 57.41.100, AVStream.codec is deprecated in favor
  // of AVStream.codecpar.  Fortunately, the two structures have
  // similarly-named members, so we can just switch out the declaration.
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
  AVCodecParameters *codecpar;
#else
  AVCodecContext *codecpar;
#endif

  // Find the video stream
  AVStream *stream = nullptr;
  for (int i = 0; i < (int)_format_ctx->nb_streams; ++i) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
    codecpar = _format_ctx->streams[i]->codecpar;
#else
    codecpar = _format_ctx->streams[i]->codec;
#endif
    if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      _video_index = i;
      stream = _format_ctx->streams[i];
      break;
    }
  }

  if (stream == nullptr) {
    ffmpeg_cat.info()
      << "Couldn't find stream\n";
    close_stream();
    return false;
  }

  _video_timebase = av_q2d(stream->time_base);
  _min_fseek = (int)(3.0 / _video_timebase);

  AVCodec *pVideoCodec = nullptr;
  if (ffmpeg_prefer_libvpx) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 0, 0)
    if (codecpar->codec_id == AV_CODEC_ID_VP9) {
      pVideoCodec = avcodec_find_decoder_by_name("libvpx-vp9");
    } else
#endif
    if (codecpar->codec_id == AV_CODEC_ID_VP8) {
      pVideoCodec = avcodec_find_decoder_by_name("libvpx");
    }
  }
  if (pVideoCodec == nullptr) {
    pVideoCodec = avcodec_find_decoder(codecpar->codec_id);
  }
  if (pVideoCodec == nullptr) {
    ffmpeg_cat.info()
      << "Couldn't find codec\n";
    close_stream();
    return false;
  }

  _video_ctx = avcodec_alloc_context3(pVideoCodec);

  if (_video_ctx == nullptr) {
    ffmpeg_cat.info()
      << "Couldn't allocate _video_ctx\n";
    close_stream();
    return false;
  }

#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 41, 100)
  avcodec_parameters_to_context(_video_ctx, codecpar);
#else
  avcodec_copy_context(_video_ctx, codecpar);
#endif

  if (avcodec_open2(_video_ctx, pVideoCodec, nullptr) < 0) {
    ffmpeg_cat.info()
      << "Couldn't open codec\n";
    close_stream();
    return false;
  }

  _size_x = _video_ctx->width;
  _size_y = _video_ctx->height;
  _num_components = 3;
  _length = (double)_format_ctx->duration / (double)AV_TIME_BASE;
  _can_seek = true;
  _can_seek_fast = true;

  return true;
}

/**
 * Closes the stream, during cleanup or when needed internally.
 */
void FfmpegVideoCursor::
close_stream() {
  // Hold the global lock while we free avcodec objects.
  ReMutexHolder av_holder(_av_lock);

  if (_video_ctx && _video_ctx->codec) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 37, 100)
    // We need to drain the codec to prevent a memory leak.
    avcodec_send_packet(_video_ctx, nullptr);
    while (avcodec_receive_frame(_video_ctx, _frame) == 0) {}
    avcodec_flush_buffers(_video_ctx);
#endif

    avcodec_close(_video_ctx);
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 52, 0)
    avcodec_free_context(&_video_ctx);
#else
    delete _video_ctx;
#endif
  }
  _video_ctx = nullptr;

  _ffvfile.close();
  _format_ctx = nullptr;

  _video_index = -1;
}

/**
 * Reset to a standard inactive state.
 */
void FfmpegVideoCursor::
cleanup() {
  stop_thread();
  close_stream();

  ReMutexHolder av_holder(_av_lock);

#ifdef HAVE_SWSCALE
  if (_convert_ctx != nullptr) {
    sws_freeContext(_convert_ctx);
  }
  _convert_ctx = nullptr;
#endif  // HAVE_SWSCALE

  if (_frame) {
    av_free(_frame);
    _frame = nullptr;
  }

  if (_frame_out) {
    _frame_out->data[0] = nullptr;
    av_free(_frame_out);
    _frame_out = nullptr;
  }

  if (_packet) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_free(&_packet);
#else
    if (_packet->data) {
      av_free_packet(_packet);
    }
    delete _packet;
    _packet = nullptr;
#endif
  }
}

/**
 * The thread main function, static version (for passing to GenericThread).
 */
void FfmpegVideoCursor::
st_thread_main(void *self) {
  ((FfmpegVideoCursor *)self)->thread_main();
}

/**
 * The thread main function.
 */
void FfmpegVideoCursor::
thread_main() {
  if (ffmpeg_cat.is_spam()) {
    ffmpeg_cat.spam()
      << "ffmpeg thread for " << _filename.get_basename() << " starting.\n";
  }

  // First, push the first frame onto the readahead queue.
  if (_frame_ready) {
    PT(FfmpegBuffer) frame = do_alloc_frame();
    export_frame(frame);
    MutexHolder holder(_lock);
    _readahead_frames.push_back(frame);
  }

  // Now repeatedly wait for something interesting to do, until we're told to
  // shut down.
  MutexHolder holder(_lock);
  while (_thread_status != TS_shutdown) {
    nassertv(_thread_status != TS_stopped);
    _action_cvar.wait();

    while (do_poll()) {
      // Keep doing stuff as long as there's something to do.
      _lock.release();
      PStatClient::thread_tick(_sync_name);
      Thread::consider_yield();
      _lock.acquire();
    }
  }

  _thread_status = TS_stopped;
  if (ffmpeg_cat.is_spam()) {
    ffmpeg_cat.spam()
      << "ffmpeg thread for " << _filename.get_basename() << " stopped.\n";
  }
}

/**
 * Called within the sub-thread.  Assumes the lock is already held.  If there
 * is something for the thread to do, does it and returns true.  If there is
 * nothing for the thread to do, returns false.
 */
bool FfmpegVideoCursor::
do_poll() {
  switch (_thread_status) {
  case TS_stopped:
  case TS_seeking:
    // This shouldn't be possible while the thread is running.
    nassertr(false, false);
    return false;

  case TS_wait:
    // The video hasn't started playing yet.
    return false;

  case TS_readahead:
    if ((int)_readahead_frames.size() < _max_readahead_frames) {
      // Time to read the next frame.
      PT(FfmpegBuffer) frame = do_alloc_frame();
      nassertr(frame != nullptr, false);
      _lock.release();
      fetch_frame(-1);
      if (_frame_ready) {
        export_frame(frame);
        _lock.acquire();
        _readahead_frames.push_back(frame);
      } else {
        // No frame.
        _lock.acquire();
      }
      return true;
    }

    // No room for the next frame yet.  Wait for more.
    return false;

  case TS_seek:
    // Seek to a specific frame.
    {
      int seek_frame = _seek_frame;
      _thread_status = TS_seeking;
      PT(FfmpegBuffer) frame = do_alloc_frame();
      nassertr(frame != nullptr, false);
      _lock.release();
      if (seek_frame != _begin_frame) {
        advance_to_frame(seek_frame);
      }
      if (_frame_ready) {
        export_frame(frame);
        _lock.acquire();
        do_clear_all_frames();
        _readahead_frames.push_back(frame);
      } else {
        _lock.acquire();
        do_clear_all_frames();
      }

      if (_thread_status == TS_seeking) {
        // After seeking, we automatically transition to readahead.
        _thread_status = TS_readahead;
      }
    }
    return true;

  case TS_shutdown:
    // Time to stop the thread.
    return false;
  }

  return false;
}

/**
 * Allocates a new Buffer object.  Assumes the lock is held.
 */
PT(FfmpegVideoCursor::FfmpegBuffer) FfmpegVideoCursor::
do_alloc_frame() {
  PT(Buffer) buffer = make_new_buffer();
  return (FfmpegBuffer *)buffer.p();
}

/**
 * Empties the entire readahead_frames queue.  Assumes the lock is held.
 */
void FfmpegVideoCursor::
do_clear_all_frames() {
  _readahead_frames.clear();
}

/**
 * Called within the sub-thread.  Fetches a video packet and stores it in the
 * packet0 buffer.  Sets packet_frame to the packet's timestamp.  If a packet
 * could not be read, the packet is cleared and the packet_frame is set to the
 * specified default value.  Returns true on failure (such as the end of the
 * video), or false on success.
 */
bool FfmpegVideoCursor::
fetch_packet(int default_frame) {
  if (ffmpeg_global_lock) {
    ReMutexHolder av_holder(_av_lock);
    return do_fetch_packet(default_frame);
  } else {
    return do_fetch_packet(default_frame);
  }
}

/**
 * As above, with the ffmpeg global lock held (if configured on).
 */
bool FfmpegVideoCursor::
do_fetch_packet(int default_frame) {
  if (_packet->data) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_unref(_packet);
#else
    av_free_packet(_packet);
#endif
  }
  while (av_read_frame(_format_ctx, _packet) >= 0) {
    if (_packet->stream_index == _video_index) {
      _packet_frame = _packet->dts;
      return false;
    }
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 12, 100)
    av_packet_unref(_packet);
#else
    av_free_packet(_packet);
#endif
  }
  _packet->data = nullptr;

  if (!_eof_known && default_frame != 0) {
    _eof_frame = _packet_frame;
    _eof_known = true;
  }

  if (ffmpeg_cat.is_spam()) {
    if (_eof_known) {
      ffmpeg_cat.spam()
        << "end of video at frame " << _eof_frame << "\n";
    } else {
      ffmpeg_cat.spam()
        << "end of video\n";
    }
  }
  _packet_frame = default_frame;
  return true;
}

/**
 * Called within the sub-thread.  Slides forward until the indicated frame,
 * then fetches a frame from the stream and stores it in the frame buffer.
 * Sets _begin_frame and _end_frame to indicate the extents of the frame.
 * Sets _frame_ready true to indicate a frame is now available, or false if it
 * is not (for instance, because the end of the video was reached).
 */
void FfmpegVideoCursor::
fetch_frame(int frame) {
  PStatTimer timer(_fetch_buffer_pcollector);

  int finished = 0;

  if (_packet_frame <= frame) {
    finished = 0;

    // Get the next packet.  The first packet beyond the frame we're looking
    // for marks the point to stop.
    while (_packet_frame <= frame) {
      PStatTimer timer(_seek_pcollector);

      // Decode the previous packet, and get the next one.
      decode_frame(finished);
      _begin_frame = _packet_frame;
      if (fetch_packet(frame)) {
        _end_frame = _packet_frame;
        _frame_ready = false;
        return;
      }
    }

  } else {
    // Just get the next frame.
    finished = 0;
    while (!finished && _packet->data) {
      decode_frame(finished);
      _begin_frame = _packet_frame;
      fetch_packet(_begin_frame + 1);
    }
  }

  _end_frame = _packet_frame;
  _frame_ready = true;
}

/**
 * Called within the sub-thread.  Decodes the data in the specified packet
 * into _frame.
 */
void FfmpegVideoCursor::
decode_frame(int &finished) {
  if (ffmpeg_global_lock) {
    ReMutexHolder av_holder(_av_lock);
    do_decode_frame(finished);
  } else {
    do_decode_frame(finished);
  }
}

/**
 * As above, with the ffmpeg global lock held (if configured on).
 */
void FfmpegVideoCursor::
do_decode_frame(int &finished) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 37, 100)
  // While the audio cursor has a really nice async loop for decoding, we
  // don't really do that much with video since we're already delegated to
  // another thread here.  This is just to silence the deprecation warning
  // on avcodec_decode_video2.
  avcodec_send_packet(_video_ctx, _packet);

  int ret = avcodec_receive_frame(_video_ctx, _frame);
  finished = (ret == 0);
#else
  avcodec_decode_video2(_video_ctx, _frame, &finished, _packet);
#endif
}

/**
 * Called within the sub-thread.  Seeks to a target location.  Afterward, the
 * packet_frame is guaranteed to be less than or equal to the specified frame.
 */
void FfmpegVideoCursor::
seek(int frame, bool backward) {
  PStatTimer timer(_seek_pcollector);

  if (ffmpeg_support_seek) {
    if (ffmpeg_global_lock) {
      ReMutexHolder av_holder(_av_lock);
      do_seek(frame, backward);
    } else {
      do_seek(frame, backward);
    }

  } else {
    // If seeking isn't supported, close-and-reopen.
    if (backward) {
      reset_stream();
    }
  }
}

/**
 * As above, with the ffmpeg global lock held (if configured on).  Also only
 * if ffmpeg-support-seek is on.
 */
void FfmpegVideoCursor::
do_seek(int frame, bool backward) {
  int64_t target_ts = (int64_t)frame;
  if (target_ts < (int64_t)(_initial_dts)) {
    // Attempts to seek before the first packet will fail.
    target_ts = _initial_dts;
  }
  int flags = 0;
  if (backward) {
    flags = AVSEEK_FLAG_BACKWARD;
  }

  if (av_seek_frame(_format_ctx, _video_index, target_ts, flags) < 0) {
    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "Seek failure.\n";
    }

    if (backward) {
      // Now try to seek forward.
      reset_stream();
      seek(frame, false);
      return;
    }

    // Try a binary search to get a little closer.
    if (binary_seek(_initial_dts, frame, frame, 1) < 0) {
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Seek double failure.\n";
      }
      reset_stream();
      return;
    }
  }

  fetch_packet(0);
  fetch_frame(-1);
}

/**
 * Casts about within the stream for a reasonably-close frame to seek to.
 * We're trying to get as close as possible to target_frame.
 */
int FfmpegVideoCursor::
binary_seek(int min_frame, int max_frame, int target_frame, int num_iterations) {
  int try_frame = (min_frame + max_frame) / 2;
  if (num_iterations > 5 || try_frame >= max_frame) {
    // Success.
    return 0;
  }

  if (av_seek_frame(_format_ctx, _video_index, try_frame, AVSEEK_FLAG_BACKWARD) < 0) {
    // Failure.  Try lower.
    if (binary_seek(min_frame, try_frame - 1, target_frame, num_iterations + 1) < 0) {
      return -1;
    }
  } else {
    // Success.  Try higher.
    if (binary_seek(try_frame + 1, max_frame, target_frame, num_iterations + 1) < 0) {
      return -1;
    }
  }
  return 0;
}

/**
 * Resets the stream to its initial, first-opened state by closing and re-
 * opening it.
 */
void FfmpegVideoCursor::
reset_stream() {
  if (ffmpeg_cat.is_spam()) {
    ffmpeg_cat.spam()
      << "Resetting ffmpeg stream.\n";
  }

  close_stream();
  if (!open_stream()) {
    ffmpeg_cat.error()
      << "Stream error, invalidating movie.\n";
    cleanup();
    return;
  }

  fetch_packet(0);
  fetch_frame(-1);
}

/**
 * Called within the sub-thread.  Advance until the specified frame is in the
 * export buffer.
 */
void FfmpegVideoCursor::
advance_to_frame(int frame) {
  PStatTimer timer(_fetch_buffer_pcollector);

  if (frame < _begin_frame) {
    // Frame is in the past.
    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "Seeking backward to " << frame << " from " << _begin_frame << "\n";
    }
    seek(frame, true);
    if (_begin_frame > frame) {
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Ended up at " << _begin_frame << ", not far enough back!\n";
      }
      reset_stream();
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Reseek to 0, got " << _begin_frame << "\n";
      }
    }
    if (frame > _end_frame) {
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Now sliding forward to " << frame << " from " << _begin_frame << "\n";
      }
      fetch_frame(frame);
    }

  } else if (frame < _end_frame) {
    // Frame is in the present: already have the frame.
    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "Currently have " << frame << " within " << _begin_frame << " .. " << _end_frame << "\n";
    }

  } else if (frame < _end_frame + _min_fseek) {
    // Frame is in the near future.
    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "Sliding forward to " << frame << " from " << _begin_frame << "\n";
    }
    fetch_frame(frame);

  } else {
    // Frame is in the far future.  Seek forward, then read.  There's a danger
    // here: because keyframes are spaced unpredictably, trying to seek
    // forward could actually move us backward in the stream!  This must be
    // avoided.  So the rule is, try the seek.  If it hurts us by moving us
    // backward, we increase the minimum threshold distance for forward-
    // seeking in the future.

    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "Jumping forward to " << frame << " from " << _begin_frame << "\n";
    }
    int base = _begin_frame;
    seek(frame, false);
    if (_begin_frame < base) {
      _min_fseek += (base - _begin_frame);
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Wrong way!  Increasing _min_fseek to " << _min_fseek << "\n";
      }
    }
    if (frame > _end_frame) {
      if (ffmpeg_cat.is_spam()) {
        ffmpeg_cat.spam()
          << "Correcting, sliding forward to " << frame << " from " << _begin_frame << "\n";
      }
      fetch_frame(frame);
    }
  }

  if (ffmpeg_cat.is_spam()) {
    ffmpeg_cat.spam()
      << "Wanted " << frame << ", got " << _begin_frame << "\n";
  }
}

/**
 * Called within the sub-thread.  Exports the contents of the frame buffer
 * into the indicated target buffer.
 */
void FfmpegVideoCursor::
export_frame(FfmpegBuffer *buffer) {
  PStatTimer timer(_export_frame_pcollector);

  if (!_frame_ready) {
    // No frame data ready, just fill with black.
    if (ffmpeg_cat.is_spam()) {
      ffmpeg_cat.spam()
        << "ffmpeg for " << _filename.get_basename()
        << ", no frame available.\n";
    }
    memset(buffer->_block, 0, buffer->_block_size);
    return;
  }

  _frame_out->data[0] = buffer->_block + ((_size_y - 1) * _size_x * _num_components);
  _frame_out->linesize[0] = _size_x * -_num_components;
  buffer->_begin_frame = _begin_frame;
  buffer->_end_frame = _end_frame;

#ifdef HAVE_SWSCALE
  nassertv(_convert_ctx != nullptr && _frame != nullptr);
  if (ffmpeg_global_lock) {
    ReMutexHolder av_holder(_av_lock);
    sws_scale(_convert_ctx, _frame->data, _frame->linesize, 0, _size_y, _frame_out->data, _frame_out->linesize);
  } else {
    sws_scale(_convert_ctx, _frame->data, _frame->linesize, 0, _size_y, _frame_out->data, _frame_out->linesize);
  }
#else
  nassertv(_frame != nullptr);
  uint8_t const *src = _frame->data[0];
  uint8_t *dst = _frame_out->data[0];
  int src_stride = _frame->linesize[0];
  int dst_stride = _frame_out->linesize[0];
  size_t copy_size = _size_x * _num_components;
  nassertv(copy_size <= (size_t)std::abs(src_stride));

  for (int y = 0; y < _size_y; ++y) {
    memcpy(dst, src, copy_size);
    src += src_stride;
    dst += dst_stride;
  }
#endif
}

/**
 * Tells the BamReader how to create objects of type FfmpegVideo.
 */
void FfmpegVideoCursor::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void FfmpegVideoCursor::
write_datagram(BamWriter *manager, Datagram &dg) {
  MovieVideoCursor::write_datagram(manager, dg);

  // No need to write any additional data here--all of it comes implicitly
  // from the underlying MovieVideo, which we process in finalize().
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void FfmpegVideoCursor::
finalize(BamReader *) {
  if (_source != nullptr) {
    FfmpegVideo *video;
    DCAST_INTO_V(video, _source);
    init_from(video);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type FfmpegVideo is encountered in the Bam file.  It should create the
 * FfmpegVideo and extract its information from the file.
 */
TypedWritable *FfmpegVideoCursor::
make_from_bam(const FactoryParams &params) {
  FfmpegVideoCursor *video = new FfmpegVideoCursor;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  video->fillin(scan, manager);

  return video;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new FfmpegVideo.
 */
void FfmpegVideoCursor::
fillin(DatagramIterator &scan, BamReader *manager) {
  MovieVideoCursor::fillin(scan, manager);

  // The MovieVideoCursor gets the underlying MovieVideo pointer.  We need a
  // finalize callback so we can initialize ourselves once that has been read
  // completely.
  manager->register_finalize(this);
}

/**
 * Used to sort different buffers to ensure they correspond to the same source
 * frame, particularly important when synchronizing the different pages of a
 * multi-page texture.
 *
 * Returns 0 if the two buffers are of the same frame, <0 if this one comes
 * earlier than the other one, and >0 if the other one comes earlier.
 */
int FfmpegVideoCursor::FfmpegBuffer::
compare_timestamp(const Buffer *other) const {
  const FfmpegBuffer *fother;
  DCAST_INTO_R(fother, other, 0);
  if (_end_frame * _video_timebase <= fother->_begin_frame * fother->_video_timebase) {
    return -1;
  } else if (_begin_frame * _video_timebase >= fother->_end_frame * fother->_video_timebase) {
    return 1;
  }
  return 0;
}

/**
 * Returns the nearest timestamp value of this particular buffer.  Ideally,
 * MovieVideoCursor::set_time() for this timestamp would return this buffer
 * again.  This need be defined only if compare_timestamp() is also defined.
 */
double FfmpegVideoCursor::FfmpegBuffer::
get_timestamp() const {
  int mid_frame = (_begin_frame + _end_frame - 1) / 2;
  return mid_frame * _video_timebase;
}
