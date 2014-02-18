// Filename: vorbisAudioCursor.cxx
// Created by: rdb (23Aug13)
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

#include "vorbisAudioCursor.h"
#include "virtualFileSystem.h"

#ifdef HAVE_VORBIS

TypeHandle VorbisAudioCursor::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::Constructor
//       Access: Protected
//  Description: Reads the .wav header from the indicated stream.
//               This leaves the read pointer positioned at the
//               start of the data.
////////////////////////////////////////////////////////////////////
VorbisAudioCursor::
VorbisAudioCursor(VorbisAudio *src, istream *stream) :
  MovieAudioCursor(src),
  _is_valid(false),
  _bitstream(0)
{
  nassertv(stream != NULL);
  nassertv(stream->good());

  // Set up the callbacks to read via the VFS.
  ov_callbacks callbacks;
  callbacks.read_func = &cb_read_func;
  callbacks.close_func = &cb_close_func;
  callbacks.tell_func = &cb_tell_func;

  if (vorbis_enable_seek) {
    callbacks.seek_func = &cb_seek_func;
  } else {
    callbacks.seek_func = NULL;
  }

  if (ov_open_callbacks((void*) stream, &_ov, NULL, 0, callbacks) != 0) {
    movies_cat.error()
      << "Failed to read Ogg Vorbis file.\n";
    return;
  }

  double time_total = ov_time_total(&_ov, -1);
  if (time_total != OV_EINVAL) {
    _length = time_total;
  }

  vorbis_info *vi = ov_info(&_ov, -1);
  _audio_channels = vi->channels;
  _audio_rate = vi->rate;

  _can_seek = vorbis_enable_seek && (ov_seekable(&_ov) != 0);
  _can_seek_fast = _can_seek;

  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::Destructor
//       Access: Protected, Virtual
//  Description: xxx
////////////////////////////////////////////////////////////////////
VorbisAudioCursor::
~VorbisAudioCursor() {
  ov_clear(&_ov);
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::seek
//       Access: Protected
//  Description: Seeks to a target location.  Afterward, the
//               packet_time is guaranteed to be less than or
//               equal to the specified time.
////////////////////////////////////////////////////////////////////
void VorbisAudioCursor::
seek(double t) {
  if (!vorbis_enable_seek) {
    return;
  }

  t = max(t, 0.0);

  // Use ov_time_seek_lap if cross-lapping is enabled.
  if (vorbis_seek_lap) {
    if (ov_time_seek_lap(&_ov, t) != 0) {
      movies_cat.error()
        << "Seek failed.  Ogg Vorbis stream may not be seekable.\n";
      return;
    }
  } else {
    if (ov_time_seek(&_ov, t) != 0) {
      movies_cat.error()
        << "Seek failed.  Ogg Vorbis stream may not be seekable.\n";
      return;
    }
  }

  _last_seek = ov_time_tell(&_ov);
  _samples_read = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::read_samples
//       Access: Public, Virtual
//  Description: Read audio samples from the stream.  N is the
//               number of samples you wish to read.  Your buffer
//               must be equal in size to N * channels.
//               Multiple-channel audio will be interleaved.
////////////////////////////////////////////////////////////////////
void VorbisAudioCursor::
read_samples(int n, PN_int16 *data) {
  int desired = n * _audio_channels;

  char *buffer = (char*) data;
  int length = desired * 2;

  // Call ov_read repeatedly until the buffer is full.
  while (length > 0) {
    int bitstream;

    // ov_read can give it to us in the exact format we need.  Nifty!
    long read_bytes = ov_read(&_ov, buffer, length, 0, 2, 1, &bitstream);
    if (read_bytes > 0) {
      buffer += read_bytes;
      length -= read_bytes;
    } else {
      break;
    }

    if (_bitstream != bitstream) {
      // It is technically possible for it to change parameters from one
      // bitstream to the next.  However, we don't offer this flexibility.
      vorbis_info *vi = ov_info(&_ov, -1);
      if (vi->channels != _audio_channels || vi->rate != _audio_rate) {
        movies_cat.error()
          << "Ogg Vorbis file has non-matching bitstreams!\n";

        // We'll change it anyway.  Not sure what happens next.
        _audio_channels = vi->channels;
        _audio_rate = vi->rate;
        break;
      }

      _bitstream = bitstream;
    }
  }

  // Fill the rest of the buffer with silence.
  if (length > 0) {
    memset(buffer, 0, length);
    n -= length / 2 / _audio_channels;
  }

  _samples_read += n;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::cb_read_func
//       Access: Private, Static
//  Description: Callback passed to libvorbisfile to implement
//               file I/O via the VirtualFileSystem.
////////////////////////////////////////////////////////////////////
size_t VorbisAudioCursor::
cb_read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {
  istream *stream = (istream*) datasource;
  nassertr(stream != NULL, -1);

  stream->read((char *)ptr, size * nmemb);

  if (stream->eof()) {
    // Gracefully handle EOF.
    stream->clear();
  }

  return stream->gcount();
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::cb_seek_func
//       Access: Private, Static
//  Description: Callback passed to libvorbisfile to implement
//               file I/O via the VirtualFileSystem.
////////////////////////////////////////////////////////////////////
int VorbisAudioCursor::
cb_seek_func(void *datasource, ogg_int64_t offset, int whence) {
  if (!vorbis_enable_seek) {
    return -1;
  }

  istream *stream = (istream*) datasource;
  nassertr(stream != NULL, -1);

  switch (whence) {
  case SEEK_SET:
    stream->seekg(offset, ios::beg);
    break;

  case SEEK_CUR:
    stream->seekg(offset, ios::cur);
    break;

  case SEEK_END:
    stream->seekg(offset, ios::end);
    break;

  default:
    movies_cat.error()
      << "Illegal parameter to seek in VorbisAudioCursor::cb_seek_func\n";
    return -1;
  }

  if (stream->fail()) {
    // This is a fatal error and usually leads to
    // a libvorbis crash.
    movies_cat.error()
      << "Failure to seek to byte " << offset;

    switch (whence) {
    case SEEK_CUR:
      movies_cat.error(false)
        << " from current location!\n";
      break;

    case SEEK_END:
      movies_cat.error(false)
        << " from end of file!\n";
      break;

    default:
      movies_cat.error(false) << "!\n";
    }

    return -1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::cb_close_func
//       Access: Private, Static
//  Description: Callback passed to libvorbisfile to implement
//               file I/O via the VirtualFileSystem.
////////////////////////////////////////////////////////////////////
int VorbisAudioCursor::
cb_close_func(void *datasource) {
  istream *stream = (istream*) datasource;
  nassertr(stream != NULL, -1);

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->close_read_file(stream);

  // Return value isn't checked, but let's be predictable
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VorbisAudioCursor::cb_tell_func
//       Access: Private, Static
//  Description: Callback passed to libvorbisfile to implement
//               file I/O via the VirtualFileSystem.
////////////////////////////////////////////////////////////////////
long VorbisAudioCursor::
cb_tell_func(void *datasource) {
  istream *stream = (istream*) datasource;
  nassertr(stream != NULL, -1);

  return stream->tellg();
}

#endif // HAVE_VORBIS
