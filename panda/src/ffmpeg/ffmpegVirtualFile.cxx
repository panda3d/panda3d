// Filename: ffmpegVirtualFile.cxx
// Created by: jyelon (02Jul07)
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

#include "pandabase.h"

#include "config_movies.h"
#include "ffmpegVirtualFile.h"
#include "virtualFileSystem.h"

extern "C" {
  #include "libavcodec/avcodec.h"
  #include "libavformat/avformat.h"
}

#ifndef AVSEEK_SIZE
  #define AVSEEK_SIZE 0x10000
#endif

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::Constructor
//       Access: Public
//  Description: 
///////////////////////////////p/////////////////////////////////////
FfmpegVirtualFile::
FfmpegVirtualFile() :
  _io_context(NULL),
  _format_context(NULL),
  _in(NULL),
  _owns_in(false),
  _buffer_size(ffmpeg_read_buffer_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVirtualFile::
~FfmpegVirtualFile() {
  close();
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::Copy Constructor
//       Access: Private
//  Description: These objects are not meant to be copied.
////////////////////////////////////////////////////////////////////
FfmpegVirtualFile::
FfmpegVirtualFile(const FfmpegVirtualFile &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::Copy Assignment Operator
//       Access: Private
//  Description: These objects are not meant to be copied.
////////////////////////////////////////////////////////////////////
void FfmpegVirtualFile::
operator = (const FfmpegVirtualFile &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::open_vfs
//       Access: Public
//  Description: Opens the movie file via Panda's VFS.  Returns true
//               on success, false on failure.  If successful, use
//               get_format_context() to get the open file handle.
////////////////////////////////////////////////////////////////////
bool FfmpegVirtualFile::
open_vfs(const Filename &filename) {
  close();

  if (ffmpeg_cat.is_debug()) {
    ffmpeg_cat.debug()
      << "ffmpeg open_vfs(" << filename << ")\n";
  }

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename fname = filename;
  fname.set_binary();
  PT(VirtualFile) vfile = vfs->get_file(fname);
  if (vfile == NULL) {
    return false;
  }

  _in = vfile->open_read_file(true);
  if (_in == NULL) {
    return false;
  }

  _owns_in = true;
  _start = 0;
  _size = vfile->get_file_size(_in);

  // NOTE: The AVIO system owns the buffer after allocation and may realloc it
  // internally. Therefore, when we're done with the buffer, we use
  // _io_context->buffer to deallocate it rather than holding on to this pointer.
  unsigned char *buffer = (unsigned char*) av_malloc(_buffer_size);
  _io_context = avio_alloc_context(buffer, _buffer_size, 0, (void*) this,
                                   &read_packet, 0, &seek);

  _format_context = avformat_alloc_context();
  _format_context->pb = _io_context;

  // Now we can open the stream.
  int result =
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 4, 0)
    avformat_open_input(&_format_context, "", NULL, NULL);
#else
    av_open_input_file(&_format_context, "", NULL, 0, NULL);
#endif
  if (result < 0) {
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::open_subfile
//       Access: Public
//  Description: Opens the movie file directly from a file on disk
//               (does not go through the VFS).  Returns true on
//               success, false on failure.  If successful, use
//               get_format_context() to get the open file handle.
////////////////////////////////////////////////////////////////////
bool FfmpegVirtualFile::
open_subfile(const SubfileInfo &info) {
  close();

  Filename fname = info.get_filename();
  fname.set_binary();
  if (!fname.open_read(_file_in)) {
    return false;
  }
  if (ffmpeg_cat.is_debug()) {
    ffmpeg_cat.debug()
      << "ffmpeg open_subfile(" << fname << ")\n";
  }

  _in = &_file_in;
  _owns_in = false;
  _start = info.get_start();
  _size = info.get_size();

  _in->seekg(_start);

  // NOTE: The AVIO system owns the buffer after allocation and may realloc it
  // internally. Therefore, when we're done with the buffer, we use
  // _io_context->buffer to deallocate it rather than holding on to this pointer.
  unsigned char *buffer = (unsigned char*) av_malloc(_buffer_size);
  _io_context = avio_alloc_context(buffer, _buffer_size, 0, (void*) this,
                                   &read_packet, 0, &seek);

  _format_context = avformat_alloc_context();
  _format_context->pb = _io_context;

  // Now we can open the stream.
  int result =
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 4, 0)
    avformat_open_input(&_format_context, fname.c_str(), NULL, NULL);
#else
    av_open_input_file(&_format_context, fname.c_str(), NULL, 0, NULL);
#endif
  if (result < 0) {
    close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::close
//       Access: Public
//  Description: Explicitly closes the opened file.  This is also
//               called implicitly by the destructor if necessary.
////////////////////////////////////////////////////////////////////
void FfmpegVirtualFile::
close() {
  if (_format_context != NULL) {
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53, 17, 0)
    avformat_close_input(&_format_context);
#else
    av_close_input_file(_format_context);
    _format_context = NULL;
#endif
  }

  if (_io_context != NULL) {
    if (_io_context->buffer != NULL) {
      av_free(_io_context->buffer);
    }
    av_free(_io_context);
    _io_context = NULL;
  }

  if (_owns_in) {
    nassertv(_in != NULL);
    VirtualFileSystem::close_read_file(_in);
    _owns_in = false;
  }
  _in = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::register_protocol
//       Access: Public, Static
//  Description: Should be called at startup to attach the appropriate
//               hooks between Panda and FFMpeg.
////////////////////////////////////////////////////////////////////
void FfmpegVirtualFile::
register_protocol() {
  static bool initialized = false;
  if (initialized) {
    return;
  }

  // Here's a good place to call this global ffmpeg initialization
  // function.
  av_register_all();

  // And this one.
#if LIBAVFORMAT_VERSION_INT >= 0x351400
  avformat_network_init();
#endif

  // Let's also register the logging to Panda's notify callback.
  av_log_set_callback(&log_callback);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::read_packet
//       Access: Private, Static
//  Description: A callback to read a virtual file.
////////////////////////////////////////////////////////////////////
int FfmpegVirtualFile::
read_packet(void *opaque, uint8_t *buf, int size) {
  streampos ssize = (streampos)size;
  FfmpegVirtualFile *self = (FfmpegVirtualFile *) opaque;
  istream *in = self->_in;

  // Since we may be simulating a subset of the opened stream, don't
  // allow it to read past the "end".
  streampos remaining = self->_start + (streampos)self->_size - in->tellg();
  if (remaining < ssize) {
    if (remaining <= 0) {
      return 0;
    }

    ssize = remaining;
  }

  in->read((char *)buf, ssize);
  streamsize gc = in->gcount();
  in->clear();

  return (int)gc;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::seek
//       Access: Private, Static
//  Description: A callback to change the read position on an istream.
////////////////////////////////////////////////////////////////////
int64_t FfmpegVirtualFile::
seek(void *opaque, int64_t pos, int whence) {
  FfmpegVirtualFile *self = (FfmpegVirtualFile *) opaque;
  istream *in = self->_in;

  switch (whence) {
  case SEEK_SET:
    in->seekg(self->_start + (streampos)pos, ios::beg);
    break;

  case SEEK_CUR:
    in->seekg(pos, ios::cur);
    break;

  case SEEK_END:
    // For seeks relative to the end, we actually compute the end
    // based on _start + _size, and then use ios::beg.
    in->seekg(self->_start + (streampos)self->_size + (streampos)pos, ios::beg);
    break;

  case AVSEEK_SIZE:
    return self->_size;

  default:
    ffmpeg_cat.error()
      << "Illegal parameter to seek in FfmpegVirtualFile\n";
    in->clear();
    return -1;
  }

  in->clear();
  return in->tellg() - self->_start;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::log_callback
//       Access: Private, Static
//  Description: These callbacks are made when ffmpeg wants to write a
//               log entry; it redirects into Panda's notify.
////////////////////////////////////////////////////////////////////
void FfmpegVirtualFile::
log_callback(void *ptr, int level, const char *fmt, va_list v1) {
  NotifySeverity severity;
#ifdef AV_LOG_PANIC
  if (level <= AV_LOG_PANIC) {
    severity = NS_fatal;
  } else
#endif
  if (level <= AV_LOG_ERROR) {
    severity = NS_error;
#ifdef AV_LOG_WARNING
  } else if (level <= AV_LOG_WARNING) {
    severity = NS_warning;
#endif
  } else if (level <= AV_LOG_INFO) {
    severity = NS_info;
#ifdef AV_LOG_VERBOSE
  } else if (level <= AV_LOG_VERBOSE) {
    severity = NS_debug;
#endif
  } else /* level <= AV_LOG_DEBUG */ {
    severity = NS_spam;
  }

  if (ffmpeg_cat.is_on(severity)) {
    static const size_t buffer_size = 4096;
    char *buffer = (char *)alloca(buffer_size);
    vsnprintf(buffer, buffer_size, fmt, v1);
    nassertv(strlen(buffer) < buffer_size);
    ffmpeg_cat.out(severity, true)
      << buffer;
  }
}
