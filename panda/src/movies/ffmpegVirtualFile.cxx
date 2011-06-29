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

#ifdef HAVE_FFMPEG

#include "pandabase.h"
#include "config_movies.h"
#include "ffmpegVirtualFile.h"
#include "virtualFileSystem.h"

#ifndef AVSEEK_SIZE
  #define AVSEEK_SIZE 0x10000
#endif

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVirtualFile::
FfmpegVirtualFile() : 
  _format_context(NULL),
  _in(NULL),
  _owns_in(false)
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

  // I tried to use av_open_input_stream(), but it (a) required a lot
  // of low-level stream analysis calls that really should be
  // automatic (and are automatic in av_open_input_file()), and (b)
  // was broken on the ffmpeg build I happened to grab.  Screw it,
  // clearly av_open_input_file() is the preferred and more
  // heavily-exercised interface.  So we'll continue to use url
  // synthesis as a hacky hook into this interface.

  // Nowadays we synthesize a "url" that references this pointer.
  ostringstream strm;
  strm << "pandavfs://" << (void *)this;
  string url = strm.str();

  // Now we can open the stream.
  int result = 
    av_open_input_file(&_format_context, url.c_str(), NULL, 0, NULL);
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

  _in = &_file_in;
  _owns_in = false;
  _start = info.get_start();
  _size = info.get_size();

  _in->seekg(_start);

  // I tried to use av_open_input_stream(), but it (a) required a lot
  // of low-level ffmpeg calls that really shouldn't be part of the
  // public API (and which aren't necessary with av_open_input_file()
  // because they happen implicitly there), and (b) was completely
  // broken on the ffmpeg build I happened to grab.  Screw it; clearly
  // av_open_input_file() is the preferred and more heavily-exercised
  // interface.  So we'll use it, even though it requires a bit of a
  // hack.

  // The hack is that we synthesize a "url" that references this
  // pointer, then open that url.  This calls pandavfs_open(), which
  // decodes the pointer and stores it for future callbacks.
  ostringstream strm;
  strm << "pandavfs://" << (void *)this;
  string url = strm.str();

  // Now we can open the stream.
  int result = 
    av_open_input_file(&_format_context, url.c_str(), NULL, 0, NULL);
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
    av_close_input_file(_format_context);
    _format_context = NULL;
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

  static URLProtocol protocol;
  protocol.name = "pandavfs";
  protocol.url_open  = pandavfs_open;
  protocol.url_read  = pandavfs_read;

#if LIBAVFORMAT_VERSION_INT < 3425280
  protocol.url_write = (int (*)(URLContext *, unsigned char *, int))pandavfs_write;
#else
  protocol.url_write = pandavfs_write;
#endif

  protocol.url_seek  = pandavfs_seek;
  protocol.url_close = pandavfs_close;
#if LIBAVFORMAT_VERSION_INT < 3415296
  ::register_protocol(&protocol);
#elif LIBAVFORMAT_VERSION_MAJOR < 53
  av_register_protocol(&protocol);
#else
  av_register_protocol2(&protocol, sizeof(protocol));
#endif

  // Let's also register the logging to Panda's notify callback.
  av_log_set_callback(&log_callback);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::pandavfs_open
//       Access: Private, Static
//  Description: A callback to "open" a virtual file.  Actually, all
//               this does is assign the pointer back to the
//               FfmpegVirtualFile instance.
////////////////////////////////////////////////////////////////////
int FfmpegVirtualFile::
pandavfs_open(URLContext *h, const char *filename, int flags) {
  filename += 11; // Skip over "pandavfs://"
  istringstream strm(filename);
  void *ptr = 0;
  strm >> ptr;

  FfmpegVirtualFile *self = (FfmpegVirtualFile *)ptr;
  h->priv_data = self;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::pandavfs_read
//       Access: Private, Static
//  Description: A callback to read a virtual file.
////////////////////////////////////////////////////////////////////
int FfmpegVirtualFile::
pandavfs_read(URLContext *h, unsigned char *buf, int size) {
  FfmpegVirtualFile *self = (FfmpegVirtualFile *)(h->priv_data);
  istream *in = self->_in;

  // Since we may be simulating a subset of the opened stream, don't
  // allow it to read past the "end".
  streampos remaining = self->_start + (streampos)self->_size - in->tellg();
  if (remaining < size) {
    if (remaining <= 0) {
      return 0;
    }

    size = (int)remaining;
  }

  in->read((char *)buf, size);
  int gc = in->gcount();
  in->clear();

  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::pandavfs_write
//       Access: Private, Static
//  Description: A callback to write a virtual file.  Unimplemented,
//               because we use ffmpeg for playback only, not for
//               encoding video streams.
////////////////////////////////////////////////////////////////////
int FfmpegVirtualFile::
pandavfs_write(URLContext *h, const unsigned char *buf, int size) {
  movies_cat.warning()
    << "ffmpeg is trying to write to the VFS.\n";
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::pandavfs_seek
//       Access: Private, Static
//  Description: A callback to change the read position on an istream.
////////////////////////////////////////////////////////////////////
int64_t FfmpegVirtualFile::
pandavfs_seek(URLContext *h, int64_t pos, int whence) {
  FfmpegVirtualFile *self = (FfmpegVirtualFile *)(h->priv_data);
  istream *in = self->_in;

  switch(whence) {
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
    movies_cat.error() 
      << "Illegal parameter to seek in ffmpegVirtualFile\n";
    in->clear();
    return -1;
  }

  in->clear();
  return in->tellg() - self->_start;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::pandavfs_close
//       Access: Private, Static
//  Description: A hook to "close" a panda VFS file.  Actually it only
//               clears the associated pointer.
////////////////////////////////////////////////////////////////////
int FfmpegVirtualFile::
pandavfs_close(URLContext *h) {
  FfmpegVirtualFile *self = (FfmpegVirtualFile *)(h->priv_data);
  h->priv_data = 0;
  return 0;
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
    static char buffer[buffer_size];
    vsnprintf(buffer, buffer_size, fmt, v1);
    ffmpeg_cat.out(severity, true)
      << buffer;
  }
}

#endif // HAVE_FFMPEG
