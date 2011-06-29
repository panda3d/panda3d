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

TypeHandle FfmpegVideoCursor::_type_handle;

#if LIBAVFORMAT_VERSION_MAJOR < 53
  #define AVMEDIA_TYPE_VIDEO CODEC_TYPE_VIDEO
#endif

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Default Constructor
//       Access: Protected
//  Description: This constructor is only used when reading from a bam
//               file.
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
FfmpegVideoCursor() :
  _packet(NULL),
  _format_ctx(NULL),
  _video_ctx(NULL),
  _video_index(-1),
  _frame(NULL),
  _frame_out(NULL),
  _min_fseek(3.0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::init_from
//       Access: Protected
//  Description: Specifies the source of the video cursor.  This is
//               normally called only by the constructor or when
//               reading from a bam file.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
init_from(FfmpegVideo *source) {
  nassertv(source != NULL);
  _source = source;
  _filename = _source->get_filename();

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

  _packet = new AVPacket;
  _frame = avcodec_alloc_frame();
  _frame_out = avcodec_alloc_frame();
  if ((_packet == 0)||(_frame == 0)||(_frame_out == 0)) {
    cleanup();
    return;
  }
  memset(_packet, 0, sizeof(AVPacket));
  
  fetch_packet(0.0);
  _initial_dts = _packet->dts;
  _packet_time = 0.0;
  _last_start = -1.0;
  _next_start = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
FfmpegVideoCursor(FfmpegVideo *src) :
  _packet(NULL),
  _format_ctx(NULL),
  _video_ctx(NULL),
  _video_index(-1),
  _frame(NULL),
  _frame_out(NULL),
  _min_fseek(3.0)
{
  init_from(src);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::Destructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideoCursor::
~FfmpegVideoCursor() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::cleanup
//       Access: Public
//  Description: Reset to a standard inactive state.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
cleanup() {
  if (_frame) {
    av_free(_frame);
    _frame = NULL;
  }

  if (_frame_out) {
    _frame_out->data[0] = 0;
    av_free(_frame_out);
    _frame_out = NULL;
  }

  if (_packet) {
    if (_packet->data) {
      av_free_packet(_packet);
    }
    delete _packet;
    _packet = NULL;
  }

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
//     Function: FfmpegVideoCursor::export_frame
//       Access: Public, Virtual
//  Description: Exports the contents of the frame buffer into the
//               user's target buffer.
////////////////////////////////////////////////////////////////////
static PStatCollector export_frame_collector("*:FFMPEG Convert Video to BGR");
void FfmpegVideoCursor::
export_frame(unsigned char *data, bool bgra, int bufx) {
  PStatTimer timer(export_frame_collector);
  if (bgra) {
    _frame_out->data[0] = data + ((_size_y - 1) * bufx * 4);
    _frame_out->linesize[0] = bufx * -4;
#ifdef HAVE_SWSCALE
    struct SwsContext *convert_ctx = sws_getContext(_size_x, _size_y,
                               _video_ctx->pix_fmt, _size_x, _size_y,
                               PIX_FMT_BGRA, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    nassertv(convert_ctx != NULL);
    sws_scale(convert_ctx, _frame->data, _frame->linesize,
              0, _size_y, _frame_out->data, _frame_out->linesize);
    sws_freeContext(convert_ctx);
#else
    img_convert((AVPicture *)_frame_out, PIX_FMT_BGRA, 
                (AVPicture *)_frame, _video_ctx->pix_fmt, _size_x, _size_y);
#endif
  } else {
    _frame_out->data[0] = data + ((_size_y - 1) * bufx * 3);
    _frame_out->linesize[0] = bufx * -3;
#ifdef HAVE_SWSCALE
    struct SwsContext *convert_ctx = sws_getContext(_size_x, _size_y,
                               _video_ctx->pix_fmt, _size_x, _size_y,
                               PIX_FMT_BGR24, SWS_FAST_BILINEAR, NULL, NULL, NULL);
    nassertv(convert_ctx != NULL);
    sws_scale(convert_ctx, _frame->data, _frame->linesize,
              0, _size_y, _frame_out->data, _frame_out->linesize);
    sws_freeContext(convert_ctx);
#else
    img_convert((AVPicture *)_frame_out, PIX_FMT_BGR24, 
                (AVPicture *)_frame, _video_ctx->pix_fmt, _size_x, _size_y);
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_packet
//       Access: Protected
//  Description: Fetches a video packet and stores it in the 
//               packet buffer.  Sets packet_time to the packet's
//               timestamp.  If a packet could not be read, the
//               packet is cleared and the packet_time is set to
//               the specified default value.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
fetch_packet(double default_time) {
  if (_packet->data) {
    av_free_packet(_packet);
  }
  while (av_read_frame(_format_ctx, _packet) >= 0) {
    if (_packet->stream_index == _video_index) {
      _packet_time = _packet->dts * _video_timebase;
      return;
    }
    av_free_packet(_packet);
  }
  _packet->data = 0;
  _packet_time = default_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_frame
//       Access: Protected
//  Description: Fetches a frame from the stream and stores it in
//               the frame buffer.  Sets last_start and next_start
//               to indicate the extents of the frame.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
fetch_frame() {
  int finished = 0;
  _last_start = _packet_time;
  while (!finished && _packet->data) {
#if LIBAVCODEC_VERSION_INT < 3414272
    avcodec_decode_video(_video_ctx, _frame,
                          &finished, _packet->data, _packet->size);
#else
    avcodec_decode_video2(_video_ctx, _frame, &finished, _packet);
#endif
    fetch_packet(_last_start + 1.0);
  }
  _next_start = _packet_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::seek
//       Access: Protected
//  Description: Seeks to a target location.  Afterward, the
//               packet_time is guaranteed to be less than or 
//               equal to the specified time.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
seek(double t) {
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
  avcodec_close(_video_ctx);
  AVCodec *pVideoCodec=avcodec_find_decoder(_video_ctx->codec_id);
  if(pVideoCodec == 0) {
    cleanup();
    return;
  }
  if(avcodec_open(_video_ctx, pVideoCodec)<0) {
    cleanup();
    return;
  }
  fetch_packet(t);
  if (_packet_time > t) {
    _packet_time = t;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_time
//       Access: Public, Virtual
//  Description: Advance until the specified time is in the 
//               export buffer.
////////////////////////////////////////////////////////////////////
void FfmpegVideoCursor::
fetch_time(double time) {
  if (time < _last_start) {
    // Time is in the past.
    seek(time);
    while (_packet_time <= time) {
      fetch_frame();
    }
  } else if (time < _next_start) {
    // Time is in the present: already have the frame.
  } else if (time < _next_start + _min_fseek) {
    // Time is in the near future.
    while ((_packet_time <= time) && (_packet->data)) {
      fetch_frame();
    }
  } else {
    // Time is in the far future.  Seek forward, then read.
    // There's a danger here: because keyframes are spaced
    // unpredictably, trying to seek forward could actually
    // move us backward in the stream!  This must be avoided.
    // So the rule is, try the seek.  If it hurts us by moving
    // us backward, we increase the minimum threshold distance
    // for forward-seeking in the future.
    
    double base = _packet_time;
    seek(time);
    if (_packet_time < base) {
      _min_fseek += (base - _packet_time);
    }
    while (_packet_time <= time) {
      fetch_frame();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_into_texture
//       Access: Public, Virtual
//  Description: See MovieVideoCursor::fetch_into_texture.
////////////////////////////////////////////////////////////////////
static PStatCollector fetch_into_texture_pcollector("*:FFMPEG Video Decoding");
void FfmpegVideoCursor::
fetch_into_texture(double time, Texture *t, int page) {
  PStatTimer timer(fetch_into_texture_pcollector);
  
  nassertv(t->get_x_size() >= size_x());
  nassertv(t->get_y_size() >= size_y());
  nassertv((t->get_num_components() == 3) || (t->get_num_components() == 4));
  nassertv(t->get_component_width() == 1);
  nassertv(page < t->get_z_size());
  
  PTA_uchar img = t->modify_ram_image();
  
  unsigned char *data = img.p() + page * t->get_expected_ram_page_size();
  
  // If there was an error at any point, synthesize black.
  if (_format_ctx==0) {
    if (data) {
      memset(data,0,t->get_x_size() * t->get_y_size() * t->get_num_components());
    }
    _last_start = time;
    _next_start = time + 1.0;
    return;
  }
  
  fetch_time(time);
  export_frame(data, (t->get_num_components()==4), t->get_x_size());
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::fetch_into_buffer
//       Access: Public, Virtual
//  Description: See MovieVideoCursor::fetch_into_buffer.
////////////////////////////////////////////////////////////////////
static PStatCollector fetch_into_buffer_pcollector("*:FFMPEG Video Decoding");
void FfmpegVideoCursor::
fetch_into_buffer(double time, unsigned char *data, bool bgra) {
  PStatTimer timer(fetch_into_buffer_pcollector);
  
  // If there was an error at any point, synthesize black.
  if (_format_ctx==0) {
    if (data) {
      memset(data,0,size_x()*size_y()*(bgra?4:3));
    }
    _last_start = time;
    _next_start = time + 1.0;
    return;
  }

  fetch_time(time);
  export_frame(data, bgra, _size_x);
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
//       Access: Protected, Static
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
//       Access: Protected
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
