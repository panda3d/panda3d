// Filename: ffmpegAudioCursor.h
// Created by: jyelon (01Aug2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FFMPEGAUDIOCURSOR_H
#define FFMPEGAUDIOCURSOR_H
#ifdef HAVE_FFMPEG

#include "pandabase.h"
#include "namable.h"
#include "texture.h"
#include "pointerTo.h"

struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct AVPacket;

////////////////////////////////////////////////////////////////////
//       Class : FfmpegAudioCursor
// Description : A stream that generates a sequence of audio samples.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegAudioCursor : public MovieAudioCursor {

PUBLISHED:
  FfmpegAudioCursor(PT(FfmpegAudio) src);
  virtual ~FfmpegAudioCursor();
  virtual void seek(double offset);
  
public:
  virtual void read_samples(int n, PN_int16 *data);
  
protected:
  void fetch_packet();
  void reload_buffer();
  void cleanup();
  Filename _filename;
  int _initial_dts;
  AVPacket *_packet;
  int            _packet_size;
  unsigned char *_packet_data;
  AVFormatContext *_format_ctx;
  AVCodecContext  *_audio_ctx;
  int _audio_index;
  double _audio_timebase;

  PN_int16 *_buffer;
  int       _buffer_size;
  PN_int16 *_buffer_alloc;
  int       _buffer_head;
  int       _buffer_tail;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "FfmpegAudioCursor",
                  MovieAudioCursor::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ffmpegAudioCursor.I"

#endif // HAVE_FFMPEG
#endif // FFMPEG_AUDIO.H
