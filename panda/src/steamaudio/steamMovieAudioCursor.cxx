/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file steamMovieAudioCursor.cxx
 * @author Jackson Sutherland
 */
#include "steamMovieAudioCursor.h"


SteamMovieAudioCursor::
SteamMovieAudioCursor(SteamMovieAudio* src) :
  MovieAudioCursor(src)
{
  _source_cursor = ((PT(SteamMovieAudio))_source)->_audio_source->open();

  //make audiosettings
  _steamAudioSettings = &IPLAudioSettings{};
  _steamAudioSettings->samplingRate = _source_cursor->audio_rate();
  _steamAudioSettings->frameSize = 65536;
  //IPLAudioSetting's values tell Steam Audio's classes the speed and amount of data being processed.
  //frameSize is just the length of the buffers we're handing them.
}

SteamMovieAudioCursor::
~SteamMovieAudioCursor() {

}

int SteamMovieAudioCursor::
read_samples(int n, int16_t* data) {//IN PROGRESS: adding a loop so we can actually do const array sizes
  if (n <= 0) {
    return 0;
  }

  int length = n * _audio_channels;

  int dataPoint = 0;
  
  while (n > 0) {
    int blocksize = (65536 / _audio_channels);
    if (blocksize > n) { blocksize = n; }

    int16_t srcData[65536];

    int nread = _source_cursor->read_samples(blocksize, srcData);
    if (nread == 0) { return n; }

    IPLfloat32 fData[65536];//we need to change 16ints to IPLfloats

    for (size_t i = 0; i < blocksize; i++) {
      fData[i] = (IPLfloat32)srcData[i];//get int16_t and cast to IPLfloat32
    }

    _steamAudioSettings->frameSize = blocksize;

    IPLAudioBuffer inBuffer;
    iplAudioBufferAllocate(*((PT(SteamMovieAudio))_source)->_steamContext, _audio_channels, blocksize, &inBuffer);
    iplAudioBufferDeinterleave(*((PT(SteamMovieAudio))_source)->_steamContext, fData, &inBuffer);
    SteamAudioSound::SteamGlobalHolder globals(_steamAudioSettings, ((PT(SteamMovieAudio))_source)->_steamContext, _audio_channels, n, &_source);

    for (size_t i = 0; i < ((PT(SteamMovieAudio))_source)->_steam_effects.size(); i++) {
      SteamAudioEffect effect = *((PT(SteamMovieAudio))_source)->_steam_effects[i];
      if (effect._isActive) {//TODO:: Add conditions for skipping simulated effects if no simulator, effects that encode to ambisonics, etc
        IPLAudioBuffer outBuffer = effect.apply_effect(&globals, inBuffer);
        std::swap(inBuffer, outBuffer);
        iplAudioBufferFree(*((PT(SteamMovieAudio))_source)->_steamContext, &outBuffer);
      }
    }

    iplAudioBufferInterleave(*((PT(SteamMovieAudio))_source)->_steamContext, &inBuffer, fData);
    iplAudioBufferFree(*((PT(SteamMovieAudio))_source)->_steamContext, &inBuffer);

    for (size_t i = 0; i < blocksize; i++) {
      data[dataPoint] = (int16_t)fData[i];
      dataPoint++
    }
  }
  return n;
}

SteamMovieAudioCursor::SteamGlobalHolder
::SteamGlobalHolder(IPLAudioSettings* audio_settings, IPLContext* steam_context, int channels, int samples, SteamMovieAudio* _source) :
  _audio_settings(audio_settings),
  _steam_context(steam_context),
  _channels(channels),
  _samples(samples),
  source(_source)
{
  
}
