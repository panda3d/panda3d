/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file flacAudioCursor.cxx
 * @author rdb
 * @date 2013-08-23
 */

 #include "flacAudioCursor.h"
 #include "flacAudio.h"
 #include "virtualFileSystem.h"
 #include "config_movies.h"
 
 #define DR_FLAC_IMPLEMENTATION
 extern "C" {
   #include "dr_flac.h"
 }
 
 /**
  * Callback passed to dr_flac to implement file I/O via the VirtualFileSystem.
  */
 static size_t cb_read_proc(void *user, void *buffer, size_t size) {
   std::istream *stream = static_cast<std::istream *>(user);
   nassertr(stream != nullptr, 0);
 
   stream->read(static_cast<char *>(buffer), size);
 
   if (stream->eof()) {
     // Gracefully handle EOF.
     stream->clear();
   }
 
   return stream->gcount();
 }
 
 /**
  * Callback passed to dr_flac to implement file seeking via the VirtualFileSystem.
  */
 static drflac_bool32 cb_seek_proc(void* user, int offset, drflac_seek_origin origin) {
   std::istream* stream = static_cast<std::istream*>(user);
   nassertr(stream != nullptr, DRFLAC_FALSE);
 
   std::ios_base::seekdir dir;
   switch (origin) {
     case drflac_seek_origin_start:
       dir = std::ios_base::beg;
       break;
     case drflac_seek_origin_current:
       dir = std::ios_base::cur;
       break;
     default:
       return DRFLAC_FALSE;
   }
 
   stream->seekg(offset, dir);
   return !stream->fail() ? DRFLAC_TRUE : DRFLAC_FALSE;
 }
 
 TypeHandle FlacAudioCursor::_type_handle;
 
 /**
  * Constructor for FlacAudioCursor. Initializes the FLAC stream and sets the audio properties.
  */
 FlacAudioCursor::FlacAudioCursor(FlacAudio *src, std::istream *stream) :
   MovieAudioCursor(src),
   _is_valid(false),
   _drflac(nullptr),
   _stream(stream)
 {
   nassertv(stream != nullptr);
   nassertv(stream->good());
 
   _drflac = drflac_open(&cb_read_proc, &cb_seek_proc, static_cast<void*>(stream), nullptr);
 
   if (_drflac == nullptr) {
     movies_cat.error() << "Failed to open FLAC file.\n";
     _is_valid = false;
     return;
   }
 
   _length = (_drflac->totalPCMFrameCount) / static_cast<double>(_drflac->sampleRate);
   _audio_channels = _drflac->channels;
   _audio_rate = _drflac->sampleRate;
 
   _can_seek = true;
   _can_seek_fast = _can_seek;
   _is_valid = true;
 }
 
 /**
  * Destructor for FlacAudioCursor. Closes the FLAC stream and associated resources.
  */
 FlacAudioCursor::~FlacAudioCursor() {
   if (_drflac != nullptr) {
     drflac_close(_drflac);
   }
   if (_stream != nullptr) {
     VirtualFileSystem::close_read_file(_stream);
   }
 }
 
 /**
  * Seeks to a specific time in the FLAC file. Updates internal states to reflect the new position.
  */
 void FlacAudioCursor::seek(double t) {
   t = std::max(t, 0.0);
   uint64_t target_frame = static_cast<uint64_t>(t * _drflac->sampleRate);
   if (drflac_seek_to_pcm_frame(_drflac, target_frame)) {
     _last_seek = target_frame / static_cast<double>(_drflac->sampleRate);
     _samples_read = 0;
   }
 }
 
 /**
  * Reads audio samples from the FLAC stream. Returns the number of samples read per channel.
  */
 int FlacAudioCursor::read_samples(int n, int16_t* data) {
   uint64_t frames_read = drflac_read_pcm_frames_s16(_drflac, static_cast<drflac_uint64>(n), data);
   _samples_read += frames_read;
   return static_cast<int>(frames_read);
 }
 