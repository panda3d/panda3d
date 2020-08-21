/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ffmpegVideoCursor.h
 * @author jyelon
 * @date 2007-08-01
 */

#ifndef FFMPEGVIDEOCURSOR_H
#define FFMPEGVIDEOCURSOR_H

#include "pandabase.h"
#include "movieVideoCursor.h"
#include "texture.h"
#include "pointerTo.h"
#include "ffmpegVirtualFile.h"
#include "genericThread.h"
#include "threadPriority.h"
#include "pmutex.h"
#include "reMutex.h"
#include "conditionVar.h"
#include "pdeque.h"

class FfmpegVideo;
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVPacket;
struct AVFrame;
struct SwsContext;

/**
 *
 */
class EXPCL_FFMPEG FfmpegVideoCursor : public MovieVideoCursor {
private:
  FfmpegVideoCursor();
  void init_from(FfmpegVideo *src);

PUBLISHED:
  FfmpegVideoCursor(FfmpegVideo *src);
  virtual ~FfmpegVideoCursor();

  void set_max_readahead_frames(int max_readahead_frames);
  int get_max_readahead_frames() const;

  void set_thread_priority(ThreadPriority thread_priority);
  ThreadPriority get_thread_priority() const;

  void start_thread();
  BLOCKING void stop_thread();
  bool is_thread_started() const;

public:
  virtual bool set_time(double timestamp, int loop_count);
  virtual PT(Buffer) fetch_buffer();

public:
  // Nested class must be public for PT(FfmpegBuffer) to work correctly.
  class EXPCL_FFMPEG FfmpegBuffer : public Buffer {
  public:
    ALLOC_DELETED_CHAIN(FfmpegBuffer);
    INLINE FfmpegBuffer(size_t block_size, double video_timebase);
    virtual int compare_timestamp(const Buffer *other) const;
    virtual double get_timestamp() const;

    int _begin_frame;
    int _end_frame;
    double _video_timebase;

  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      Buffer::init_type();
      register_type(_type_handle, "FfmpegVideoCursor::FfmpegBuffer",
                    Buffer::get_class_type());
    }
    virtual TypeHandle get_type() const {
      return get_class_type();
    }
    virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

  private:
    static TypeHandle _type_handle;
  };

protected:
  virtual PT(Buffer) make_new_buffer();

private:
  bool open_stream();
  void close_stream();
  void cleanup();

  Filename _filename;
  std::string _sync_name;
  int _max_readahead_frames;
  ThreadPriority _thread_priority;
  PT(GenericThread) _thread;

  int _pixel_format;

  // This global Mutex protects calls to avcodec_opencloseetc.
  static ReMutex _av_lock;

  // Protects _readahead_frames and all the immediately following members.
  Mutex _lock;

  // Condition: the thread has something to do.
  ConditionVar _action_cvar;

  typedef pdeque<PT(FfmpegBuffer) > Buffers;
  Buffers _readahead_frames;
  enum ThreadStatus {
    TS_stopped,
    TS_wait,
    TS_readahead,
    TS_seek,
    TS_seeking,
    TS_shutdown,
  };
  ThreadStatus _thread_status;
  int _seek_frame;

  int _current_frame;
  PT(FfmpegBuffer) _current_frame_buffer;

private:
  // The following functions will be called in the sub-thread.
  static void st_thread_main(void *self);
  void thread_main();
  bool do_poll();

  PT(FfmpegBuffer) do_alloc_frame();
  void do_clear_all_frames();

  bool fetch_packet(int default_frame);
  bool do_fetch_packet(int default_frame);
  void fetch_frame(int frame);
  void decode_frame(int &finished);
  void do_decode_frame(int &finished);
  void seek(int frame, bool backward);
  void do_seek(int frame, bool backward);
  int binary_seek(int min_frame, int max_frame, int target_frame, int num_iterations);
  void advance_to_frame(int frame);
  void reset_stream();
  void export_frame(FfmpegBuffer *buffer);

  // The following data members will be accessed by the sub-thread.
  AVPacket *_packet;
  int _packet_frame;
  AVFormatContext *_format_ctx;
  AVCodecContext *_video_ctx;
  SwsContext *_convert_ctx;

  FfmpegVirtualFile _ffvfile;
  int _video_index;
  double _video_timebase;
  AVFrame *_frame;
  AVFrame *_frame_out;
  int _initial_dts;
  double _min_fseek;
  int _begin_frame;
  int _end_frame;
  bool _frame_ready;
  bool _eof_known;
  int _eof_frame;

  // PStat collectors.
  static PStatCollector _fetch_buffer_pcollector;
  static PStatCollector _seek_pcollector;
  static PStatCollector _export_frame_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MovieVideoCursor::init_type();
    register_type(_type_handle, "FfmpegVideoCursor",
                  MovieVideoCursor::get_class_type());
    FfmpegBuffer::init_type();
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class FfmpegVideo;
};

#include "ffmpegVideoCursor.I"

#endif // FFMPEGVIDEO_H
