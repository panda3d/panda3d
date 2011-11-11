// Filename: ffmpegVideoCursor.cxx
// Created by: jyelon (01Aug2007)
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

#ifdef HAVE_FFMPEG

#include "ffmpegVideoCursor.h"
#include "config_movies.h"
extern "C" {
  #include "libavcodec/avcodec.h"
  #include "libavformat/avformat.h"
#ifdef HAVE_SWSCALE
  #include "libswscale/swscale.h"
#endif
}
#include "pStatCollector.h"
#include "pStatTimer.h"
#include "mutexHolder.h"
#include "reMutexHolder.h"

ReMutex FfmpegVideoCursor::_av_lock;
TypeHandle FfmpegVideoCursor::_type_handle;


#if LIBAVFORMAT_VERSION_MAJOR < 53
  #define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#endif

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Default Constructor
//       Access: Private
//  Description: This constructor is only used when reading from a bam
//               file.
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
FfmpegVideoCursor() :
  _max_readahead_frames(0),
  _thread_priority(ffmpeg_thread_priority),
  _lock("FfmpegVideoCursor::_lock"),
  _action_cvar(_lock),
  _thread_status(TS_stopped),
  _seek_time(0.0),
  _packet0(NULL),
  _packet1(NULL),
  _format_ctx(NULL),
  _video_ctx(NULL),
  _convert_ctx(NULL),
  _video_index(-1),
  _frame(NULL),
  _frame_out(NULL),
  _min_fseek(3.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::init_from
//       Access: Private
//  Description: Specifies the source of the video cursor.  This is
//               normally called only by the constructor or when
//               reading from a bam file.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
init_from(FfmpegVideo *source) {
  nassertv(_thread == NULL && _thread_status == TS_stopped);
  nassertv(source != NULL);
  _source = source;
  _filename = _source->get_filename();

  // Hold the global lock while we open the file and create avcodec
  // objects.
  ReMutexHolder av_holder(_av_lock);

  if (!_source->get_subfile_info().is_empty()) {
    // Read a subfile.
    if (!_ffvfile.open_subfile(_source->get_subfile_info())) {
      movies_cat.info() 
        << "Couldn't open " << _source->get_subfile_info() << "\n";
      cleanup();
      return;
    }

  } else {
    // Read a filename.
    if (!_ffvfile.open_vfs(_filename)) {
      movies_cat.info() 
        << "Couldn't open " << _filename << "\n";
      cleanup();
      return;
    }
  }

  _format_ctx = _ffvfile.get_format_context();
  nassertv(_format_ctx != NULL);

  if (av_find_stream_info(_format_ctx) < 0) {
    movies_cat.info() 
      << "Couldn't find stream info\n";
    cleanup();
    return;
  }
  
  // Find the video stream
  for (int i = 0; i < (int)_format_ctx->nb_streams; ++i) {
    if (_format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      _video_index = i;
      _video_ctx = _format_ctx->streams[i]->codec;
      _video_timebase = av_q2d(_format_ctx->streams[i]->time_base);
    }
  }
  
  if (_video_ctx == 0) {
    movies_cat.info() 
      << "Couldn't find video_ctx\n";
    cleanup();
    return;
  }

  AVCodec *pVideoCodec = avcodec_find_decoder(_video_ctx->codec_id);
  if (pVideoCodec == NULL) {
    movies_cat.info() 
      << "Couldn't find codec\n";
    cleanup();
    return;
  }
  if (avcodec_open(_video_ctx, pVideoCodec) < 0) {
    movies_cat.info() 
      << "Couldn't open codec\n";
    cleanup();
    return;
  }
  
  _size_x = _video_ctx->width;
  _size_y = _video_ctx->height;
  _num_components = 3; // Don't know how to implement RGBA movies yet.
  _length = (_format_ctx->duration * 1.0) / AV_TIME_BASE;
  _can_seek = true;
  _can_seek_fast = true;
  
#ifdef HAVE_SWSCALE
  _convert_ctx = sws_getContext(_size_x, _size_y,
                                _video_ctx->pix_fmt, _size_x, _size_y,
                                PIX_FMT_BGR24, SWS_BILINEAR | SWS_PRINT_INFO, NULL, NULL, NULL);
#endif  // HAVE_SWSCALE

  _frame = avcodec_alloc_frame();
  _frame_out = avcodec_alloc_frame();

  if ((_frame == 0)||(_frame_out == 0)) {
    cleanup();
    return;
  }

  _packet0 = new AVPacket;
  _packet1 = new AVPacket;
  memset(_packet0, 0, sizeof(AVPacket));
  memset(_packet1, 0, sizeof(AVPacket));
  
  fetch_packet(0.0);
  _initial_dts = _packet0->dts;
  fetch_frame(-1);

#ifdef HAVE_THREADS
  set_max_readahead_frames(ffmpeg_max_readahead_frames);
#endif  // HAVE_THREADS
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
FfmpegVideoCursor(FfmpegVideo *src) : 
  _max_readahead_frames(0),
  _thread_priority(ffmpeg_thread_priority),
  _lock("FfmpegVideoCursor::_lock"),
  _action_cvar(_lock),
  _thread_status(TS_stopped),
  _seek_time(0.0),
  _packet0(NULL),
  _packet1(NULL),
  _format_ctx(NULL),
  _video_ctx(NULL),
  _convert_ctx(NULL),
  _video_index(-1),
  _frame(NULL),
  _frame_out(NULL),
  _min_fseek(3.0)
{
  init_from(src);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
~FfmpegVideoCursor() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::set_max_readahead_frames
//       Access: Published
//  Description: Specifies the maximum number of frames that a
//               sub-thread will attempt to read ahead of the current
//               frame.  Setting this to a nonzero allows the video
//               decoding to take place in a sub-thread, which
//               smoothes out the video decoding time by spreading it
//               evenly over several frames.  Set this number larger
//               to increase the buffer between the currently visible
//               frame and the first undecoded frame; set it smaller
//               to reduce memory consumption.
//
//               Setting this to zero forces the video to be decoded
//               in the main thread.  If threading is not available in
//               the Panda build, this value is always zero.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::get_max_readahead_frames
//       Access: Published
//  Description: Returns the maximum number of frames that a
//               sub-thread will attempt to read ahead of the current
//               frame.  See set_max_readahead_frames().
////////////////////////////////////////////////////////////////////
int FfmpegVideoCursor::
get_max_readahead_frames() const {
  return _max_readahead_frames;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::set_thread_priority
//       Access: Published
//  Description: Changes the thread priority of the thread that
//               decodes the ffmpeg video stream (if
//               max_readahead_frames is nonzero).  Normally you
//               shouldn't mess with this, but there may be special
//               cases where a precise balance of CPU utilization
//               between the main thread and the various ffmpeg
//               service threads may be needed.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::get_thread_priority
//       Access: Published
//  Description: Returns the current thread priority of the thread that
//               decodes the ffmpeg video stream (if
//               max_readahead_frames is nonzero).  See
//               set_thread_priority().
////////////////////////////////////////////////////////////////////
ThreadPriority FfmpegVideoCursor::
get_thread_priority() const {
  return _thread_priority;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::start_thread
//       Access: Published
//  Description: Explicitly starts the ffmpeg decoding thread after it
//               has been stopped by a call to stop_thread().  The
//               thread is normally started automatically, so there is
//               no need to call this method unless you have
//               previously called stop_thread() for some reason.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
start_thread() {
  MutexHolder holder(_lock);

  if (_thread_status == TS_stopped && _max_readahead_frames > 0) {
    // Get a unique name for the thread's sync name.
    ostringstream strm;
    strm << (void *)this;
    _sync_name = strm.str();

    // Create and start the thread object.
    _thread_status = TS_wait;
    _thread = new GenericThread(_filename.get_basename(), _sync_name, st_thread_main, this);
    if (!_thread->start(_thread_priority, true)) {
      // Couldn't start the thread.
      _thread = NULL;
      _thread_status = TS_stopped;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::stop_thread
//       Access: Published
//  Description: Explicitly stops the ffmpeg decoding thread.  There
//               is normally no reason to do this unless you want to
//               maintain precise control over what threads are
//               consuming CPU resources.  Calling this method will
//               make the video update in the main thread, regardless
//               of the setting of max_readahead_frames, until you
//               call start_thread() again.
////////////////////////////////////////////////////////////////////
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
      _thread = NULL;
    }

    // Now that we've released the lock, we can join the thread.
    thread->join();
  }

  // This is a good time to clean up all of the allocated frame
  // objects.  It's not really necessary to be holding the lock, since
  // the thread is gone, but we'll grab it anyway just in case someone
  // else starts the thread up again.
  MutexHolder holder(_lock);

  Buffers::iterator bi;
  for (bi = _readahead_frames.begin(); bi != _readahead_frames.end(); ++bi) {
    internal_free_buffer(*bi);
  }
  _readahead_frames.clear();
  for (bi = _recycled_frames.begin(); bi != _recycled_frames.end(); ++bi) {
    internal_free_buffer(*bi);
  }
  _recycled_frames.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::is_thread_started
//       Access: Published
//  Description: Returns true if the thread has been started, false if
//               not.  This will always return false if
//               max_readahead_frames is 0.
////////////////////////////////////////////////////////////////////
bool FfmpegVideoCursor::
is_thread_started() const {
  return (_thread_status != TS_stopped);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_buffer
//       Access: Public, Virtual
//  Description: See MovieVideoCursor::fetch_buffer.
////////////////////////////////////////////////////////////////////
MovieVideoCursor::Buffer *FfmpegVideoCursor::
fetch_buffer(double time) {
  MutexHolder holder(_lock);
  
  // If there was an error at any point, just return NULL.
  if (_format_ctx == (AVFormatContext *)NULL) {
    return NULL;
  }

  Buffer *frame = NULL;
  if (_thread_status == TS_stopped) {
    // Non-threaded case.  Just get the next frame directly.
    fetch_time(time);
    if (_frame_ready) {
      frame = do_alloc_frame();
      export_frame(frame);
    }

  } else {
    // Threaded case.  Wait for the thread to serve up the required
    // frames.
    if (!_readahead_frames.empty()) {
      frame = _readahead_frames.front();
      _readahead_frames.pop_front();
      _action_cvar.notify();
      while (frame->_end_time < time && !_readahead_frames.empty()) {
        // This frame is too old.  Discard it.
        if (ffmpeg_cat.is_debug()) {
          ffmpeg_cat.debug()
            << "ffmpeg for " << _filename.get_basename()
            << " at time " << time << ", discarding frame at "
            << frame->_begin_time << "\n";
        }
        do_recycle_frame(frame);
        frame = _readahead_frames.front();
        _readahead_frames.pop_front();
      }
      if (frame->_begin_time > time) {
        // This frame is too new.  Empty all remaining frames and seek
        // backwards.
        do_recycle_all_frames();
        if (_thread_status == TS_wait || _thread_status == TS_seek || _thread_status == TS_readahead) {
          _thread_status = TS_seek;
          _seek_time = time;
          _action_cvar.notify();
        }
      }
    }
    if (frame == NULL || frame->_end_time < time) {
      // No frame available, or the frame is too old.  Seek.
      if (_thread_status == TS_wait || _thread_status == TS_seek || _thread_status == TS_readahead) {
        _thread_status = TS_seek;
        _seek_time = time;
        _action_cvar.notify();
      }
    }
  }

  if (frame != NULL && (frame->_end_time < time || frame->_begin_time > time)) {
    // The frame is too old or too new.  Just recycle it.
    do_recycle_frame(frame);
    frame = NULL;
  }
  
  return frame;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::release_buffer
//       Access: Public, Virtual
//  Description: Should be called after processing the Buffer object
//               returned by fetch_buffer(), this releases the Buffer
//               for future use again.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
release_buffer(Buffer *buffer) {
  MutexHolder holder(_lock);
  do_recycle_frame(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::cleanup
//       Access: Private
//  Description: Reset to a standard inactive state.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
cleanup() {
  stop_thread();
  
  // Hold the global lock while we free avcodec objects.
  ReMutexHolder av_holder(_av_lock);

  if (_frame) {
    av_free(_frame);
    _frame = NULL;
  }

  if (_frame_out) {
    _frame_out->data[0] = 0;
    av_free(_frame_out);
    _frame_out = NULL;
  }

  if (_packet0) {
    if (_packet0->data) {
      av_free_packet(_packet0);
    }
    delete _packet0;
    _packet0 = NULL;
  }

  if (_packet1) {
    if (_packet1->data) {
      av_free_packet(_packet1);
    }
    delete _packet1;
    _packet1 = NULL;
  }
  
#ifdef HAVE_SWSCALE
  if (_convert_ctx != NULL) {
    sws_freeContext(_convert_ctx);
  }
  _convert_ctx = NULL;
#endif  // HAVE_SWSCALE
  
  if ((_video_ctx)&&(_video_ctx->codec)) {
    avcodec_close(_video_ctx);
  }
  _video_ctx = NULL;
  
  if (_format_ctx) {
    _ffvfile.close();
    _format_ctx = NULL;
  }

  _video_index = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::st_thread_main
//       Access: Private, Static
//  Description: The thread main function, static version (for passing
//               to GenericThread).
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
st_thread_main(void *self) {
  ((FfmpegVideoCursor *)self)->thread_main();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::thread_main
//       Access: Private
//  Description: The thread main function.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
thread_main() {
  MutexHolder holder(_lock);
  if (ffmpeg_cat.is_debug()) {
    ffmpeg_cat.debug()
      << "ffmpeg thread for " << _filename.get_basename() << " starting.\n";
  }
  
  // Repeatedly wait for something interesting to do, until we're told
  // to shut down.
  while (_thread_status != TS_shutdown) {
    nassertv(_thread_status != TS_stopped);
    _action_cvar.wait();

    while (do_poll()) {
      // Keep doing stuff as long as there's something to do.
      PStatClient::thread_tick(_sync_name);
      Thread::consider_yield();
    }
  }

  _thread_status = TS_stopped;
  if (ffmpeg_cat.is_debug()) {
    ffmpeg_cat.debug()
      << "ffmpeg thread for " << _filename.get_basename() << " stopped.\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::do_poll
//       Access: Private
//  Description: Called within the sub-thread.  Assumes the lock is
//               already held.  If there is something for the thread
//               to do, does it and returns true.  If there is nothing
//               for the thread to do, returns false.
////////////////////////////////////////////////////////////////////
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
      Buffer *frame = do_alloc_frame();
      nassertr(frame != NULL, false);
      _lock.release();
      fetch_frame(-1);
      if (_frame_ready) {
        export_frame(frame);
        _lock.acquire();
        _readahead_frames.push_back(frame);
      } else {
        // No frame.
        _lock.acquire();
        do_recycle_frame(frame);
      }
      return true;
    }

    // No room for the next frame yet.  Wait for more.
    return false;

  case TS_seek:
    // Seek to a specific time.
    {
      double seek_time = _seek_time;
      _thread_status = TS_seeking;
      Buffer *frame = do_alloc_frame();
      nassertr(frame != NULL, false);
      _lock.release();
      fetch_time(seek_time);
      if (_frame_ready) {
        export_frame(frame);
        _lock.acquire();
        do_recycle_all_frames();
        _readahead_frames.push_back(frame);
      } else {
        _lock.acquire();
        do_recycle_all_frames();
        do_recycle_frame(frame);
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

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::do_alloc_frame
//       Access: Private
//  Description: Allocates a new Buffer object, or returns a
//               previously-recycled object.  Assumes the lock is
//               held.
////////////////////////////////////////////////////////////////////
MovieVideoCursor::Buffer *FfmpegVideoCursor::
do_alloc_frame() {
  if (!_recycled_frames.empty()) {
    Buffer *frame = _recycled_frames.front();
    _recycled_frames.pop_front();
    return frame;
  }
  return internal_alloc_buffer();
}
 
////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::do_recycle_frame
//       Access: Private
//  Description: Recycles a previously-allocated Buffer object for
//               future reuse.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
do_recycle_frame(Buffer *frame) {
  _recycled_frames.push_back(frame);
}
 
////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::do_recycle_all_frames
//       Access: Private
//  Description: Empties the entire readahead_frames queue into the
//               recycle bin.  Assumes the lock is held.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
do_recycle_all_frames() {
  while (!_readahead_frames.empty()) {
    Buffer *frame = _readahead_frames.front();
    _readahead_frames.pop_front();
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "ffmpeg for " << _filename.get_basename()
        << " recycling frame at " << frame->_begin_time << "\n";
    }
    _recycled_frames.push_back(frame);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_packet
//       Access: Private
//  Description: Called within the sub-thread.  Fetches a video packet
//               and stores it in the packet0 buffer.  Sets packet_time
//               to the packet's timestamp.  If a packet could not be
//               read, the packet is cleared and the packet_time is
//               set to the specified default value.  Returns true on
//               failure (such as the end of the video), or false on
//               success.
////////////////////////////////////////////////////////////////////
bool FfmpegVideoCursor::
fetch_packet(double default_time) {
  if (_packet0->data) {
    av_free_packet(_packet0);
  }
  while (av_read_frame(_format_ctx, _packet0) >= 0) {
    if (_packet0->stream_index == _video_index) {
      _packet_time = _packet0->dts * _video_timebase;
      return false;
    }
    av_free_packet(_packet0);
  }
  _packet0->data = 0;
  _packet_time = default_time;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::flip_packets
//       Access: Private
//  Description: Called within the sub-thread.  Reverses _packet0 and _packet1.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
flip_packets() {
  AVPacket *t = _packet0;
  _packet0 = _packet1;
  _packet1 = t;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_frame
//       Access: Private
//  Description: Called within the sub-thread.  Slides forward until
//               the indicated time, then fetches a frame from the
//               stream and stores it in the frame buffer.  Sets
//               last_start and next_start to indicate the extents of
//               the frame.  Returns true if the end of the video is
//               reached.
////////////////////////////////////////////////////////////////////
bool FfmpegVideoCursor::
fetch_frame(double time) {
  static PStatCollector fetch_buffer_pcollector("*:FFMPEG Video Decoding:Fetch");
  PStatTimer timer(fetch_buffer_pcollector);

  int finished = 0;
  _begin_time = _packet_time;

  if (_packet_time <= time) {
    static PStatCollector seek_pcollector("*:FFMPEG Video Decoding:Seek");
    PStatTimer timer(seek_pcollector);

    _video_ctx->skip_frame = AVDISCARD_BIDIR;
    // Put the current packet aside in case we discover it's the
    // packet to keep.
    flip_packets();
    
    // Get the next packet.  The first packet beyond the time we're
    // looking for marks the point to stop.
    if (fetch_packet(time)) {
      if (ffmpeg_cat.is_debug()) {
        ffmpeg_cat.debug()
          << "end of video\n";
      }
      _frame_ready = false;
      return true;
    }
    while (_packet_time <= time) {
      // Decode and discard the previous packet.
#if LIBAVCODEC_VERSION_INT < 3414272
      avcodec_decode_video(_video_ctx, _frame,
                           &finished, _packet1->data, _packet1->size);
#else
      avcodec_decode_video2(_video_ctx, _frame, &finished, _packet1);
#endif
      flip_packets();
      if (fetch_packet(time)) {
        if (ffmpeg_cat.is_debug()) {
          ffmpeg_cat.debug()
            << "end of video\n";
        }
        _frame_ready = false;
        return true;
      }
    }
    _video_ctx->skip_frame = AVDISCARD_DEFAULT;

    // At this point, _packet0 contains the *next* packet to be
    // decoded next frame, and _packet1 contains the packet to decode
    // for this frame.
#if LIBAVCODEC_VERSION_INT < 3414272
    avcodec_decode_video(_video_ctx, _frame,
                         &finished, _packet1->data, _packet1->size);
#else
    avcodec_decode_video2(_video_ctx, _frame, &finished, _packet1);
#endif
    
  } else {
    // Just get the next frame.
    finished = 0;
    while (!finished && _packet0->data) {
#if LIBAVCODEC_VERSION_INT < 3414272
      avcodec_decode_video(_video_ctx, _frame,
                           &finished, _packet0->data, _packet0->size);
#else
      avcodec_decode_video2(_video_ctx, _frame, &finished, _packet0);
#endif
      fetch_packet(_begin_time + 1.0);
    }
  }

  _end_time = _packet_time;
  _frame_ready = true;
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::seek
//       Access: Private
//  Description: Called within the sub-thread. Seeks to a target
//               location.  Afterward, the packet_time is guaranteed
//               to be less than or equal to the specified time.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
seek(double t) {
  static PStatCollector seek_pcollector("*:FFMPEG Video Decoding:Seek");
  PStatTimer timer(seek_pcollector);

  PN_int64 target_ts = (PN_int64)(t / _video_timebase);
  if (target_ts < (PN_int64)(_initial_dts)) {
    // Attempts to seek before the first packet will fail.
    target_ts = _initial_dts;
  }
  if (av_seek_frame(_format_ctx, _video_index, target_ts, AVSEEK_FLAG_BACKWARD) < 0) {
    if (t >= _packet_time) {
      return;
    }
    movies_cat.error() << "Seek failure. Shutting down movie.\n";
    cleanup();
    _packet_time = t;
    return;
  }
  {
    // Hold the global lock while we close and re-open the video
    // stream.
    ReMutexHolder av_holder(_av_lock);

    avcodec_close(_video_ctx);
    AVCodec *pVideoCodec = avcodec_find_decoder(_video_ctx->codec_id);
    if (pVideoCodec == 0) {
      cleanup();
      return;
    }

    if (avcodec_open(_video_ctx, pVideoCodec)<0) {
      cleanup();
      return;
    }
  }

  fetch_packet(t);
  fetch_frame(-1);
  /*
  if (_packet_time > t) {
    _packet_time = t;
  }
  */
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_time
//       Access: Private 
//  Description: Called within the sub-thread.  Advance until the
//               specified time is in the export buffer.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
fetch_time(double time) {
  static PStatCollector fetch_buffer_pcollector("*:FFMPEG Video Decoding:Fetch");
  PStatTimer timer(fetch_buffer_pcollector);

  if (time < _begin_time) {
    // Time is in the past.
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Seeking backward to " << time << " from " << _begin_time << "\n";
    }
    seek(time);
    if (_packet_time > time) {
      ffmpeg_cat.debug()
        << "Not far enough back!\n";
      seek(0);
    }
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Correcting, sliding forward to " << time << " from " << _packet_time << "\n";
    }
    fetch_frame(time);

  } else if (time < _end_time) {
    // Time is in the present: already have the frame.
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Currently have " << time << "\n";
    }

  } else if (time < _end_time + _min_fseek) {
    // Time is in the near future.
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Sliding forward to " << time << " from " << _packet_time << "\n";
    }
    fetch_frame(time);

  } else {
    // Time is in the far future.  Seek forward, then read.
    // There's a danger here: because keyframes are spaced
    // unpredictably, trying to seek forward could actually
    // move us backward in the stream!  This must be avoided.
    // So the rule is, try the seek.  If it hurts us by moving
    // us backward, we increase the minimum threshold distance
    // for forward-seeking in the future.
    
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Jumping forward to " << time << " from " << _begin_time << "\n";
    }
    double base = _packet_time;
    seek(time);
    if (_packet_time < base) {
      _min_fseek += (base - _packet_time);
      if (ffmpeg_cat.is_debug()) {
        ffmpeg_cat.debug()
          << "Wrong way!  Increasing _min_fseek to " << _min_fseek << "\n";
      }
    }
    if (ffmpeg_cat.is_debug()) {
      ffmpeg_cat.debug()
        << "Correcting, sliding forward to " << time << " from " << _packet_time << "\n";
    }
    fetch_frame(time);
  }

  if (ffmpeg_cat.is_debug()) {
    ffmpeg_cat.debug()
      << "Wanted " << time << ", got " << _packet_time << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::export_frame
//       Access: Private
//  Description: Called within the sub-thread.  Exports the contents
//               of the frame buffer into the indicated target buffer.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
export_frame(MovieVideoCursor::Buffer *buffer) {
  static PStatCollector export_frame_collector("*:FFMPEG Convert Video to BGR");
  PStatTimer timer(export_frame_collector);

  if (!_frame_ready) {
    // No frame data ready, just fill with black.
    memset(buffer->_block, 0, buffer->_block_size);
    return;
  }

  _frame_out->data[0] = buffer->_block + ((_size_y - 1) * _size_x * 3);
  _frame_out->linesize[0] = _size_x * -3;
  buffer->_begin_time = _begin_time;
  buffer->_end_time = _end_time;
#ifdef HAVE_SWSCALE
  nassertv(_convert_ctx != NULL && _frame != NULL && _frame_out != NULL);
  sws_scale(_convert_ctx, _frame->data, _frame->linesize, 0, _size_y, _frame_out->data, _frame_out->linesize);
#else
  img_convert((AVPicture *)_frame_out, PIX_FMT_BGR24, 
              (AVPicture *)_frame, _video_ctx->pix_fmt, _size_x, _size_y);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               FfmpegVideo.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
write_datagram(BamWriter *manager, Datagram &dg) {
  MovieVideoCursor::write_datagram(manager, dg);

  // No need to write any additional data here--all of it comes
  // implicitly from the underlying MovieVideo, which we process in
  // finalize().
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
finalize(BamReader *) {
  if (_source != (MovieVideo *)NULL) {
    FfmpegVideo *video;
    DCAST_INTO_V(video, _source);
    init_from(video);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::make_from_bam
//       Access: Private, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type FfmpegVideo is encountered
//               in the Bam file.  It should create the FfmpegVideo
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *FfmpegVideoCursor::
make_from_bam(const FactoryParams &params) {
  FfmpegVideoCursor *video = new FfmpegVideoCursor;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  video->fillin(scan, manager);

  return video;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fillin
//       Access: Private
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new FfmpegVideo.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
fillin(DatagramIterator &scan, BamReader *manager) {
  MovieVideoCursor::fillin(scan, manager);
  
  // The MovieVideoCursor gets the underlying MovieVideo pointer.  We
  // need a finalize callback so we can initialize ourselves once that
  // has been read completely.
  manager->register_finalize(this);
}

#endif // HAVE_FFMPEG
