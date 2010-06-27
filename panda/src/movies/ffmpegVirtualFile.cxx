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
extern "C" {
  #include "libavformat/avio.h"
}

#ifndef AVSEEK_SIZE
  #define AVSEEK_SIZE 0x10000
#endif

////////////////////////////////////////////////////////////////////
// These functions need to use C calling conventions.
////////////////////////////////////////////////////////////////////
extern "C" {
  static int       pandavfs_open(URLContext *h, const char *filename, int flags);
  static int       pandavfs_read(URLContext *h, unsigned char *buf, int size);
// This change happened a few days before 52.68.0
#if LIBAVFORMAT_VERSION_INT < 3425280
  static int       pandavfs_write(URLContext *h, const unsigned char *buf, int size);
#else
  static int       pandavfs_write(URLContext *h, unsigned char *buf, int size);
#endif
  static PN_int64  pandavfs_seek(URLContext *h, PN_int64 pos, int whence);
  static int       pandavfs_close(URLContext *h);
}

////////////////////////////////////////////////////////////////////
//     Function: pandavfs_open
//       Access: Static Function
//  Description: A hook to open a panda VFS file.
////////////////////////////////////////////////////////////////////
static int
pandavfs_open(URLContext *h, const char *filename, int flags) {
  if (flags != 0) {
    movies_cat.error() << "ffmpeg is trying to write to the VFS.\n";
    return -1;
  }
  filename += 9; // Skip over "pandavfs:"
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *s = vfs->open_read_file(filename, true);
  if (s == 0) {
    return -1;
  }
  // Test whether seek works.
  s->seekg(1, ios::beg);
  int tel1 = s->tellg();
  s->seekg(0, ios::beg);
  int tel2 = s->tellg();
  if (s->fail() || (tel1!=1) || (tel2!=0)) {
    movies_cat.error() << "cannot play movie (not seekable): " << h->filename << "\n";
    delete s;
    return -1;
  }
  h->priv_data = s;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: pandavfs_read
//       Access: Static Function
//  Description: A hook to read a panda VFS file.
////////////////////////////////////////////////////////////////////
static int
pandavfs_read(URLContext *h, unsigned char *buf, int size) {
  istream *s = (istream*)(h->priv_data);
  s->read((char*)buf, size);
  int gc = s->gcount();
  s->clear();
  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: pandavfs_write
//       Access: Static Function
//  Description: A hook to write a panda VFS file.
////////////////////////////////////////////////////////////////////
static int
#if LIBAVFORMAT_VERSION_INT < 3425280
pandavfs_write(URLContext *h, unsigned char *buf, int size) {
#else
pandavfs_write(URLContext *h, const unsigned char *buf, int size) {
#endif
  movies_cat.error() << "ffmpeg is trying to write to the VFS.\n";
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: pandavfs_seek
//       Access: Static Function
//  Description: A hook to seek a panda VFS file.
////////////////////////////////////////////////////////////////////
static PN_int64
pandavfs_seek(URLContext *h, PN_int64 pos, int whence) {
  istream *s = (istream*)(h->priv_data);
  switch(whence) {
  case SEEK_SET: s->seekg(pos, ios::beg); break;
  case SEEK_CUR: s->seekg(pos, ios::cur); break;
  case SEEK_END: s->seekg(pos, ios::end); break;
  case AVSEEK_SIZE: {
    s->seekg(0, ios::cur);
    int p = s->tellg();
    s->seekg(-1, ios::end);
    int size = s->tellg();
    if (size < 0) {
      movies_cat.error() << "Failed to determine filesize in ffmpegVirtualFile\n";
      s->clear();
      return -1;
    }
    size++;
    s->seekg(p, ios::beg);
    s->clear();
    return size; }
  default:
    movies_cat.error() << "Illegal parameter to seek in ffmpegVirtualFile\n";
    s->clear();
    return -1;
  }
  s->clear();
  int tl = s->tellg();
  return tl;
}

////////////////////////////////////////////////////////////////////
//     Function: pandavfs_close
//       Access: Static Function
//  Description: A hook to close a panda VFS file.
////////////////////////////////////////////////////////////////////
static int
pandavfs_close(URLContext *h) {
  istream *s = (istream*)(h->priv_data);
  delete s;
  h->priv_data = 0;
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::register_protocol
//       Access: Public, Static
//  Description: Enables ffmpeg to access panda's VFS.
//
//               After calling this method, ffmpeg will be
//               able to open "URLs" that look like this:
//               
//               pandavfs:/c/mygame/foo.avi
//
////////////////////////////////////////////////////////////////////
void FfmpegVirtualFile::
register_protocol() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  static URLProtocol protocol;
  protocol.name = "pandavfs";
  protocol.url_open  = pandavfs_open;
  protocol.url_read  = pandavfs_read;
  protocol.url_write = pandavfs_write;
  protocol.url_seek  = pandavfs_seek;
  protocol.url_close = pandavfs_close;
#if LIBAVFORMAT_VERSION_INT < 3415296
  ::register_protocol(&protocol);
#else
  av_register_protocol(&protocol);
#endif
}

#endif // HAVE_FFMPEG
